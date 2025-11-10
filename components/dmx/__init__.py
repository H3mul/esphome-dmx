import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID, CONF_MODE
from esphome.core import CORE

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["output", "sensor"]

CONF_ENABLED = "enabled"
CONF_ENABLE_PIN = "enable_pin"
CONF_TX_PIN = "tx_pin"
CONF_RX_PIN = "rx_pin"
CONF_DMX_PORT_ID = "dmx_port_id"
CONF_READ_FREQUENCY = "read_frequency"
CONF_WRITE_FREQUENCY = "write_frequency"
CONF_RECEIVE_TIMEOUT_TICKS = "receive_timeout_ticks"
CONF_NAME = "name"
CONF_CONCURRENCY_RESOLUTION = "concurrency_resolution"

dmx_ns = cg.esphome_ns.namespace("dmx")
DMXComponent = dmx_ns.class_("DMXComponent", cg.Component)
DMXMode = dmx_ns.enum("DMXMode")
ConcurrencyResolution = dmx_ns.enum("ConcurrencyResolution")

DMX_MODES = {
    "send": DMXMode.DMX_MODE_SEND,
    "receive": DMXMode.DMX_MODE_RECEIVE,
}

CONCURRENCY_RESOLUTIONS = {
    "HTP": ConcurrencyResolution.CONCURRENCY_RESOLUTION_HTP,
    "LTP": ConcurrencyResolution.CONCURRENCY_RESOLUTION_LTP,
}

DMX_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(DMXComponent),
        cv.Optional(CONF_NAME): cv.string,
        cv.Optional(CONF_ENABLED, default=True): cv.boolean,
        cv.Required(CONF_TX_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_RX_PIN): pins.gpio_input_pin_schema,
        cv.Required(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_DMX_PORT_ID): cv.int_range(min=0, max=2),
        cv.Optional(CONF_MODE, default="send"): cv.enum(DMX_MODES, lower=True),
        cv.Optional(CONF_READ_FREQUENCY, default=40.0): cv.float_range(min=0.1, max=44.1),
        cv.Optional(CONF_WRITE_FREQUENCY, default=40.0): cv.float_range(min=0.1, max=44.1),
        cv.Optional(CONF_RECEIVE_TIMEOUT_TICKS, default=100): cv.int_range(min=0, max=1250),
        cv.Optional(CONF_CONCURRENCY_RESOLUTION, default="LTP"): cv.enum(CONCURRENCY_RESOLUTIONS, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.All(cv.ensure_list(DMX_COMPONENT_SCHEMA))


async def to_code(config):
    # Process each DMX bus in the list
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)

        if CONF_NAME in conf:
            cg.add(var.set_name(conf[CONF_NAME]))

        tx_pin = await cg.gpio_pin_expression(conf[CONF_TX_PIN])
        cg.add(var.set_tx_pin(tx_pin))

        rx_pin = await cg.gpio_pin_expression(conf[CONF_RX_PIN])
        cg.add(var.set_rx_pin(rx_pin))

        enable_pin = await cg.gpio_pin_expression(conf[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable_pin))

        cg.add(var.set_dmx_port_id(conf[CONF_DMX_PORT_ID]))
        cg.add(var.set_mode(conf[CONF_MODE]))
        cg.add(var.set_read_frequency(conf[CONF_READ_FREQUENCY]))
        cg.add(var.set_write_frequency(conf[CONF_WRITE_FREQUENCY]))
        cg.add(var.set_receive_timeout_ticks(conf[CONF_RECEIVE_TIMEOUT_TICKS]))
        cg.add(var.set_enabled(conf[CONF_ENABLED]))
        cg.add(var.set_concurrency_resolution(conf[CONF_CONCURRENCY_RESOLUTION]))

    # Add esp_dmx library once
    cg.add_library(
        name="esp_dmx", 
        repository="https://github.com/H3mul/esp_dmx.git#d415508459953083e464a4c3c693dba92425be83",
        version=None)
