import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_CHANNEL,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
)
from .. import dmx_ns, DMXComponent

CONF_DMX_BUS_ID = "dmx_bus_id"

DMXSensor = dmx_ns.class_("DMXSensor", sensor.Sensor, cg.Component)

CONFIG_SCHEMA = sensor.sensor_schema(
    DMXSensor,
    unit_of_measurement=UNIT_EMPTY,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
).extend(
    {
        cv.GenerateID(CONF_DMX_BUS_ID): cv.use_id(DMXComponent),
        cv.Required(CONF_CHANNEL): cv.int_range(min=1, max=512),
    }
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_DMX_BUS_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
