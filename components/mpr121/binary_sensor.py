import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_CHANNEL,
    CONF_GPIO,
    CONF_MODE,
    CONF_PULLDOWN,
    CONF_PULLUP
)    
    
from . import (
    mpr121_ns,
    MPR121Component,
    CONF_MPR121_ID,
    CONF_TOUCH_THRESHOLD,
    CONF_RELEASE_THRESHOLD
)

DEPENDENCIES = ["mpr121"]
MPR121Channel = mpr121_ns.class_("MPR121Channel", binary_sensor.BinarySensor)

def validate_supports(value):
    num = value[CONF_CHANNEL]
    is_touch = value[CONF_TOUCH_THRESHOLD] if CONF_TOUCH_THRESHOLD in value else False
    is_release = value[CONF_RELEASE_THRESHOLD] if CONF_RELEASE_THRESHOLD in value else False
    is_gpio = value[CONF_GPIO] if CONF_GPIO in value else False
    mode = value[CONF_NODE] if CONF_MODE in value else {}
    is_pullup = mode[CONF_PULLUP] if CONF_PULLUP in mode else False
    is_pulldown = mode[CONF_PULLDOWN] if CONF_PULLDOWN in mode else False


    if is_gpio:
        if not (num >= 4 and num <= 11):
            raise cv.Invalid(
                "GPIO Channel must be in range [4..11]",
                [CONF_CHANNEL],
            )
        
        if is_pullup and is_pulldown:
            raise cv.Invalid(
                "Pull-up and pull-down modes can not be used at same time.",
                [CONF_PULLUP, CONF_PULLDOWN]
            )   
        
        if is_touch or is_release:
            raise cv.Invalid(
                "Treshold settings not available in GPIO mode.",
                [CONF_TOUCH_THRESHOLD, CONF_RELEASE_THRESHOLD]
            ) 
              
    else:
        if is_pullup or is_pulldown:
            raise cv.Invalid(
                "Pull-up or pull-down modes works only whit GPIO channels.",
                [CONF_GPIO, CONF_PULLUP, CONF_PULLDOWN]
            )        


    return value

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(MPR121Channel).extend({
        cv.GenerateID(CONF_MPR121_ID): cv.use_id(MPR121Component),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=11),
        cv.Optional(CONF_TOUCH_THRESHOLD): cv.int_range(min=0x05, max=0x30),
        cv.Optional(CONF_RELEASE_THRESHOLD): cv.int_range(min=0x05, max=0x30),
        cv.Optional(CONF_GPIO, default=False): cv.boolean,
        cv.Optional(CONF_MODE): cv.Schema({
            cv.Optional(CONF_PULLUP, default=False): cv.boolean,
            cv.Optional(CONF_PULLDOWN, default=False): cv.boolean
        })
    }),
    validate_supports    
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    hub = await cg.get_variable(config[CONF_MPR121_ID])

    
    if CONF_GPIO in config and config[CONF_GPIO]:
        modes = config[CONF_MODE] if CONF_MODE in config else {}
        pullup = (CONF_PULLUP in modes and modes[CONF_PULLUP])
        pulldown = (CONF_PULLDOWN in modes and modes[CONF_PULLDOWN])
            
        cg.add(var.set_input(config[CONF_CHANNEL], pullup, pulldown)) 
        cg.add(hub.register_input(var))        
    
    else:
        cg.add(var.set_channel(config[CONF_CHANNEL]))        
        
        if CONF_TOUCH_THRESHOLD in config:
            cg.add(var.set_touch_threshold(config[CONF_TOUCH_THRESHOLD]))
        if CONF_RELEASE_THRESHOLD in config:
            cg.add(var.set_release_threshold(config[CONF_RELEASE_THRESHOLD]))
            cg.add(hub.register_channel(var))
    