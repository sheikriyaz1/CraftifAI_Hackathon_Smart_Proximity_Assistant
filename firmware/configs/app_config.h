#ifndef APP_CONFIG_H
#define APP_CONFIG_H
// On-board LED pins grounded from the board BSP (G0). APP_LED_GPIO_PIN is a HAL pin MASK
// (GPIO_PIN_x) — use it directly with HAL_GPIO_TogglePin/WritePin; do NOT shift it. Nucleo user
// LEDs are active-high (GPIO_PIN_SET = on). All on-board LEDs are listed below.
#define APP_LED_GPIO_PORT   GPIOA
#define APP_LED_GPIO_PIN    GPIO_PIN_5
#define APP_LED_ACTIVE_HIGH 1
#define APP_LED4_GPIO_PORT  GPIOA
#define APP_LED4_GPIO_PIN   GPIO_PIN_5
#define APP_LED_PERIOD_MS 200

/* HC-SR04 connections. The echo input must not exceed 3.3 V. */
#define APP_HCSR04_TRIGGER_PORT GPIOA
#define APP_HCSR04_TRIGGER_PIN  GPIO_PIN_0
#define APP_HCSR04_ECHO_PORT    GPIOA
#define APP_HCSR04_ECHO_PIN     GPIO_PIN_1
#define APP_DISTANCE_UPDATE_MS  500U
#define APP_DISTANCE_NEAR_CM    10U
#define APP_DISTANCE_CAUTION_CM 30U
#define APP_ECHO_TIMEOUT_US     30000U
#define APP_UART_BAUDRATE       115200U
#define APP_UART_INSTANCE       USART2
#define APP_UART_TX_PORT        GPIOA
#define APP_UART_TX_PIN         GPIO_PIN_2
#define APP_UART_RX_PORT        GPIOA
#define APP_UART_RX_PIN         GPIO_PIN_3
#define APP_BUTTON_PRESENT      0
#endif
