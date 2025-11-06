import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID, CONF_CHANNEL
from . import dmx_ns, DMXComponent, DMXMode, CONF_MODE
import sys

DEPENDENCIES = ["dmx"]

CONF_DMX_BUS_ID = "dmx_bus_id"

DMXOutput = dmx_ns.class_("DMXOutput", output.FloatOutput, cg.Component)

CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.Required(CONF_ID): cv.declare_id(DMXOutput),
            cv.Required(CONF_DMX_BUS_ID): cv.use_id(DMXComponent),
            cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=512),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_DMX_BUS_ID])
    cg.add(var.set_dmx_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
