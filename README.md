# Robotics Kit — Firmware

PlatformIO firmware for the **Robotics Kit**, built on the **ESP32-S3-WROOM-1**.

The full hardware/firmware interface is specified in
[`Robotics_Kit_System_Prompt (2).md`](./Robotics_Kit_System_Prompt%20(2).md).
That architecture is approved and standard — this repository implements it
exactly.

## Hardware

- MCU: ESP32-S3-WROOM-1
- 4 motors across 2 motor drivers (front and rear, independently driven on each side)
- 1 ultrasonic distance sensor, 2 digital IR line sensors, 1 microphone, 1 battery monitor
- 1 I²C OLED, 1 buzzer, 1 servo, 4 addressable LEDs

### Approved pin map

| Device | Signal | GPIO | Resource |
|---|---|---:|---|
| Microphone | Audio input | GPIO1 | ADC1 |
| Battery monitor | Voltage sense | GPIO2 | ADC1 |
| Front-Left motor (Driver 1) | PWM / speed | GPIO4 | PWM (LEDC) |
| Front-Left motor (Driver 1) | Direction | GPIO5 | Digital GPIO |
| Rear-Left motor (Driver 1) | PWM / speed | GPIO6 | PWM (LEDC) |
| Rear-Left motor (Driver 1) | Direction | GPIO7 | Digital GPIO |
| Front-Right motor (Driver 2) | PWM / speed | GPIO8 | PWM (LEDC) |
| Front-Right motor (Driver 2) | Direction | GPIO9 | Digital GPIO |
| Rear-Right motor (Driver 2) | PWM / speed | GPIO10 | PWM (LEDC) |
| Rear-Right motor (Driver 2) | Direction | GPIO11 | Digital GPIO |
| Ultrasonic | Trigger | GPIO12 | Digital output |
| Ultrasonic | Echo | GPIO13 | Digital input + timing capture |
| Left IR sensor | Line signal | GPIO14 | Digital GPIO |
| Right IR sensor | Line signal | GPIO15 | Digital GPIO |
| OLED | SDA | GPIO16 | I²C data |
| OLED | SCL | GPIO17 | I²C clock |
| Buzzer | Audio output | GPIO18 | PWM (LEDC) |
| Servo | Control signal | GPIO21 | PWM (LEDC) |
| Addressable LEDs | Data | GPIO38 | Digital output |

Only the microphone and battery monitor need analog readings, so they are the
only signals on ADC-capable pins (GPIO1–2, ADC1); ADC2 is unused, so the
ADC2 + Wi-Fi conflict cannot occur. Strapping pins (0, 3, 45, 46), native-USB
pins (19, 20), SPI flash/PSRAM pins (26–37) and UART0 (43, 44) are deliberately
unused.

## Architecture

`config → driver → controller → application`

```
include/config.h            the only file containing raw GPIO numbers
include/ + src/             motor, ultrasonic, line_sensor, display, buzzer,
                            microphone, led, servo, battery   (driver layer)
include/ + src/robot_controller.*   controller layer
src/main.cpp                application layer: FreeRTOS tasks
```

Application code never references a GPIO number; all hardware is reached
through a named module.

The `motor` module treats the drive system as four independently driven
motors, and exposes per-side control, per-corner control and independent
front/rear speed control on each side.

## Task model (FreeRTOS)

| Task | Priority | Core | Responsibility |
|---|---|---|---|
| Sensor Task | Medium (3) | 1 | Reads ultrasonic, IR, microphone and battery; timestamps readings |
| Control Task | **Highest (4)** | 1 | Consumes sensor data, applies safety limits, commands all 4 motors |
| UI Task | Low–medium (2) | 0 | OLED, LEDs, buzzer; never blocks or delays the Control Task |
| Application Task | Medium (3) | 0 | Startup, mode switching, demonstrations |

Data flow: **Sensors → Sensor Task → Control Task → Motors (x4) / Servo / UI**

Samples move through a depth-1 mailbox queue (`xQueueOverwrite` /
`xQueuePeek` with zero timeout), so neither task ever blocks on the other, and
UI work runs on the other core.

## Safety rules enforced in code

1. All 4 motors default to STOP on startup (`motor::init` → `stopAll`).
2. All 4 motors stop on any detected fault (`robot_controller::raiseFault`).
3. A sample older than `SENSOR_MAX_AGE_MS` never produces a motor command.
4. Invalid sensor data never produces a motor command.
5. Speeds are clamped in the controller (`CONTROL_SPEED_LIMIT`) and again in
   the driver (`MOTOR_SPEED_MIN/MAX`).
6. / 7. UI work never delays the Control Task: no blocking UI calls, lower
   priority, separate core.
8. GPIO assignments come only from `include/config.h`.
9. No unapproved GPIO is used anywhere.

## Operating modes

`IDLE → LINE_FOLLOW → OBSTACLE_AVOID → DEMO`, advanced by a loud sound on the
microphone. A loud sound while faulted clears the fault and returns to `IDLE`.

## Build

```sh
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial monitor @ 115200
```
