#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "dmx.h"

namespace esphome::dmx {

class DMXOutput : public output::FloatOutput, public Component {
 public:
  void set_dmx_parent(DMXComponent *parent) { parent_ = parent; }
  void set_channel(uint16_t channel) { channel_ = channel; }

  void setup() override;
  void dump_config() override;

 protected:
  void write_state(float state) override;

  DMXComponent *parent_{nullptr};
  uint16_t channel_{1};
};

}  // namespace esphome::dmx
