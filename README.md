# EZO-PRS — ESPHome Component for the Atlas Scientific Pressure Sensor

> An [ESPHome](https://esphome.io/) external component that turns an
> [Atlas Scientific EZO-PRS](https://atlas-scientific.com/) embedded pressure
> sensor into a first-class, I²C-connected sensor for ESP32 and Home Assistant.

[![ESPHome](https://img.shields.io/badge/ESPHome-component-000000?logo=esphome&logoColor=white)](https://esphome.io/)
[![Platform: ESP32](https://img.shields.io/badge/platform-ESP32-blue?logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![Bus: I²C](https://img.shields.io/badge/bus-I²C-orange)](https://en.wikipedia.org/wiki/I%C2%B2C)
[![Validate](https://github.com/jnrivra/EZO_PRS/actions/workflows/validate.yml/badge.svg)](https://github.com/jnrivra/EZO_PRS/actions/workflows/validate.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## Overview

The Atlas Scientific **EZO-PRS** is a high-precision embedded pressure sensor
that speaks the Atlas Scientific EZO command protocol over I²C. This repository
provides a complete, native ESPHome component (`ezo_prs`) so you can read and
control the sensor declaratively from YAML — no custom firmware required.

With this component you can:

- 📈 **Read pressure** in your choice of unit — `psi`, `atm`, `bar`, `kPa`,
  `inH₂O` or `cmH₂O`
- 🔢 **Configure precision** from 0 to 3 decimal places
- 🛠️ **Calibrate** the zero and high points, and clear calibration
- ⚖️ **Manage offsets** (set / read / clear)
- 💡 **Control the on-board LED**
- ℹ️ **Read device information and status** into Home Assistant text sensors
- ⌨️ **Send any raw EZO command** for full low-level control

> 📄 Full hardware specifications, command set and wiring are in the included
> [**EZO-PRS Datasheet (PDF)**](EZO-PRS-Datasheet.pdf).

## Supported hardware

| Item        | Detail                                              |
|-------------|-----------------------------------------------------|
| Sensor      | Atlas Scientific EZO-PRS embedded pressure sensor   |
| Bus         | I²C (default address `0x6A`)                         |
| MCU         | ESP32 (`esp32dev` and compatible boards)            |
| Framework   | ESPHome                                             |

## Hardware setup

Wire the EZO-PRS to your ESP32 over I²C:

| EZO-PRS | ESP32                          |
|---------|--------------------------------|
| VCC     | 3.3 V                          |
| GND     | GND                            |
| SDA     | any I²C-capable GPIO (SDA)     |
| SCL     | any I²C-capable GPIO (SCL)     |

The I²C pins are configurable. The bundled [`example.yaml`](example.yaml) uses
`SDA = GPIO15` and `SCL = GPIO16`; the common ESP32 defaults are
`SDA = GPIO21` / `SCL = GPIO22`. Pick whatever your board exposes and set them
in the `i2c:` block.

## Installation

### Method 1 — External component (recommended)

Point ESPHome at this repository and it will pull the component automatically:

```yaml
external_components:
  - source: github://jnrivra/EZO_PRS@main
    components: [ezo_prs]
```

### Method 2 — Reusable package

Include the ready-made package, which adds the external component **and** a
configured pressure sensor in one line (your config must define an I²C bus with
`id: bus_a`):

```yaml
packages:
  ezo_prs: github://jnrivra/EZO_PRS/ezo_prs.yaml@main
```

### Method 3 — Manual installation

Copy the component into your ESPHome configuration folder:

```
/config/
└── esphome/
    └── components/
        └── ezo_prs/
            ├── __init__.py
            ├── sensor.py
            ├── ezo_prs.h
            └── ezo_prs.cpp
```

## Usage

A minimal device configuration:

```yaml
i2c:
  - id: bus_a
    sda: 15
    scl: 16
    scan: true

sensor:
  - platform: ezo_prs
    i2c_id: bus_a
    id: pressure_sensor
    name: "Pressure Sensor"
    address: 0x6A          # default EZO-PRS I²C address
    update_interval: 5s
    pressure_unit: cmh2o   # psi | atm | bar | kpa | inh2o | cmh2o
    decimals: 3            # 0-3

    # Optional: surface device info / status into Home Assistant
    info_text_sensor: sensor_info
    status_text_sensor: sensor_status

text_sensor:
  - platform: template
    name: "Pressure Sensor Info"
    id: sensor_info
  - platform: template
    name: "Pressure Sensor Status"
    id: sensor_status
```

For a complete, real-world configuration with calibration buttons, status
monitoring and unit switching, see [`example.yaml`](example.yaml).

### Configuration options

| Option              | Default  | Description                                                                    |
|---------------------|----------|--------------------------------------------------------------------------------|
| `pressure_unit`     | `cmh2o`  | Measurement unit: `psi`, `atm`, `bar`, `kpa`, `inh2o`, `cmh2o`                  |
| `decimals`          | `3`      | Number of decimal places in readings (0-3)                                      |
| `address`           | `0x6A`   | I²C address of the EZO-PRS sensor                                               |
| `update_interval`   | `5s`     | How often to poll the sensor                                                    |
| `info_text_sensor`  | optional | Existing `text_sensor` to receive device information                            |
| `status_text_sensor`| optional | Existing `text_sensor` to receive device status                                 |

> The component is also a standard ESPHome `sensor`, so all common options
> (`filters`, `accuracy_decimals`, `device_class`, etc.) are available too.

## Advanced usage — lambda methods

Every command is exposed as a method you can call from a lambda (for example
from a template `button`):

```yaml
button:
  - platform: template
    name: "Calibrate Zero"
    on_press:
      - lambda: 'id(pressure_sensor).calibrate_zero();'

  - platform: template
    name: "Clear Calibration"
    on_press:
      - lambda: 'id(pressure_sensor).clear_calibration();'
```

### Available methods

| Method                     | Description                                  |
|----------------------------|----------------------------------------------|
| `calibrate_zero()`         | Calibrate the zero point                     |
| `clear_calibration()`      | Clear all calibration data                   |
| `check_calibration()`      | Check current calibration                    |
| `get_status()`             | Request sensor status                        |
| `set_offset(float)`        | Set the offset value                         |
| `get_offset()`             | Read the current offset                      |
| `clear_offset()`           | Clear the offset                             |
| `set_led_state(bool)`      | Turn the on-board LED on / off               |
| `get_led_state()`          | Read the LED state                           |
| `set_pressure_unit(string)`| Change the active pressure unit              |
| `get_device_information()` | Read device firmware / model information     |
| `send_custom(string)`      | Send any raw EZO command (e.g. `"Cal,30"`)   |

## WiFi & secrets

The example config keeps all credentials out of version control using ESPHome's
`!secret` mechanism. To run it:

```bash
cp secrets.yaml.example secrets.yaml   # then edit with your own values
```

`secrets.yaml` is gitignored — **never commit real credentials**.

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file.

## Acknowledgements

- [Atlas Scientific](https://atlas-scientific.com/) — for the EZO-PRS sensor
- [ESPHome](https://esphome.io/) — for the framework

## Contributing

Contributions are welcome. Feel free to open an issue or submit a pull request.
