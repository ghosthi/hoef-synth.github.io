#include <Arduino.h>
#include "PCF8575.h"

#define NUM_EXPANDERS 2
#define TOTAL_ENCODERS 9

#define BUTTON_PIN_OFFSET 10 // botoes ocupam os pinos 10..14 de cada chip
#define DEBOUNCE_MS 30

#define BPM_MIN 60
#define BPM_MAX 360
#define BPM_DEFAULT 120

const int BPM_ENCODER = 0;
int bpm = BPM_DEFAULT;

const uint8_t encodersPerExpander[NUM_EXPANDERS] = {5, 4}; // 4 de steps em cada e um de BPM

// Endereços I2C dos dois PCF8575 (ajuste conforme A0/A1/A2 da sua fiação)
const uint8_t addr[NUM_EXPANDERS] = {0x20, 0x21};

long lastActiveValue = 0;

int expOffset[NUM_EXPANDERS];

void computeOffsets()
{
  expOffset[0] = 0;
  for (int i = 1; i < NUM_EXPANDERS; i++)
  {
    expOffset[i] = expOffset[i - 1] + encodersPerExpander[i - 1];
  }
}

PCF8575 pcf[NUM_EXPANDERS] = {
    PCF8575(addr[0]),
    PCF8575(addr[1])};

uint16_t lastPortState[NUM_EXPANDERS];
long encoderPos[TOTAL_ENCODERS] = {0};
uint8_t lastButtonState[TOTAL_ENCODERS];
unsigned long lastButtonTime[TOTAL_ENCODERS] = {0};
bool encoderEnabled[TOTAL_ENCODERS];

// Tabela de decodificacao de quadratura
// indice = (prevA prevB currA currB), valor = -1, 0 ou +1
const int8_t qdec[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

void mux_setup()
{
  int encoder_qtd = 0;
  for (int i = 0; i < NUM_EXPANDERS; i++)
  {
    encoder_qtd += encodersPerExpander[i];
  }
  if (encoder_qtd != (int)TOTAL_ENCODERS)
  {
    Serial.print("Número de encoders inconsistente");
    exit(1);
  }

  for (int i = 0; i < NUM_EXPANDERS; i++)
  {
    pcf[i].begin();
    pcf[i].write16(0xFFFF); // todos os pinos como entrada
    lastPortState[i] = pcf[i].read16();
    for (int re = 0; re < encodersPerExpander[i]; re++)
    {
      int globalIdx = expOffset[i] + re;
      int btnPin = BUTTON_PIN_OFFSET + re;
      lastButtonState[globalIdx] = (lastPortState[i] >> btnPin) & 1;
    }
  }
  for (int i = 0; i < TOTAL_ENCODERS; i++)
  {
    encoderEnabled[i] = true;
  }
}

void update_encoder_values()
{
  for (int i = 0; i < NUM_EXPANDERS; i++)
  {
    uint16_t current = pcf[i].read16();
    uint16_t changed = current ^ lastPortState[i];

    // --- rotacao (quadratura) ---
    if (changed)
    {
      for (int re = 0; re < encodersPerExpander[i]; re++)
      {
        int pinA = re * 2;
        int pinB = re * 2 + 1;
        uint16_t mask = (1 << pinA) | (1 << pinB);

        if (changed & mask)
        {
          uint8_t prevA = (lastPortState[i] >> pinA) & 1;
          uint8_t prevB = (lastPortState[i] >> pinB) & 1;
          uint8_t currA = (current >> pinA) & 1;
          uint8_t currB = (current >> pinB) & 1;

          uint8_t idx = (prevA << 3) | (prevB << 2) | (currA << 1) | currB;
          int globalIdx = expOffset[i] + re;

          if (globalIdx == BPM_ENCODER)
          {
            bpm += qdec[idx];
            bpm = constrain(bpm, BPM_MIN, BPM_MAX);
          }
          else if (encoderEnabled[globalIdx])
          {
            encoderPos[globalIdx] += qdec[idx];
          }
        }
      }
    }

    // --- botao  ---
    for (int re = 0; re < encodersPerExpander[i]; re++)
    {
      int btnPin = BUTTON_PIN_OFFSET + re;
      uint8_t btn = (current >> btnPin) & 1;
      int globalIdx = expOffset[i] + re;

      // borda de descida (HIGH -> LOW), com debounce
      if (btn == LOW && lastButtonState[globalIdx] == HIGH &&
          (millis() - lastButtonTime[globalIdx] > DEBOUNCE_MS))
      {
        lastButtonTime[globalIdx] = millis();
        if (globalIdx == BPM_ENCODER)
        {
          bpm = BPM_DEFAULT;
        }
        else
        {
          encoderEnabled[globalIdx] = !encoderEnabled[globalIdx];
        }
      }
      lastButtonState[globalIdx] = btn;
    }

    lastPortState[i] = current;
  }
}

float step_to_volt(long valor)
{
  return valor / 12.0f;
}

void get_steps(long values[])
{
  lastActiveValue = 0;
  int step_idx = 0;

  for (int i = 0; i < TOTAL_ENCODERS; i++)
  {
    if (i == BPM_ENCODER)
    {
      continue;
    }

    if (encoderEnabled[i])
    {
      lastActiveValue = encoderPos[i];
    }

    values[step_idx] = lastActiveValue;
    step_idx++;
  }
}

bool encoder_enabled(int i)
{
  return encoderEnabled[i];
}

int get_num_steps()
{
  return (int)TOTAL_ENCODERS - 1;
}

int get_bpm()
{
  return bpm;
}