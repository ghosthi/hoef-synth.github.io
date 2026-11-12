#include "rotaryDecoder8.h"
#include "config.h"

rotaryDecoder8 decoder(MUX_STEP_I2C_ADDR);

volatile bool newTickFlag = false;
// criado no setup da main
extern SemaphoreHandle_t i2cMutex;

void moved()
{
  const int IRQ_PULSES_PER_TICK = 1;
  static int count = 0;
  count++;
  if (count == IRQ_PULSES_PER_TICK)
  {
    count = 0;
    newTickFlag = true;
  }
}

void mux_encoder_setup()
{
  if (MUX_STEPS_ENABLED == 0)
  {
    return;
  }
  pinMode(MUX_STEPS_INTERRUPT_PIN, INPUT); // GPIO34 não tem pull-up interno
  attachInterrupt(digitalPinToInterrupt(MUX_STEPS_INTERRUPT_PIN), moved, FALLING);
  newTickFlag = false;

  decoder.begin(8);
  decoder.readInitialState();
  Serial.print("SETUP MUX OK\n");
}

bool mux_encoder_update()
{
  if (newTickFlag && MUX_STEPS_ENABLED == 1)
  {
    xSemaphoreTake(i2cMutex, portMAX_DELAY);
    decoder.update();
    xSemaphoreGive(i2cMutex);
    newTickFlag = false;
    return true;
  }
  return false;
}

int32_t mux_encoder_get_values(uint8_t i)
{
  int32_t val = constrain(
      decoder.getValue(i),
      (int32_t)STEP_MIN_VALUE,
      (int32_t)STEP_MAX_VALUE);

  return val;
}