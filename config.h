#pragma once

// ENABLES
#define DAC_ENABLED 0
#define MUX_BPM_CLICK_ENABLED 1
#define MUX_STEPS_ENABLED 0
#define LCD_ENABLED 0
#define CLK_GATE_ENABLED 0

// MAIN
#define DEBUG 1
#define NUM_STEPS 8

// DAC
#define DAC_RESOLUTION (12)
#define MCP4725_VREF 5.0f

// MUX_BPM_CLICK
#define MIN_BPM 60
#define MAX_BPM 300
#define BPM_CLICK_PORT_NUMBER 11

// MUX STEPS
#define STEP_MIN_VALUE 0
#define STEP_MAX_VALUE 60

// LCD

// I2C
#define I2C_CLOCK_FREQUENCY 400000 // frequencia em Hz do clock I2C
#define MUX_BPM_CLICK_I2C_ADDR 0x20
#define MUX_STEP_I2C_ADDR 0x20
#define DAC_I2C_ADDR 0x62 // MCP4725A1: 0x62 (default) ou 0x63 (ADDR pin vinculado to VCC)
                          // MCP4725A0: 0x60 ou 0x61
                          // MCP4725A2: 0x64 ou 0x65

// PINS

#define PIN_CLOCK 26               // Clock
#define PIN_GATE 27                // Gate
#define MUX_STEPS_INTERRUPT_PIN 34 // STEP INT
#define MUX_BPM_INTERRUPT_PIN 34   // BPM INT
#define SDA_PIN 21                 // Numero da GPIO usado como SDA
#define SCL_PIN 22                 // Numero da GPIO usado como SCL