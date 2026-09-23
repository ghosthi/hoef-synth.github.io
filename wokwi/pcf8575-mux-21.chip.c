#include "wokwi-api.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define PCF8575_ADDRESS 0x21

typedef struct
{
  i2c_dev_t i2c;

  pin_t pins[16];
  pin_t int_pin;

  /*
   * PCF8575 port latch.
   *
   * 1 = input/released
   * 0 = output LOW
   */
  uint16_t latch;

  /*
   * Number of bytes received during
   * current I2C write transaction.
   */
  uint8_t write_pos;

} chip_state_t;

static chip_state_t chip;

/*
 * Apply the latch configuration to
 * the physical simulation pins.
 */
static void update_pins(void)
{
  for (int i = 0; i < 16; i++)
  {

    if (chip.latch & (1 << i))
    {

      /*
       * HIGH on PCF8575 means the pin
       * is released and can be used as input.
       */
      pin_mode(
          chip.pins[i],
          INPUT_PULLUP);
    }
    else
    {

      /*
       * LOW means the PCF8575 actively
       * drives the pin LOW.
       */
      pin_mode(
          chip.pins[i],
          OUTPUT_LOW);
    }
  }
}

/*
 * Read all 16 physical pins.
 */
static uint16_t read_pins(void)
{
  uint16_t value = 0;

  for (int i = 0; i < 16; i++)
  {

    if (pin_read(chip.pins[i]) == HIGH)
    {
      value |= (1 << i);
    }
  }

  return value;
}

/*
 * I2C address received.
 */
static bool on_i2c_connect(
    void *user_data,
    uint32_t address,
    bool read)
{
  (void)user_data;
  (void)read;

  chip.write_pos = 0;

  if (address != PCF8575_ADDRESS)
  {
    return false;
  }

  return true;
}

/*
 * ESP32 reads from PCF8575.
 *
 * PCF8575 sends:
 *
 * byte 0 = P0..P7
 * byte 1 = P8..P15
 */
static uint8_t on_i2c_read(
    void *user_data)
{
  (void)user_data;

  static bool low_byte = true;

  uint16_t value =
      read_pins();

  uint8_t result;

  if (low_byte)
  {

    result =
        value & 0xFF;
  }
  else
  {

    result =
        (value >> 8) & 0xFF;
  }

  low_byte = !low_byte;

  printf(
      "PCF8575 READ: 0x%04X\n",
      value);

  return result;
}

/*
 * ESP32 writes to PCF8575.
 *
 * First byte:
 * P0..P7
 *
 * Second byte:
 * P8..P15
 */
static bool on_i2c_write(
    void *user_data,
    uint8_t data)
{
  (void)user_data;

  if (chip.write_pos == 0)
  {

    chip.latch =
        (chip.latch & 0xFF00) |
        data;

    chip.write_pos = 1;
  }
  else
  {

    chip.latch =
        (chip.latch & 0x00FF) |
        ((uint16_t)data << 8);

    chip.write_pos = 0;

    update_pins();

    printf(
        "PCF8575 WRITE: 0x%04X\n",
        chip.latch);
  }

  return true;
}

/*
 * End of I2C transaction.
 */
static void on_i2c_disconnect(
    void *user_data)
{
  (void)user_data;

  chip.write_pos = 0;
}

/*
 * Initialize chip.
 */
void chip_init(void)
{
  /*
   * Initial PCF8575 state:
   *
   * all pins HIGH/released.
   */
  chip.latch = 0xFFFF;

  chip.write_pos = 0;

  /*
   * Initialize P00-P15.
   */
  chip.pins[0] =
      pin_init("P00", INPUT_PULLUP);

  chip.pins[1] =
      pin_init("P01", INPUT_PULLUP);

  chip.pins[2] =
      pin_init("P02", INPUT_PULLUP);

  chip.pins[3] =
      pin_init("P03", INPUT_PULLUP);

  chip.pins[4] =
      pin_init("P04", INPUT_PULLUP);

  chip.pins[5] =
      pin_init("P05", INPUT_PULLUP);

  chip.pins[6] =
      pin_init("P06", INPUT_PULLUP);

  chip.pins[7] =
      pin_init("P07", INPUT_PULLUP);

  chip.pins[8] =
      pin_init("P10", INPUT_PULLUP);

  chip.pins[9] =
      pin_init("P11", INPUT_PULLUP);

  chip.pins[10] =
      pin_init("P12", INPUT_PULLUP);

  chip.pins[11] =
      pin_init("P13", INPUT_PULLUP);

  chip.pins[12] =
      pin_init("P14", INPUT_PULLUP);

  chip.pins[13] =
      pin_init("P15", INPUT_PULLUP);

  chip.pins[14] =
      pin_init("P16", INPUT_PULLUP);

  chip.pins[15] =
      pin_init("P17", INPUT_PULLUP);

  /*
   * Interrupt output.
   *
   * For now we initialize it HIGH.
   */
  chip.int_pin =
      pin_init(
          "INT",
          OUTPUT_HIGH);

  /*
   * I2C.
   */
  const i2c_config_t config = {

      .address = PCF8575_ADDRESS,

      .sda =
          pin_init(
              "SDA",
              INPUT_PULLUP),

      .scl =
          pin_init(
              "SCL",
              INPUT_PULLUP),

      .connect =
          on_i2c_connect,

      .read =
          on_i2c_read,

      .write =
          on_i2c_write,

      .disconnect =
          on_i2c_disconnect,

      .user_data = NULL};

  chip.i2c =
      i2c_init(&config);

  printf(
      "PCF8575 initialized at address 0x%02X\n",
      PCF8575_ADDRESS);
}