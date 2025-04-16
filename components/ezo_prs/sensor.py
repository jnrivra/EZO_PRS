import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_PRESSURE,
    STATE_CLASS_MEASUREMENT,
    CONF_UNIT_OF_MEASUREMENT,
)
from . import ezo_prs_ns

DEPENDENCIES = ["i2c"]

EZOPRSSensor = ezo_prs_ns.class_("EZOPRSSensor", sensor.Sensor, cg.PollingComponent, i2c.I2CDevice)

CONF_PRESSURE_UNIT = "pressure_unit"
CONF_DECIMALS = "decimals"
CONF_INFO_TEXT_SENSOR = "info_text_sensor"
CONF_STATUS_TEXT_SENSOR = "status_text_sensor"

PRESSURE_UNIT_PSI = "psi"
PRESSURE_UNIT_ATM = "atm"
PRESSURE_UNIT_BAR = "bar"
PRESSURE_UNIT_KPA = "kpa"
PRESSURE_UNIT_INH2O = "inh2o"
PRESSURE_UNIT_CMH2O = "cmh2o"

PRESSURE_UNITS = {
    PRESSURE_UNIT_PSI: "psi",
    PRESSURE_UNIT_ATM: "atm",
    PRESSURE_UNIT_BAR: "bar",
    PRESSURE_UNIT_KPA: "kPa",
    PRESSURE_UNIT_INH2O: "inH₂O",
    PRESSURE_UNIT_CMH2O: "cmH₂O",
}

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        EZOPRSSensor,
        unit_of_measurement="cmH₂O",  # Cambiado de psi a cmH₂O
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_PRESSURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.Optional(CONF_PRESSURE_UNIT, default=PRESSURE_UNIT_CMH2O): cv.enum(PRESSURE_UNITS),
            cv.Optional(CONF_DECIMALS, default=3): cv.int_range(0, 3),
            cv.Optional(CONF_INFO_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_STATUS_TEXT_SENSOR): cv.use_id(text_sensor.TextSensor),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x6A))
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    unit = config[CONF_PRESSURE_UNIT]
    cg.add(var.set_pressure_unit(unit))
    
    decimals = config[CONF_DECIMALS]
    cg.add(var.set_decimal_places(decimals))
    
    if CONF_INFO_TEXT_SENSOR in config:
        info_sensor = await cg.get_variable(config[CONF_INFO_TEXT_SENSOR])
        cg.add(var.set_info_text_sensor(info_sensor))
        
    if CONF_STATUS_TEXT_SENSOR in config:
        status_sensor = await cg.get_variable(config[CONF_STATUS_TEXT_SENSOR])
        cg.add(var.set_status_text_sensor(status_sensor))