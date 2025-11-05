#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../dmx.h"

namespace esphome::dmx {

class DMXSensor : public sensor::Sensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_parent(DMXComponent *parent) { parent_ = parent; }
  void set_channel(uint16_t channel) { channel_ = channel; }

 protected:
  DMXComponent *parent_{nullptr};
  uint16_t channel_{1};
  uint8_t last_value_{0};
};

}  // namespace esphome::dmx
