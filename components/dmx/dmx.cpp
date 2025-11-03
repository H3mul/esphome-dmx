#include "dmx.h"
#include "dmx/include/types.h"
#include "esphome/core/log.h"
#include <string.h>

namespace esphome::dmx {

static const char *const TAG = "dmx";

void DMXComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DMX...");

  // Configure DMX driver
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {{1, "Default"}};
  int personality_count = 1;

  // Set up the DMX driver
  dmx_driver_install(this->dmx_port_id_, &config, personalities, personality_count);

  // Set DMX pins
  dmx_set_pin(this->dmx_port_id_, this->tx_pin_->get_pin(), this->rx_pin_->get_pin(),
              this->enable_pin_ != nullptr ? this->enable_pin_->get_pin() : -1);

  // Initialize DMX data array
  memset(this->dmx_data_, 0, DMX_PACKET_SIZE);

  this->initialized_ = true;
  ESP_LOGCONFIG(TAG, "DMX setup complete");
}

void DMXComponent::loop() {
  if (!this->initialized_) {
    return;
  }

  dmx_write(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
  dmx_send(this->dmx_port_id_);
  dmx_wait_sent(this->dmx_port_id_, DMX_TIMEOUT_TICK);
}

void DMXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DMX:");
  ESP_LOGCONFIG(TAG, "  Port: %d", this->dmx_port_id_);
  LOG_PIN("  TX Pin: ", this->tx_pin_);
  LOG_PIN("  RX Pin: ", this->rx_pin_);
  if (this->enable_pin_ != nullptr) {
    LOG_PIN("  Enable Pin: ", this->enable_pin_);
  }
}

void DMXComponent::write_channel(uint16_t channel, uint8_t value) {
  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return;
  }
  // DMX channels are 1-indexed, but array is 0-indexed
  this->dmx_data_[channel - 1] = value;
}

uint8_t DMXComponent::read_channel(uint16_t channel) {
  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return 0;
  }
  // DMX channels are 1-indexed, but array is 0-indexed
  return this->dmx_data_[channel - 1];
}

}  // namespace esphome::dmx
