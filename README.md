# ESPHome DMX Component

An ESPHome component that wraps the [someweisguy/esp_dmx](https://github.com/someweisguy/esp_dmx) library to provide DMX512 support for ESP32.

## Features

- Multiple DMX buses/ports support (up to 2 ports on ESP32)
- DMX output platform for controlling individual DMX channels
- Seamless integration with ESPHome's output and light components
- Configurable TX, RX, and Enable pins

## Installation

Add this component to your ESPHome configuration by placing the `dmx` folder in your `components` directory.

## Configuration

### DMX Bus Configuration

```yaml
dmx:
  - id: dmx_bus_a
    enable_pin: 
      number: GPIO4
    tx_pin:
      number: GPIO16
    rx_pin:
      number: GPIO17
    dmx_port_id: 1
   
  - id: dmx_bus_b
    enable_pin: 
      number: GPIO5
    tx_pin:
      number: GPIO21
    rx_pin:
      number: GPIO22
    dmx_port_id: 2
```

#### Configuration variables:
- **id** (**Required**, ID): The ID for this DMX bus.
- **tx_pin** (**Required**, Pin): The GPIO pin for DMX transmission.
- **rx_pin** (**Required**, Pin): The GPIO pin for DMX reception.
- **enable_pin** (*Optional*, Pin): The GPIO pin for the RS485 driver enable signal.
- **dmx_port_id** (**Required**, int): The DMX port number (0-2). Each bus must use a unique port ID.

### DMX Output Platform

```yaml
output:
  - platform: dmx
    channel: 1
    dmx_bus_id: dmx_bus_a
    id: dmx_output_1
```

#### Configuration variables:
- **id** (**Required**, ID): The ID for this output.
- **dmx_bus_id** (**Required**, ID): The ID of the DMX bus to use.
- **channel** (**Required**, int): The DMX channel number (1-512).

## Usage Examples

### Single Channel Light
```yaml
output:
  - platform: dmx
    channel: 1
    dmx_bus_id: dmx_bus_a
    id: dmx_ch1

light:
  - platform: monochromatic
    name: "DMX Dimmer"
    output: dmx_ch1
```

### RGB Light
```yaml
output:
  - platform: dmx
    channel: 1
    dmx_bus_id: dmx_bus_a
    id: dmx_red
  - platform: dmx
    channel: 2
    dmx_bus_id: dmx_bus_a
    id: dmx_green
  - platform: dmx
    channel: 3
    dmx_bus_id: dmx_bus_a
    id: dmx_blue

light:
  - platform: rgb
    name: "DMX RGB Light"
    red: dmx_red
    green: dmx_green
    blue: dmx_blue
```

### RGBW Light
```yaml
output:
  - platform: dmx
    channel: 10
    dmx_bus_id: dmx_bus_a
    id: dmx_red
  - platform: dmx
    channel: 11
    dmx_bus_id: dmx_bus_a
    id: dmx_green
  - platform: dmx
    channel: 12
    dmx_bus_id: dmx_bus_a
    id: dmx_blue
  - platform: dmx
    channel: 13
    dmx_bus_id: dmx_bus_a
    id: dmx_white

light:
  - platform: rgbw
    name: "DMX RGBW Light"
    red: dmx_red
    green: dmx_green
    blue: dmx_blue
    white: dmx_white
```

## Hardware Requirements

- ESP32 (ESP8266 is not supported)
- RS485 transceiver (e.g., MAX485, SN75176)
- XLR connectors for DMX

## Wiring

Connect an RS485 transceiver to your ESP32:
- TX pin → RS485 DI (Driver Input)
- RX pin → RS485 RO (Receiver Output)
- Enable pin → RS485 DE and RE (Driver Enable and Receiver Enable, tied together)
- RS485 A and B → DMX+ and DMX- on XLR connector

## License

This component wraps the [esp_dmx library](https://github.com/someweisguy/esp_dmx) by someweisguy.
