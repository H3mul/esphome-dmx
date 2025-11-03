#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/hal.h"
#include <esp_dmx.h>

namespace esphome::dmx {

class DMXComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::BUS; }

  void set_tx_pin(InternalGPIOPin *pin) { tx_pin_ = pin; }
  void set_rx_pin(InternalGPIOPin *pin) { rx_pin_ = pin; }
  void set_enable_pin(InternalGPIOPin *pin) { enable_pin_ = pin; }
  void set_dmx_port_id(int port_id) { dmx_port_id_ = port_id; }

  /// Write a value to a DMX channel (1-512)
  void write_channel(uint16_t channel, uint8_t value);

  /// Read a value from a DMX channel (1-512)
  uint8_t read_channel(uint16_t channel);

  // Write the DMX data to the bus and wait for sent
  void write_data();

  dmx_port_t get_port() const { return dmx_port_id_; }

 protected:
  InternalGPIOPin *tx_pin_{nullptr};
  InternalGPIOPin *rx_pin_{nullptr};
  InternalGPIOPin *enable_pin_{nullptr};
  dmx_port_t dmx_port_id_{DMX_NUM_0};
  uint8_t dmx_data_[DMX_PACKET_SIZE]{};
};

}  // namespace esphome::dmx
