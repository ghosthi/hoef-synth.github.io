#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <math.h>
// resolução no define pode ter valores 5, 6, 7, 8 ou
//      corresponde aos limites de valores 2^DAC_RESOLUTION
#define DAC_RESOLUTION (8)
#define MCP4725_MAX 4095
#define MCP4725_VREF 5.0f

Adafruit_MCP4725 dac;

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

    return (uint16_t)round((volts / MCP4725_VREF) * MCP4725_MAX);
}

void dac_output(float volts)
{
    uint16_t valor = volts_to_dac_output(volts);
    if (valor > pow(2, DAC_RESOLUTION))
    {
        Serial.print("Valor (");
        Serial.print(valor);
        Serial.print(") maior que o suportado para a resolucao de ");
        Serial.print(DAC_RESOLUTION);
        Serial.println("!");
        valor = 0;
    }
    dac.setVoltage((uint16_t)valor, true);
}