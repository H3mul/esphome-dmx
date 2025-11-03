#include "dmx_output.h"
#include "esphome/core/log.h"

namespace esphome::dmx {

static const char *const TAG = "dmx.output";

void DMXOutput::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DMX Output...");
  // Initialize to 0
  this->write_state(0.0f);
}

void DMXOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "DMX Output:");
  ESP_LOGCONFIG(TAG, "  Channel: %d", this->channel_);
}

void DMXOutput::write_state(float state) {
  // Convert float (0.0-1.0) to uint8_t (0-255)
  uint8_t value = static_cast<uint8_t>(state * 255.0f);
  
  if (this->parent_ != nullptr) {
    this->parent_->write_channel(this->channel_, value);
  } else {
    ESP_LOGW(TAG, "DMX parent not set!");
  }
}

}  // namespace esphome::dmx
