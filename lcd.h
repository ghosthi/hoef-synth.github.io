#ifndef LCD
#define LCD

#include "TFT_eSPI.h"
#include <SPI.h>
#include <Arduino.h>

// SETUP
void lcd_setup();

// SAIDA
void lcd_output(long steps[], int bpm, int num_steps);

#endif
