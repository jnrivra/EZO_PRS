#include "ezo_prs.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ezo_prs {

static const char *const EZO_COMMAND_TYPE_STRINGS[] = {
    "EZO_READ", "EZO_LED", "EZO_DEVICE_INFORMATION", "EZO_SLEEP",
    "EZO_I2C", "EZO_UNITS", "EZO_DECIMALS", "EZO_ALARM", "EZO_CUSTOM", 
    "EZO_STATUS", "EZO_OFFSET"};

void EZOPRSSensor::setup() {
  // Inicialización
  ESP_LOGD(TAG, "Inicializando sensor EZO-PRS");
  
  // Configurar unidad cmH2O al iniciar
  this->set_pressure_unit("cmh2o");
  
  // Obtener información del dispositivo
  this->get_device_information();
}

void EZOPRSSensor::dump_config() {
  LOG_SENSOR("", "EZO PRS", this);
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with EZO circuit failed!");
  }
  LOG_UPDATE_INTERVAL(this);
}

void EZOPRSSensor::update() {
  // Check if a read is in there already and if not insert one in the second position
  if (!this->commands_.empty() && this->commands_.front()->command_type != EzoCommandType::EZO_READ &&
      this->commands_.size() > 1) {
    bool found = false;

    for (auto &i : this->commands_) {
      if (i->command_type == EzoCommandType::EZO_READ) {
        found = true;
        break;
      }
    }

    if (!found) {
      std::unique_ptr<EzoCommand> ezo_command(new EzoCommand);
      ezo_command->command = "R";
      ezo_command->command_type = EzoCommandType::EZO_READ;
      ezo_command->delay_ms = 900;

      auto it = this->commands_.begin();
      ++it;
      this->commands_.insert(it, std::move(ezo_command));
    }

    return;
  }

  this->get_state();
}

void EZOPRSSensor::loop() {
  if (this->commands_.empty()) {
    return;
  }

  EzoCommand *to_run = this->commands_.front().get();

  if (!to_run->command_sent) {
    const uint8_t *data = reinterpret_cast<const uint8_t *>(to_run->command.c_str());
    ESP_LOGVV(TAG, "Sending command \"%s\"", data);

    this->write(data, to_run->command.length());

    if (to_run->command_type == EzoCommandType::EZO_SLEEP ||
        to_run->command_type == EzoCommandType::EZO_I2C) {  // Commands with no return data
      this->commands_.pop_front();
      if (to_run->command_type == EzoCommandType::EZO_I2C)
        this->address_ = this->new_address_;
      return;
    }

    this->start_time_ = millis();
    to_run->command_sent = true;
    return;
  }

  if (millis() - this->start_time_ < to_run->delay_ms)
    return;

  uint8_t buf[32];

  buf[0] = 0;

  if (!this->read_bytes_raw(buf, 32)) {
    ESP_LOGE(TAG, "read error");
    this->commands_.pop_front();
    return;
  }

  switch (buf[0]) {
    case 1:
      break;
    case 2:
      ESP_LOGE(TAG, "device returned a syntax error");
      break;
    case 254:
      return;  // keep waiting
    case 255:
      ESP_LOGE(TAG, "device returned no data");
      break;
    default:
      ESP_LOGE(TAG, "device returned an unknown response: %d", buf[0]);
      break;
  }

  ESP_LOGV(TAG, "Received buffer \"%s\" for command type %s", &buf[1], EZO_COMMAND_TYPE_STRINGS[to_run->command_type]);

  if (buf[0] == 1) {
    std::string payload = reinterpret_cast<char *>(&buf[1]);
    if (!payload.empty()) {
      auto start_location = payload.find(',');
      switch (to_run->command_type) {
        case EzoCommandType::EZO_READ: {
          // some sensors return multiple comma-separated values, terminate string after first one
          if (start_location != std::string::npos) {
            payload.erase(start_location);
          }
          auto val = parse_number<float>(payload);
          if (!val.has_value()) {
            ESP_LOGW(TAG, "Can't convert '%s' to number!", payload.c_str());
          } else {
            this->publish_state(*val);
          }
          break;
        }
        case EzoCommandType::EZO_LED:
          this->led_callback_.call(payload.back() == '1');
          break;
        case EzoCommandType::EZO_DEVICE_INFORMATION:
          if (start_location != std::string::npos) {
            this->device_information_callback_.call(payload.substr(start_location + 1));
            // También actualizar el sensor de texto si está disponible
            if (this->info_text_sensor_ != nullptr) {
              this->info_text_sensor_->publish_state(payload);
            }
          }
          break;
        case EzoCommandType::EZO_STATUS:
          ESP_LOGI(TAG, "Estado del sensor: %s", payload.c_str());
          if (this->status_text_sensor_ != nullptr) {
            this->status_text_sensor_->publish_state(payload);
          }
          break;
        case EzoCommandType::EZO_OFFSET:
          ESP_LOGI(TAG, "Información de offset: %s", payload.c_str());
          if (this->info_text_sensor_ != nullptr) {
            this->info_text_sensor_->publish_state("Offset: " + payload);
          }
          break;
        case EzoCommandType::EZO_ALARM:
          ESP_LOGI(TAG, "Estado de alarma: %s", payload.c_str());
          if (this->info_text_sensor_ != nullptr) {
            this->info_text_sensor_->publish_state("Alarm: " + payload);
          }
          break;
        case EzoCommandType::EZO_CUSTOM:
          this->custom_callback_.call(payload);
          // Para visualizar respuestas del sensor en el log
          ESP_LOGI(TAG, "Respuesta a comando personalizado: %s", payload.c_str());
          // Actualizar el sensor de texto si está disponible
          if (this->info_text_sensor_ != nullptr) {
            this->info_text_sensor_->publish_state(payload);
          }
          break;
        default:
          break;
      }
    }
  }
  this->commands_.pop_front();
}

