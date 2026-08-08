# HC-SR04 Distance Monitor

This STM32G031 firmware measures an HC-SR04 ultrasonic sensor every 500 ms and reports the result over the ST-Link virtual COM port at 115200 baud.

## Hardware map

| Function | Pin |
|---|---|
| HC-SR04 trigger | PA0 |
| HC-SR04 echo | PA1 |
| Serial TX | PA2 / USART2_TX |
| Serial RX | PA3 / USART2_RX |
| Onboard LED | PA5, active high |

The HC-SR04 echo signal must be limited to 3.3 V before connecting it to PA1.

## Behavior

- Below 10 cm: LED solid on and `STOP: too close`
- 10–30 cm inclusive: LED blinks slowly and `Caution: object approaching`
- Above 30 cm: LED off and `Clear`
- No valid echo: LED off and `Distance: timeout`

TIM2 runs as a 1 MHz free-running counter for microsecond echo-pulse measurement. The measurement is bounded by a 30 ms timeout so a missing sensor cannot hang the application.

## Build and deploy

Build with the STM32 CMake/Ninja workflow. Flash through the connected ST-Link and open the detected ST-Link Virtual COM Port at 115200 baud, 8 data bits, no parity, and one stop bit.

Application logic is in `firmware/app/app.c`; board and sensor constants are in `firmware/configs/app_config.h`. The generated STM32 startup, clock, HAL configuration, and linker files are under `Core/` and the project root.
