#include "dmx.h"
#include "dmx/include/types.h"
#include "esphome/core/log.h"
#include <string.h>
#include <algorithm>

namespace esphome::dmx {

static const char *const TAG = "dmx";

void DMXComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DMX...");

  // Configure DMX driver
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {};
  int personality_count = 0;
  dmx_driver_install(this->dmx_port_id_, &config, personalities, personality_count);

  dmx_set_pin(this->dmx_port_id_,
    this->tx_pin_->get_pin(),
    this->rx_pin_->get_pin(),
    this->enable_pin_->get_pin());

  memset(this->dmx_data_, 0, DMX_PACKET_SIZE);

  ESP_LOGCONFIG(TAG, "DMX setup complete");
}

void DMXComponent::loop() { }

void DMXComponent::send_data() {
  dmx_write(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
  dmx_send(this->dmx_port_id_);
  dmx_wait_sent(this->dmx_port_id_, DMX_TIMEOUT_TICK);
}

void DMXComponent::write_universe(const uint8_t *data, size_t length) {
  // Zeroth byte in DMX packet is the DMX start code. Always 0x00.
  length = std::min(length, static_cast<size_t>(DMX_PACKET_SIZE - 1));
  memcpy(this->dmx_data_ + 1, data, length);
}

void DMXComponent::send_universe(const uint8_t *data, size_t length) {
  write_universe(data, length);
  send_data();
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

void DMXComponent::send_channel(uint16_t channel, uint8_t value) {
  this->write_channel(channel, value);
  this->send_data();
}

void DMXComponent::write_channel(uint16_t channel, uint8_t value) {
  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return;
  }
  // Zeroth byte in DMX packet is the DMX start code. Don't offset by -1.
  this->dmx_data_[channel] = value;
}

}  // namespace esphome::dmx
