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

void DMXComponent::loop() {
  if (this->mode_ == DMX_MODE_RECEIVE) {
    uint32_t now = millis();
    if (now - this->last_read_time_ >= this->read_interval_ms_) {
      this->last_read_time_ = now;
      
      // Read DMX data from the bus into our buffer
      dmx_packet_t packet;
      if (dmx_receive(this->dmx_port_id_, &packet, DMX_TIMEOUT_TICK) > 0) {
        // Copy received data to our buffer
        dmx_read(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
      }
    }
  }
}

void DMXComponent::send_data() {
  dmx_write(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
  dmx_send(this->dmx_port_id_);
  dmx_wait_sent(this->dmx_port_id_, DMX_TIMEOUT_TICK);
}

void DMXComponent::write_universe(const uint8_t *data, size_t length) {
  // Zero byte in DMX packet is the DMX start code. Always 0x00.
  this->dmx_data_[0] = 0x00;
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
  ESP_LOGCONFIG(TAG, "  Mode: %s", this->mode_ == DMX_MODE_SEND ? "SEND" : "RECEIVE");
  if (this->mode_ == DMX_MODE_RECEIVE) {
    ESP_LOGCONFIG(TAG, "  Read Interval: %d ms", this->read_interval_ms_);
  }
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
  // Zero slot in DMX packet is the DMX start code. Don't offset by -1.
  this->dmx_data_[channel] = value;
}

uint8_t DMXComponent::read_channel(uint16_t channel) {
  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return 0;
  }
  // Zero slot in DMX packet is the DMX start code. Don't offset by -1.
  return this->dmx_data_[channel];
}

}  // namespace esphome::dmx
