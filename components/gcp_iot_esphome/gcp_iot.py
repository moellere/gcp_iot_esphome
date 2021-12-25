import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.logger import HARDWARE_UART_TO_SERIAL
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
    CONF_MODE,
    CONF_SWING_MODE,
)
from esphome.core import CORE, coroutine

AUTO_LOAD = ["climate"]

CONF_SUPPORTS = "supports"

GCPIoTEsp = cg.global_ns.class_("GCPIoTEsp", climate.Climate, cg.PollingComponent)

CONF_PROJECT_ID =  'project_id'
CONF_LOCATION =  'location'
CONF_REGISTRY_ID =  'registry_id'
CONF_DEVICE_ID =  'device_id'
CONF_PRIVATE_KEY =  'private_key'
CONF_POLL_INTERVAL =  'poll_interval'
CONF_PRIMARY_CA =  'primary_ca'
CONF_BACKUP_CA =  'backup_ca'

CONFIG_SCHEMA = cv.Schema({
  cv.Required(CONF_PROJECT_ID): cv.string,
  cv.Required(CONF_LOCATION): cv.string,
  cv.Required(CONF_REGISTRY_ID): cv.string,
  cv.Required(CONF_DEVICE_ID): cv.string,
  cv.Required(CONF_PRIVATE_KEY): cv.string,
  cv.Required(CONF_PRIMARY_CA): cv.string,
  cv.Required(CONF_BACKUP_CA): cv.string,
  cv.Optional(CONF_POLL_INTERVAL, default=500): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)


@coroutine
def to_code(config):
    
    var = cg.new_Pvariable(config[CONF_ID], cg.RawExpression(f'&{serial}'))

    supports = config[CONF_SUPPORTS]
    traits = var.config_traits()

    for mode in supports[CONF_MODE]:
        if mode == 'OFF':
            continue
        cg.add(traits.add_supported_mode(climate.CLIMATE_MODES[mode]))

    for mode in supports[CONF_FAN_MODE]:
        cg.add(traits.add_supported_fan_mode(climate.CLIMATE_FAN_MODES[mode]))

    for mode in supports[CONF_SWING_MODE]:
        cg.add(traits.add_supported_swing_mode(climate.CLIMATE_SWING_MODES[mode]))

    yield cg.register_component(var, config)
    yield climate.register_climate(var, config)
    cg.add_library("https://github.com/SwiCago/HeatPump", None)

