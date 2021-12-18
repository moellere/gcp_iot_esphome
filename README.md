# esphome-mitsubishiheatpump

Initial attempt at integrating GCP IoT Core using the [ESPHome](https://esphome.io) framework.

## Features
* Stuff.
* Things.
* Uses the [Google Cloud IoT JWT](https://github.com/GoogleCloudPlatform/google-cloud-iot-arduino) Arduino
  libary as a foundation for IoT Core integration

## Requirements
* https://github.com/GoogleCloudPlatform/google-cloud-iot-arduino
* ESPHome 1.19.1 or greater

## Supported Microcontrollers
This library should work on most ESP8266 or ESP32 platforms. It has been tested
with the following MCUs:
* WeMos D1 Mini (ESP8266)
* Generic ESP32 Dev Kit (ESP32)

## Usage
### Step 1: Find a microcontroller
### Step 2: Use ESPHome 1.19.1 or higher
### Step 3: Add this repository as an external component

Add this repository to your ESPHome config:

```yaml
external_components:
  - source: github://moellere/gcp_iot_esphome
```

# Example configuration

Below is an example configuration which will include wireless strength
indicators and permit over the air updates. You'll need to create a
`secrets.yaml` file inside of your `esphome` directory with entries for the
various items prefixed with `!secret`.

```yaml
substitutions:
  name: gcpiottest
  friendly_name: Test GCP IoT Core


esphome:
  name: ${name}
  platform: ESP8266
  board: d1_mini
  # Boards tested: Wemos D1 Mini (ESP8266)

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "${friendly_name} Fallback Hotspot"
    password: !secret fallback_password

captive_portal:

# Enable logging
logger:
  # ESP8266 only - disable serial port logging, as the HeatPump component
  # needs the sole hardware UART on the ESP8266
  baud_rate: 0

# Enable Home Assistant API
api:

ota:

# Enable Web server.
web_server:
  port: 80

  # Sync time with Home Assistant.
time:
  - platform: homeassistant
    id: homeassistant_time

# Text sensors with general information.
text_sensor:
  # Expose ESPHome version as sensor.
  - platform: version
    name: ${name} ESPHome Version
  # Expose WiFi information as sensors.
  - platform: wifi_info
    ip_address:
      name: ${name} IP
    ssid:
      name: ${name} SSID
    bssid:
      name: ${name} BSSID

# Sensors with general information.
sensor:
  # Uptime sensor.
  - platform: uptime
    name: ${name} Uptime

  # WiFi Signal sensor.
  - platform: wifi_signal
    name: ${name} WiFi Signal
    update_interval: 60s

external_components:
  - source: github://moellere/gcp_iot_esphome

# See Also

## Reference documentation

The author referred to the following documentation repeatedly:
* [ESPHome Custom Sensors Reference](https://esphome.io/components/sensor/custom.html)
* [ESPHome Custom Climate Components Reference](https://esphome.io/components/climate/custom.html)
* [ESPHome External Components Reference](https://esphome.io/components/external_components.html)
* [Source for ESPHome's Climate Component](https://github.com/esphome/esphome/tree/master/esphome/components/climate)
