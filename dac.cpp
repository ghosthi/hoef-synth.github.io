#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <math.h>
#include "config.h"

Adafruit_MCP4725 dac;
long MAX_VALUE = pow(2, DAC_RESOLUTION);
// criado no setup da main
extern SemaphoreHandle_t i2cMutex;

void dac_setup(void)
{
    if (DAC_ENABLED == 0)
    {
        return;
    }
    dac.begin(DAC_I2C_ADDR);
}

uint16_t volts_to_dac_output(float volts)
{
    // Limita a tensão à faixa do DAC
    volts = constrain(volts, 0.0f, MCP4725_VREF);

    return (uint16_t)round((volts / MCP4725_VREF) * MAX_VALUE);
}

void dac_output(float volts)
{
    if (DAC_ENABLED == 0)
    {
        return;
    }
    uint16_t valor = volts_to_dac_output(volts);
    if (valor > MAX_VALUE)
    {
        Serial.printf(
            "Valor limite estourado: valor=%u | max=%ld | volts=%f bits\n",
            valor,
            MAX_VALUE,
            volts);
        valor = 0;
    }
    xSemaphoreTake(i2cMutex, portMAX_DELAY);
    dac.setVoltage(valor, false);
    xSemaphoreGive(i2cMutex);
}