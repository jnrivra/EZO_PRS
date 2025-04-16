# EZO PRS ESPHome Component

This repository contains an ESPHome component for the Atlas Scientific EZO PRS pressure sensor. This component allows you to easily integrate the EZO PRS sensor into your Home Assistant setup using ESPHome and ESP32 devices.

## Overview

The Atlas Scientific EZO PRS is a high-precision pressure sensor that can be connected to an ESP32 via I2C. This component provides a complete implementation for ESPHome, allowing you to:

- Read pressure values with configurable units (PSI, ATM, BAR, kPa, inH₂O, cmH₂O)
- Configure decimal precision
- Access device information and status
- Perform calibration operations
- Control the LED state
- Set and manage offset values

## Installation

### Method 1: External Component

1. Add the following to your ESPHome YAML configuration:

```yaml
external_components:
  - source: github://jnrivra/EZO_PRS@main
    components: [ ezo_prs ]
```

### Method 2: Manual Installation

1. Create the following directory structure in your ESPHome configuration folder:
```
/config/
└──esphome/
    └── components/
        └── ezo_prs/
            ├── __init__.py
            ├── sensor.py
            ├── ezo_prs.h
            └── ezo_prs.cpp
```

2. Copy the files from this repository into the corresponding locations.

## Configuration

Add the following to your ESPHome device configuration file:

```yaml
# Example configuration
sensor:
  - platform: ezo_prs
    name: "Pressure Sensor"
    address: 0x6A  # Default address for EZO PRS sensor
    update_interval: 5s
    pressure_unit: cmh2o  # Options: psi, atm, bar, kpa, inh2o, cmh2o
    decimals: 3  # Number of decimal places (0-3)
    
    # Optional: Text sensors for additional information
    info_text_sensor: info_sensor
    status_text_sensor: status_sensor

# Optional text sensors for additional information
text_sensor:
  - platform: template
    name: "Pressure Sensor Info"
    id: info_sensor
    
  - platform: template
    name: "Pressure Sensor Status"
    id: status_sensor
```

## Available Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `pressure_unit` | `cmh2o` | The unit of measurement. Options: `psi`, `atm`, `bar`, `kpa`, `inh2o`, `cmh2o` |
| `decimals` | `3` | Number of decimal places in readings (0-3) |
| `info_text_sensor` | optional | Text sensor to display device information |
| `status_text_sensor` | optional | Text sensor to display device status |
| `address` | `0x6A` | I2C address of the EZO PRS sensor |
| `update_interval` | `5s` | How often to poll the sensor |

## Advanced Usage

### Device Methods

The following methods are available in the component for advanced usage in lambdas:

```yaml
# Example of using advanced methods in lambdas
button:
  - platform: template
    name: "Calibrate Zero"
    on_press:
      then:
        - lambda: |-
            id(pressure_sensor).calibrate_zero();
            
  - platform: template
    name: "Clear Calibration"
    on_press:
      then:
        - lambda: |-
            id(pressure_sensor).clear_calibration();
```

## Supported Methods

| Method | Description |
|--------|-------------|
| `calibrate_zero()` | Calibrate the zero point |
| `clear_calibration()` | Clear all calibration data |
| `check_calibration()` | Check current calibration |
| `get_status()` | Get sensor status |
| `set_offset(float)` | Set offset value |
| `get_offset()` | Get current offset |
| `clear_offset()` | Clear offset |
| `set_led_state(bool)` | Turn LED on/off |
| `get_led_state()` | Get LED state |
| `get_device_information()` | Get device information |
| `send_custom(string)` | Send custom command |

## Hardware Setup

Connect the EZO PRS sensor to your ESP32 using I2C:

| EZO PRS | ESP32 |
|---------|-------|
| VCC     | 3.3V  |
| GND     | GND   |
| SDA     | SDA (GPIO21) |
| SCL     | SCL (GPIO22) |

## Real-world Example

For a more comprehensive example including additional features like sensor calibration, status monitoring, and unit switching, see the `example.yaml` file in this repository.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgements

- [Atlas Scientific](https://atlas-scientific.com/) for the EZO PRS sensor
- [ESPHome](https://esphome.io/) for the amazing framework

## Contributing

Contributions to improve the component are welcome. Please feel free to submit a pull request or open an issue on GitHub. 