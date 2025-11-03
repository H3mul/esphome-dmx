import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID
from esphome.core import CORE

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["output"]

CONF_ENABLE_PIN = "enable_pin"
CONF_TX_PIN = "tx_pin"
CONF_RX_PIN = "rx_pin"
CONF_DMX_PORT_ID = "dmx_port_id"

dmx_ns = cg.esphome_ns.namespace("dmx")
DMXComponent = dmx_ns.class_("DMXComponent", cg.Component)

DMX_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DMXComponent),
        cv.Required(CONF_TX_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_RX_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_DMX_PORT_ID): cv.int_range(min=0, max=2),
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(cv.ensure_list(DMX_COMPONENT_SCHEMA))


async def to_code(config):
    # Process each DMX bus in the list
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)

        tx_pin = await cg.gpio_pin_expression(conf[CONF_TX_PIN])
        cg.add(var.set_tx_pin(tx_pin))

        rx_pin = await cg.gpio_pin_expression(conf[CONF_RX_PIN])
        cg.add(var.set_rx_pin(rx_pin))

        if CONF_ENABLE_PIN in conf:
            enable_pin = await cg.gpio_pin_expression(conf[CONF_ENABLE_PIN])
            cg.add(var.set_enable_pin(enable_pin))

        cg.add(var.set_dmx_port_id(conf[CONF_DMX_PORT_ID]))

    # Add esp_dmx library once
    cg.add_library("someweisguy/esp_dmx", "4.1.0")