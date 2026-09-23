#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MCP4725_ADDRESS 0x62
#define VREF 3.3f

typedef struct
{
  pin_t vout;
  i2c_dev_t i2c;

  uint16_t dac_value;

  uint8_t buffer[2];
  uint8_t buffer_pos;
} chip_state_t;

static chip_state_t chip;

// ============================================================
// Atualiza VOUT
// ============================================================

static void update_output(void)
{
  float voltage =
      ((float)chip.dac_value / 4095.0f) * VREF;

  pin_dac_write(chip.vout, voltage);

  printf(
      "MCP4725: DAC=%u -> VOUT=%.3f V\n",
      chip.dac_value,
      voltage);
}

// ============================================================
// I2C CONNECT
// ============================================================

static bool on_i2c_connect(
    void *user_data,
    uint32_t address,
    bool read)
{
  (void)user_data;
  (void)read;

  chip.buffer_pos = 0;

  if (address != MCP4725_ADDRESS)
  {
    return false;
  }

  return true;
}

// ============================================================
// I2C WRITE
// ============================================================

static bool on_i2c_write(
    void *user_data,
    uint8_t data)
{
  (void)user_data;

  if (chip.buffer_pos < 2)
  {
    chip.buffer[chip.buffer_pos++] = data;
  }

  /*
   * Fast Write / Write DAC Register:
   *
   * Byte 0:
   * 0100 D11 D10 D9 D8
   *
   * Byte 1:
   * D7 ... D0
   */

  if (chip.buffer_pos == 2)
  {

    uint8_t command = chip.buffer[0];
    uint8_t low = chip.buffer[1];

    /*
     * 0x40 = Write DAC register
     */
    if ((command & 0xF0) == 0x40)
    {

      uint16_t high =
          command & 0x0F;

      chip.dac_value =
          (high << 8) | low;

      chip.dac_value &= 0x0FFF;

      update_output();
    }

    chip.buffer_pos = 0;
  }

  return true;
}

// ============================================================
// I2C READ
// ============================================================

static uint8_t on_i2c_read(
    void *user_data)
{
  (void)user_data;

  static bool high_byte = true;

  uint8_t result;

  if (high_byte)
  {
    result =
        (chip.dac_value >> 8) & 0x0F;
  }
  else
  {
    result =
        chip.dac_value & 0xFF;
  }

  high_byte = !high_byte;

  return result;
}

// ============================================================
// I2C DISCONNECT
// ============================================================

static void on_i2c_disconnect(
    void *user_data)
{
  (void)user_data;

  chip.buffer_pos = 0;
}

// ============================================================
// CHIP INIT
// ============================================================

void chip_init(void)
{
  /*
   * VOUT
   */
  chip.vout =
      pin_init("VOUT", ANALOG);

  /*
   * Valor inicial do DAC
   */
  chip.dac_value = 0;
  chip.buffer_pos = 0;

  pin_dac_write(
      chip.vout,
      0.0f);

  /*
   * I2C
   */
  const i2c_config_t config = {
      .address = MCP4725_ADDRESS,

      .sda = pin_init(
          "SDA",
          INPUT_PULLUP),

      .scl = pin_init(
          "SCL",
          INPUT_PULLUP),

      .connect = on_i2c_connect,
      .read = on_i2c_read,
      .write = on_i2c_write,
      .disconnect = on_i2c_disconnect,

      .user_data = NULL};

  chip.i2c =
      i2c_init(&config);

  printf(
      "MCP4725 initialized at 0x%02X\n",
      MCP4725_ADDRESS);
}