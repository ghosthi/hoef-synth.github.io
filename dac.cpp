#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <math.h>

#define DAC_RESOLUTION (12)
#define MCP4725_VREF 5.0f

Adafruit_MCP4725 dac;
long MAX_VALUE = pow(2, DAC_RESOLUTION);

void dac_setup(void)
{
    // MCP4725A1: 0x62 (default) ou 0x63 (ADDR pin vinculado to VCC)
    // MCP4725A0: 0x60 ou 0x61
    // MCP4725A2: 0x64 ou 0x65
    dac.begin(0x62);
}

uint16_t volts_to_dac_output(float volts)
{
    // Limita a tensão à faixa do DAC
    volts = constrain(volts, 0.0f, MCP4725_VREF);

    return (uint16_t)round((volts / MCP4725_VREF) * MAX_VALUE);
}

void dac_output(float volts)
{
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
    dac.setVoltage((uint16_t)valor, true);
}