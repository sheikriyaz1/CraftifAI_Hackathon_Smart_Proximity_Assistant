#include "main.h"
#include "app.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef uart;
static TIM_HandleTypeDef echo_timer;

static void led_set(uint8_t on)
{
    GPIO_PinState state = on ? GPIO_PIN_SET : GPIO_PIN_RESET;
#if APP_LED_ACTIVE_HIGH
    HAL_GPIO_WritePin(APP_LED_GPIO_PORT, APP_LED_GPIO_PIN, state);
#else
    HAL_GPIO_WritePin(APP_LED_GPIO_PORT, APP_LED_GPIO_PIN,
                      on ? GPIO_PIN_RESET : GPIO_PIN_SET);
#endif
}

static uint16_t timer_elapsed(uint16_t start)
{
    return (uint16_t)(__HAL_TIM_GET_COUNTER(&echo_timer) - start);
}

static void delay_us(uint16_t microseconds)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&echo_timer);
    while (timer_elapsed(start) < microseconds) {
    }
}

static void print_line(const char *text)
{
    HAL_UART_Transmit(&uart, (uint8_t *)text, (uint16_t)strlen(text), 100);
}

static void peripherals_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    TIM_ClockConfigTypeDef timer_clock = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    gpio.Pin = APP_LED_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(APP_LED_GPIO_PORT, &gpio);
    led_set(0);

    gpio.Pin = APP_HCSR04_TRIGGER_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(APP_HCSR04_TRIGGER_PORT, &gpio);
    HAL_GPIO_WritePin(APP_HCSR04_TRIGGER_PORT, APP_HCSR04_TRIGGER_PIN,
                      GPIO_PIN_RESET);

    gpio.Pin = APP_HCSR04_ECHO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(APP_HCSR04_ECHO_PORT, &gpio);

    gpio.Pin = APP_UART_TX_PIN | APP_UART_RX_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(APP_UART_TX_PORT, &gpio);

    uart.Instance = APP_UART_INSTANCE;
    uart.Init.BaudRate = APP_UART_BAUDRATE;
    uart.Init.WordLength = UART_WORDLENGTH_8B;
    uart.Init.StopBits = UART_STOPBITS_1;
    uart.Init.Parity = UART_PARITY_NONE;
    uart.Init.Mode = UART_MODE_TX_RX;
    uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart.Init.OverSampling = UART_OVERSAMPLING_16;
    uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLED;
    uart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    uart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&uart) != HAL_OK) {
        Error_Handler();
    }

    echo_timer.Instance = TIM2;
    echo_timer.Init.Prescaler = 63;
    echo_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    echo_timer.Init.Period = 0xFFFF;
    echo_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    echo_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&echo_timer) != HAL_OK) {
        Error_Handler();
    }
    timer_clock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&echo_timer, &timer_clock) != HAL_OK ||
        HAL_TIM_Base_Start(&echo_timer) != HAL_OK) {
        Error_Handler();
    }
}

static uint8_t wait_for_echo(GPIO_PinState state, uint16_t timeout_us)
{
    uint16_t start = __HAL_TIM_GET_COUNTER(&echo_timer);
    while (HAL_GPIO_ReadPin(APP_HCSR04_ECHO_PORT, APP_HCSR04_ECHO_PIN) != state) {
        if (timer_elapsed(start) >= timeout_us) {
            return 0;
        }
    }
    return 1;
}

static uint8_t measure_distance_cm(uint32_t *distance_cm)
{
    uint16_t pulse_start;
    uint16_t pulse_width;

    HAL_GPIO_WritePin(APP_HCSR04_TRIGGER_PORT, APP_HCSR04_TRIGGER_PIN,
                      GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(APP_HCSR04_TRIGGER_PORT, APP_HCSR04_TRIGGER_PIN,
                      GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(APP_HCSR04_TRIGGER_PORT, APP_HCSR04_TRIGGER_PIN,
                      GPIO_PIN_RESET);

    if (!wait_for_echo(GPIO_PIN_SET, APP_ECHO_TIMEOUT_US)) {
        return 0;
    }
    pulse_start = __HAL_TIM_GET_COUNTER(&echo_timer);
    if (!wait_for_echo(GPIO_PIN_RESET, APP_ECHO_TIMEOUT_US)) {
        return 0;
    }
    pulse_width = timer_elapsed(pulse_start);
    if (pulse_width == 0U || pulse_width > APP_ECHO_TIMEOUT_US) {
        return 0;
    }

    /* HC-SR04 pulse width is round-trip time: cm ~= us / 58. */
    *distance_cm = ((uint32_t)pulse_width + 29U) / 58U;
    return 1;
}

void app_start(void)
{
    uint32_t distance_cm;
    uint32_t next_update = HAL_GetTick();
    uint32_t caution_toggle = HAL_GetTick();
    char message[64];
    uint8_t caution_led = 0;

    peripherals_init();
    print_line("HC-SR04 distance monitor ready\r\n");

    for (;;) {
        uint32_t now = HAL_GetTick();
        if ((int32_t)(now - next_update) >= 0) {
            next_update = now + APP_DISTANCE_UPDATE_MS;
            if (!measure_distance_cm(&distance_cm)) {
                /* Timeout is still calculated and handled, but remains silent on UART. */
                led_set(0);
            } else if (distance_cm < APP_DISTANCE_NEAR_CM) {
                led_set(1);
                (void)snprintf(message, sizeof(message),
                               "Distance: %lu cm\r\nSTOP: too close\r\n",
                               (unsigned long)distance_cm);
                print_line(message);
            } else if (distance_cm <= APP_DISTANCE_CAUTION_CM) {
                (void)snprintf(message, sizeof(message),
                               "Distance: %lu cm\r\nCaution: object approaching\r\n",
                               (unsigned long)distance_cm);
                print_line(message);
                if ((int32_t)(now - caution_toggle) >= 0) {
                    caution_toggle = now + 500U;
                    caution_led = (uint8_t)!caution_led;
                    led_set(caution_led);
                }
            } else {
                led_set(0);
                (void)snprintf(message, sizeof(message),
                               "Distance: %lu cm\r\nClear\r\n",
                               (unsigned long)distance_cm);
                print_line(message);
            }
        }
    }
}
