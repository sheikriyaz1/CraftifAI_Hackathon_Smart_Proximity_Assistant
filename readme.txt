# Smart Proximity Assistant

An STM32-based smart proximity/parking assistant built with **FirmGen** (CraftifAI Orbit) for the FirmGen Hackathon. It uses an ultrasonic distance sensor to give real-time, three-zone proximity feedback through an onboard LED and a live UART status feed.

---

## Demo video
I have given in vedio section plz watch it.

---

## Problem definition & target users

Reversing a vehicle, docking equipment, or positioning objects near a wall/obstacle is hard to judge by eye alone, and cheap physical parking sensors are either bulky, expensive, or not programmable. This project is a **minimal, low-cost proximity-warning module** anyone can build with an STM32 dev board and a single ultrasonic sensor.

**Target users:**
- Hobbyists building DIY parking-assist add-ons
- Makers needing a simple "keep-out zone" sensor for robotics or automation projects
- Students/educators demonstrating real-time embedded sensing

## What it does

The system continuously measures distance and reacts in three zones:

| Distance | LED behavior | UART message |
|---|---|---|
| `> 30 cm` | Off | `Clear` |
| `10–30 cm` | Slow blink | `Caution: object approaching` |
| `< 10 cm` | Solid on | `STOP: too close` |
| No echo received | — | `Distance: timeout` |

Readings and status update every **500 ms**, printed over UART at **115200 baud**.

---

## Hardware / bill of materials

| Item | Qty | Notes |
|---|---|---|
| STM32 Nucleo-G071RB | 1 | Board used for this build (`BOARD G0`, device `STM32G07x/STM32G08x`) |
| HC-SR04 ultrasonic distance sensor | 1 | 5V module |
| Resistors | 1x 1kΩ, 1x 2kΩ | Voltage divider for the echo line |
| Jumper wires | ~6 | |
| USB cable (ST-Link) | 1 | Power + flashing + UART monitor |

## Wiring
I have given in image section.


| HC-SR04 pin | Nucleo-G071RB pin | Notes |
|---|---|---|
| VCC | 5V | |
| TRIG | PA0 | Direct connection, 3.3V trigger is sufficient |
| ECHO | PA1 (via divider) | **Required:** 1kΩ in series from ECHO→PA1, 2kΩ from that junction to GND. Steps the 5V echo signal down to ~3.3V to protect the GPIO. |
| GND | GND | |

UART (USART2) is broken out over the ST-Link virtual COM port — no extra wiring needed to view console output; just connect via USB.

---

## Software architecture

```
firmware/
├── Core/
│   ├── Inc/            # Header files, project config
│   └── Src/
│       ├── main.c        # Application entry point, main loop
│       └── ...            # HAL init, peripheral config
├── Drivers/
│   ├── CMSIS/
│   └── STM32G0xx_HAL_Driver/
├── docs/
│   └── api/               # Auto-generated Doxygen documentation
└── build/                  # CMake/ninja build output, .bin/.elf
```

Sensor timing, echo pulse measurement, and LED/UART status logic are implemented in the application layer, with HAL-specific setup isolated per FirmGen's generated platform structure.

---

## Build & flash instructions

This project was built using **FirmGen v0.3.1**.

1. Open the project in FirmGen and select **Board G0 / STM32G07X-STM32G08X**.
2. Review the generated task list under the **Planning** stage.
3. Click **Build, flash & monitor** to compile, flash over ST-Link, and open the serial console in one step.
4. If prompted about a missing `pyserial` dependency for the monitor, run:
   ```
   python -m pip install pyserial
   ```
5. Connect the hardware per the wiring table above, then power the board via USB.
6. Watch the console (COM port at 115200 baud) for live distance and zone status as you move an object toward the sensor.

---

## FirmGen task list & iteration history

> **[Insert screenshots of the FirmGen task list and chat/plan iterations here]**

The FirmGen chat export for this build is included in this repo as `firmgen_chat_export.json` (or `.md`), documenting the prompts used to scaffold, debug, and refine the firmware.

---

## Limitations

- The 1kΩ/2kΩ voltage divider is a fixed ratio; it is not a substitute for a logic-level shifter if reused with sensors of a different output voltage.
- Distance readings assume a single, direct reflective surface — angled or soft/absorptive surfaces can produce inaccurate or missing echoes (reported as `Distance: timeout`).
- No debouncing/smoothing filter is applied to raw distance readings, so a single noisy reading can briefly flip the reported zone.
- Onboard user button handling is not enabled in this build.

## Future improvements

- Add a rolling average filter to smooth distance readings.
- Add a buzzer for audible feedback in the `STOP` zone.
- Support multiple sensors for wider-angle coverage.

---

## Built with

[FirmGen](https://craftifai.com) by CraftifAI Orbit — AI-assisted embedded firmware generation for STM32 and ESP32.
