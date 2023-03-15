import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_CHANNEL,
    CONF_GPIO,
    CONF_MODE
)    
    
from . import (
    mpr121_ns,
    MPR121Component,
    CONF_MPR121_ID,
)

CONF_HIGHSIDE = "high_side"
CONF_LOWSIDE = "low_side"

DEPENDENCIES = ["mpr121"]
MPR121Switch = mpr121_ns.class_("MPR121Switch", switch.Switch)

def validate_supports(value):
    num = value[CONF_CHANNEL]
    mode = value[CONF_MODE] if CONF_MODE in value else {}
    is_highside = mode[CONF_HIGHSIDE] if CONF_HIGHSIDE in mode else False
    is_lowside = mode[CONF_LOWSIDE] if CONF_LOWSIDE in mode else False    


    if not (num >= 4 and num <= 11):
        raise cv.Invalid(
            "GPIO Channel must be in range of [3..11]",
            [CONF_CHANNEL],
        )
    
    if is_highside and is_lowside:
        raise cv.Invalid(
            "High-side and Low-side modes can not be used at same time.",
            [CONF_PULLUP, CONF_PULLDOWN]
        )   

    return value

CONFIG_SCHEMA = cv.All(
    switch.switch_schema(MPR121Switch).extend({
        cv.GenerateID(CONF_MPR121_ID): cv.use_id(MPR121Component),
        cv.Required(CONF_CHANNEL): cv.int_range(min=4, max=11),
        cv.Optional(CONF_MODE): cv.Schema({
            cv.Optional(CONF_HIGHSIDE, default=False): cv.boolean,
            cv.Optional(CONF_LOWSIDE, default=False): cv.boolean,
        })
    }),
    validate_supports
)


async def to_code(config):
    var = await switch.new_switch(config)
    hub = await cg.get_variable(config[CONF_MPR121_ID])

    modes = config[CONF_MODE] if CONF_MODE in config else {}
    highside = (CONF_HIGHSIDE in modes and modes[CONF_HIGHSIDE])
    lowside = (CONF_LOWSIDE in modes and modes[CONF_LOWSIDE])
        
    cg.add(var.set_output(config[CONF_CHANNEL], highside, lowside))
    cg.add(var.set_parent(hub))
