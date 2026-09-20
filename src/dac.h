#ifndef DAC
#define DAC

#include <Wire.h>
#include <Adafruit_MCP4725.h>

// SETUP
void dac_setup(void);

// SAIDA
void dac_output(float volts);

#endif