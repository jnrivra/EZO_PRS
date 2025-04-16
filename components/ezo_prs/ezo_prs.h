#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/i2c/i2c.h"
#include <deque>

namespace esphome {
namespace ezo_prs {

static const char *const TAG = "ezo_prs.sensor";

enum EzoCommandType : uint8_t {
  EZO_READ = 0,
  EZO_LED,
  EZO_DEVICE_INFORMATION,
  EZO_SLEEP,
  EZO_I2C,
  EZO_UNITS,
  EZO_DECIMALS,
  EZO_ALARM,
  EZO_CUSTOM,
  EZO_STATUS,
  EZO_OFFSET
};

class EzoCommand {
 public:
  std::string command;
  uint16_t delay_ms = 0;
  bool command_sent = false;
  EzoCommandType command_type;
};

class EZOPRSSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; };

  // I2C
  void set_address(uint8_t address);

  // Device Information
  void get_device_information();
  void add_device_information_callback(std::function<void(std::string)> &&callback) {
    this->device_information_callback_.add(std::move(callback));
  }

  // Sleep
  void set_sleep();

  // R
  void get_state();

  // LED
  void get_led_state();
  void set_led_state(bool on);
  void add_led_state_callback(std::function<void(bool)> &&callback) { this->led_callback_.add(std::move(callback)); }

  // Pressure Units
  void set_pressure_unit(const std::string &unit);

  // Decimal Places
  void set_decimal_places(int places);

  // Custom
  void send_custom(const std::string &to_send);
  void add_custom_callback(std::function<void(std::string)> &&callback) {
    this->custom_callback_.add(std::move(callback));
  }
  
  // Calibration
  void calibrate_zero() { this->add_command_("Cal,0", EzoCommandType::EZO_CUSTOM, 900); }
  void clear_calibration() { this->add_command_("Cal,clear", EzoCommandType::EZO_CUSTOM, 300); }
  void check_calibration() { this->add_command_("Cal,?", EzoCommandType::EZO_CUSTOM, 300); }

  // Status
  void get_status() { this->add_command_("Status", EzoCommandType::EZO_STATUS, 300); }

  // Offset
  void set_offset(float offset) {
    std::string command = str_sprintf("O,%f", offset);
    this->add_command_(command, EzoCommandType::EZO_OFFSET, 300);
  }
  void get_offset() { this->add_command_("O,?", EzoCommandType::EZO_OFFSET, 300); }
  void clear_offset() { this->add_command_("O,0", EzoCommandType::EZO_OFFSET, 300); }

  // Text Sensor for info
  void set_info_text_sensor(text_sensor::TextSensor *info_text_sensor) {
    this->info_text_sensor_ = info_text_sensor;
  }

  // Status Text Sensor
  void set_status_text_sensor(text_sensor::TextSensor *status_text_sensor) {
    this->status_text_sensor_ = status_text_sensor;
  }

 protected:
  std::deque<std::unique_ptr<EzoCommand>> commands_;
  int new_address_;
  text_sensor::TextSensor *info_text_sensor_{nullptr};
  text_sensor::TextSensor *status_text_sensor_{nullptr};

  void add_command_(const std::string &command, EzoCommandType command_type, uint16_t delay_ms = 300);

  CallbackManager<void(std::string)> device_information_callback_{};
  CallbackManager<void(std::string)> custom_callback_{};
  CallbackManager<void(bool)> led_callback_{};

  uint32_t start_time_ = 0;
};

}  // namespace ezo_prs
}  // namespace esphome