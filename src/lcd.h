#ifndef LCD
#define LCD

#include <Wire.h>
#include <Adafruit_MCP4725.h>

// SETUP
void lcd_setup();

// SAIDA
void lcd_output(long steps, uint16_t BPM);

#endif
