#include "dmx.h"
#include "dmx/include/service.h"
#include "dmx/include/types.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <string.h>

namespace esphome::dmx {

static const char *const TAG = "dmx";

void DMXComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DMX...");

  // Configure DMX driver
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {};
  int personality_count = 0;
  dmx_driver_install(this->dmx_port_id_, &config, personalities,
                     personality_count);

  dmx_set_pin(this->dmx_port_id_, this->tx_pin_->get_pin(),
              this->rx_pin_->get_pin(), this->enable_pin_->get_pin());

  memset(this->dmx_data_, 0, DMX_PACKET_SIZE);

  ESP_LOGCONFIG(TAG, "DMX setup complete");
}

void DMXComponent::loop() {
  if (!this->enabled_) {
    return;
  }

  uint32_t now = millis();

  if (this->mode_ == DMX_MODE_RECEIVE) {
    if (now - this->last_read_time_ >= this->read_interval_ms_) {
      this->last_read_time_ = now;

      // Read DMX data from the bus into our buffer
      dmx_packet_t packet;
      size_t bytes_received = dmx_receive(this->dmx_port_id_, &packet,
                                          this->receive_timeout_ticks_);
      if (bytes_received > 0) {
        // Copy received data to our buffer
        dmx_read(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
        ESP_LOGV(TAG, "DMX Received: %zu bytes", bytes_received);
      } else {
        ESP_LOGV(TAG, "DMX Receive timeout");
      }
    }
  } else if (this->mode_ == DMX_MODE_SEND) {
    if (now - this->last_send_time_ >= this->write_interval_ms_) {
      this->send_data();
    }
  }
}

void DMXComponent::send_data() {
  int dmx_status = dmx_get_status(this->dmx_port_id_);
  if (dmx_status == DMX_STATUS_SENDING) {
    ESP_LOGD(TAG,
             "DMX '%s': DMX busy with previous write (status: %d), ignoring "
             "send request",
             this->name_.c_str(), dmx_status);
    return;
  }

  ESP_LOGVV(TAG, "DMX '%s': status: %d Sending data (512 bytes)",
            this->name_.c_str(), dmx_status);
  dmx_write(this->dmx_port_id_, this->dmx_data_, DMX_PACKET_SIZE);
  dmx_send(this->dmx_port_id_);

  this->last_send_time_ = millis();
}

void DMXComponent::read_universe(uint8_t *buffer, size_t buffer_size) {
  // Copy DMX data to the provided buffer
  size_t copy_size =
      std::min(buffer_size, static_cast<size_t>(DMX_PACKET_SIZE - 1));
  memcpy(buffer, this->dmx_data_ + 1, copy_size);
  ESP_LOGVV(TAG, "DMX '%s': Universe read (%zu bytes copied)",
            this->name_.c_str(), copy_size);
}

void DMXComponent::write_universe(const uint8_t *data, size_t length) {
  if (this->mode_ != DMX_MODE_SEND) {
    ESP_LOGV(TAG, "DMX '%s': port not in send mode, ignoring write request",
             this->name_.c_str());
    return;
  }
  // Make sure zero byte in DMX packet is the DMX start code. Always 0x00.
  this->dmx_data_[0] = 0x00;

  length = std::min(length, static_cast<size_t>(DMX_PACKET_SIZE - 1));

  if (this->concurrency_resolution_ == CONCURRENCY_RESOLUTION_HTP) {
    // HTP: Merge with existing data, keeping the highest value for each channel
    for (size_t i = 0; i < length; i++) {
      uint8_t new_value = data[i];
      uint8_t current_value = this->dmx_data_[i + 1];
      this->dmx_data_[i + 1] =
          (new_value > current_value) ? new_value : current_value;
    }
    ESP_LOGVV(TAG, "DMX '%s': Universe merged (HTP) with %zu channels",
              this->name_.c_str(), length);
  } else {
    // LTP: Copy new data
    memcpy(this->dmx_data_ + 1, data, length);
    ESP_LOGVV(TAG, "DMX '%s': Universe written (LTP) with %zu channels",
              this->name_.c_str(), length);
  }
}

void DMXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DMX:");
  if (!this->name_.empty()) {
    ESP_LOGCONFIG(TAG, "  Name: %s", this->name_.c_str());
  }
  ESP_LOGCONFIG(TAG, "  Port: %d", this->dmx_port_id_);
  ESP_LOGCONFIG(TAG, "  Mode: %s",
                this->mode_ == DMX_MODE_SEND ? "SEND" : "RECEIVE");
  if (this->mode_ == DMX_MODE_SEND) {
    float write_frequency = 1000.0f / this->write_interval_ms_;
    ESP_LOGCONFIG(TAG, "  Write Frequency: %.1f Hz", write_frequency);
    ESP_LOGCONFIG(TAG, "  Concurrency Resolution: %s",
                  this->concurrency_resolution_ == CONCURRENCY_RESOLUTION_HTP
                      ? "HTP"
                      : "LTP");
  }
  if (this->mode_ == DMX_MODE_RECEIVE) {
    float read_frequency = 1000.0f / this->read_interval_ms_;
    ESP_LOGCONFIG(TAG, "  Read Frequency: %.1f Hz", read_frequency);
  }
  LOG_PIN("  TX Pin: ", this->tx_pin_);
  LOG_PIN("  RX Pin: ", this->rx_pin_);
  if (this->enable_pin_ != nullptr) {
    LOG_PIN("  Enable Pin: ", this->enable_pin_);
  }
}

void DMXComponent::write_channel(uint16_t channel, uint8_t value) {
  if (this->mode_ != DMX_MODE_SEND) {
    ESP_LOGVV(TAG, "DMX '%s': port not in send mode, ignoring write request",
              this->name_.c_str());
    return;
  }

  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return;
  }

  // Apply concurrency resolution strategy
  if (this->concurrency_resolution_ == CONCURRENCY_RESOLUTION_HTP) {
    // HTP: Keep the highest value
    uint8_t old_value = this->dmx_data_[channel];
    this->dmx_data_[channel] =
        (value > this->dmx_data_[channel]) ? value : this->dmx_data_[channel];
    if (value > old_value) {
      ESP_LOGVV(TAG, "DMX '%s': Channel %d set to %d (HTP, updated)",
                this->name_.c_str(), channel, value);
    } else {
      ESP_LOGVV(TAG,
                "DMX '%s': Channel %d not updated - %d is not higher than %d "
                "(HTP)",
                this->name_.c_str(), channel, value, old_value);
    }
  } else {
    // LTP: Always take the latest value
    this->dmx_data_[channel] = value;
    ESP_LOGVV(TAG, "DMX '%s': Channel %d set to %d (LTP)", this->name_.c_str(),
              channel, value);
  }
}

uint8_t DMXComponent::read_channel(uint16_t channel) {
  if (channel < 1 || channel > DMX_PACKET_SIZE) {
    ESP_LOGW(TAG, "Invalid DMX channel: %d (must be 1-512)", channel);
    return 0;
  }
  // Zero slot in DMX packet is the DMX start code. Don't offset by -1.
  uint8_t value = this->dmx_data_[channel];
  ESP_LOGVV(TAG, "DMX '%s': Channel %d read as %d", this->name_.c_str(),
            channel, value);
  return value;
}

} // namespace esphome::dmx
