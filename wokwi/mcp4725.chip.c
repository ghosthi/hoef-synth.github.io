#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  pin_t vcc;
  pin_t gnd;
  pin_t sda;
  pin_t scl;
  pin_t vout;

  i2c_dev_t i2c;

  uint16_t dac_value;
} chip_state_t;

// Called when Arduino starts an I2C transaction
bool on_i2c_connect(void *user_data, uint32_t address, bool read)
{
  chip_state_t *chip = (chip_state_t *)user_data;

  printf("MCP4725: I2C connected, address=0x%02X, read=%d\n",
         address, read);

  return true;
}

// Arduino writes one byte to the MCP4725
bool on_i2c_write(void *user_data, uint8_t data)
{
  chip_state_t *chip = (chip_state_t *)user_data;

  static uint8_t byte_index = 0;
  static uint8_t high_byte = 0;

  if (byte_index == 0)
  {
    /*
       MCP4725 Fast Mode:
       Byte 0:
       D7..D4 = command/control bits
       D3..D0 = upper DAC bits
    */

    high_byte = data & 0x0F;
    byte_index = 1;
  }
  else
  {
    /*
       Byte 1:
       D7..D0 = lower DAC bits
    */

    chip->dac_value =
        ((uint16_t)high_byte << 8) |
        data;

    // MCP4725 is 12-bit
    chip->dac_value &= 0x0FFF;

    // Convert 12-bit DAC value to 0–5 V
    float voltage =
        (5.0f * chip->dac_value) / 4095.0f;

    // Output the virtual analog voltage
    pin_dac_write(chip->vout, voltage);

    printf("MCP4725: DAC=%u -> VOUT=%.3f V\n",
           chip->dac_value,
           voltage);

    byte_index = 0;
  }

  return true;
}

void on_i2c_disconnect(void *user_data)
{
  // End of I2C transaction
}

void chip_init(void)
{

  chip_state_t *chip =
      (chip_state_t *)malloc(sizeof(chip_state_t));

  chip->dac_value = 0;

  // Initialize pins
  chip->vcc = pin_init("VCC", INPUT);
  chip->gnd = pin_init("GND", INPUT);
  chip->sda = pin_init("SDA", INPUT);
  chip->scl = pin_init("SCL", INPUT);

  // VOUT is analog
  chip->vout = pin_init("VOUT", ANALOG);

  // Initial DAC output = 0 V
  pin_dac_write(chip->vout, 0.0f);

  // Configure I2C device
  i2c_config_t config = {
      .address = 0x62,
      .scl = chip->scl,
      .sda = chip->sda,
      .connect = on_i2c_connect,
      .read = NULL,
      .write = on_i2c_write,
      .disconnect = on_i2c_disconnect,
      .user_data = chip,
  };

  chip->i2c = i2c_init(&config);

  printf("MCP4725 virtual DAC initialized at I2C address 0x62\n");
}