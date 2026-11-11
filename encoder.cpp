#include "rotaryDecoder8.h"

#define MIN_VALUE 0
#define MAX_VALUE 60
#define INTERRUPT_PIN 34
#define MUX_STEPS_ENABLED 0

rotaryDecoder8 decoder(0x20);

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
  pinMode(INTERRUPT_PIN, INPUT); // GPIO34 não tem pull-up interno
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), moved, FALLING);
  newTickFlag = false;

  Wire.begin();
  Wire.setClock(100000);
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
      (int32_t)MIN_VALUE,
      (int32_t)MAX_VALUE);

  return val;
}