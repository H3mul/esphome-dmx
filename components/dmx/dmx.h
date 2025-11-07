#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include <esp_dmx.h>
#include <string>

namespace esphome::dmx {

enum DMXMode {
  DMX_MODE_SEND,
  DMX_MODE_RECEIVE,
};

class DMXComponent : public Component {
public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::BUS; }

  std::string get_name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }
  void set_tx_pin(InternalGPIOPin *pin) { tx_pin_ = pin; }
  void set_rx_pin(InternalGPIOPin *pin) { rx_pin_ = pin; }
  void set_enable_pin(InternalGPIOPin *pin) { enable_pin_ = pin; }
  void set_dmx_port_id(int port_id) { dmx_port_id_ = port_id; }
  void set_mode(DMXMode mode) { mode_ = mode; }
  void set_read_interval(uint32_t interval_ms) {
    read_interval_ms_ = interval_ms;
  }
  void set_send_timeout_ticks(uint16_t ticks) { send_timeout_ticks_ = ticks; }
  void set_receive_timeout_ticks(uint16_t ticks) {
    receive_timeout_ticks_ = ticks;
  }
  void set_enabled(bool enabled) { enabled_ = enabled; }

  DMXMode get_mode() const { return mode_; }

  /// Write a value to a DMX channel (1-512)
  void write_channel(uint16_t channel, uint8_t value);

  /// Write and send a value to a DMX channel (1-512)
  void send_channel(uint16_t channel, uint8_t value);

  // Write the DMX data to the bus and wait for sent
  void send_data();

  /// Read a value from a DMX channel (1-512)
  uint8_t read_channel(uint16_t channel);

  /// Read the DMX universe buffer (copies data to provided buffer)
  void read_universe(uint8_t *buffer, size_t buffer_size);

  /// Write a whole DMX universe (packet) to the internal buffer
  void write_universe(const uint8_t *data, size_t length);

  /// Write and send a whole DMX universe (packet)
  void send_universe(const uint8_t *data, size_t length);

  dmx_port_t get_port() const { return dmx_port_id_; }

protected:
  std::string name_{};
  InternalGPIOPin *tx_pin_{nullptr};
  InternalGPIOPin *rx_pin_{nullptr};
  InternalGPIOPin *enable_pin_{nullptr};
  dmx_port_t dmx_port_id_{DMX_NUM_0};
  uint8_t dmx_data_[DMX_PACKET_SIZE]{};
  DMXMode mode_{DMX_MODE_SEND};
  uint32_t read_interval_ms_{100};
  uint32_t last_read_time_{0};
  uint16_t send_timeout_ticks_{100};
  uint16_t receive_timeout_ticks_{100};
  bool enabled_{true};
};

} // namespace esphome::dmx
