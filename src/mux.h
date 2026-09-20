#ifndef MUX
#define MUX

#include <Arduino.h>
#include <Wire.h>
#include "PCF8575.h"

// SETUP
void mux_setup();

// INPUT
void update_encoder_values();

// SAIDA
float step_to_volt(long valor);
void get_steps(long values[]);
int get_num_steps();
int get_bpm(); // para o LCD
bool encoder_enabled(int i);

#endif