void EZOPRSSensor::add_command_(const std::string &command, EzoCommandType command_type, uint16_t delay_ms) {
  std::unique_ptr<EzoCommand> ezo_command(new EzoCommand);
  ezo_command->command = command;
  ezo_command->command_type = command_type;
  ezo_command->delay_ms = delay_ms;
  this->commands_.push_back(std::move(ezo_command));
}

void EZOPRSSensor::set_address(uint8_t address) {
  if (address > 0 && address < 128) {
    std::string payload = str_sprintf("I2C,%u", address);
    this->new_address_ = address;
    this->add_command_(payload, EzoCommandType::EZO_I2C);
  } else {
    ESP_LOGE(TAG, "Invalid I2C address");
  }
}

void EZOPRSSensor::get_device_information() { this->add_command_("i", EzoCommandType::EZO_DEVICE_INFORMATION); }

void EZOPRSSensor::set_sleep() { this->add_command_("Sleep", EzoCommandType::EZO_SLEEP); }

void EZOPRSSensor::get_state() { this->add_command_("R", EzoCommandType::EZO_READ, 900); }

void EZOPRSSensor::get_led_state() { this->add_command_("L,?", EzoCommandType::EZO_LED); }

void EZOPRSSensor::set_led_state(bool on) {
  std::string to_send = "L,";
  to_send += on ? "1" : "0";
  this->add_command_(to_send, EzoCommandType::EZO_LED);
}

void EZOPRSSensor::set_pressure_unit(const std::string &unit) {
  std::string to_send = "U,";
  to_send += unit;
  this->add_command_(to_send, EzoCommandType::EZO_UNITS);
}

void EZOPRSSensor::set_decimal_places(int places) {
  if (places >= 0 && places <= 3) {
    std::string to_send = str_sprintf("Dec,%d", places);
    this->add_command_(to_send, EzoCommandType::EZO_DECIMALS);
  } else {
    ESP_LOGE(TAG, "Invalid decimal places: %d (valid range 0-3)", places);
  }
}

void EZOPRSSensor::send_custom(const std::string &to_send) { this->add_command_(to_send, EzoCommandType::EZO_CUSTOM); }

}  // namespace ezo_prs
}  // namespace esphome