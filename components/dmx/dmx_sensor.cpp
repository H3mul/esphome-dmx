#include "dmx_sensor.h"
#include "esphome/core/log.h"

namespace esphome::dmx {

static const char *const TAG = "dmx.sensor";

void DMXSensor::setup() { ESP_LOGCONFIG(TAG, "Setting up DMX Sensor..."); }

void DMXSensor::loop() {
  if (this->parent_ == nullptr) {
    return;
  }

  // Read the current value from the DMX component
  uint8_t value = this->parent_->read_channel(this->channel_);

  // Only publish if the value has changed
  if (value != this->last_value_) {
    this->last_value_ = value;
    this->publish_state(static_cast<float>(value));
  }
}

void DMXSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "DMX Sensor:");
  ESP_LOGCONFIG(TAG, "  Channel: %d", this->channel_);
  LOG_SENSOR("  ", "DMX Sensor", this);
}

} // namespace esphome::dmx
