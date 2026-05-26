#include <Arduino.h>
#include <ESP32Encoder.h>
#include "AiEsp32RotaryEncoder.h"

// =========================================================
// PINOS DE SAÍDA
// =========================================================

const int PIN_DAC_CV = 25;   // Saída CV
const int PIN_CLOCK = 26;    // Clock
const int PIN_GATE = 27;     // Gate

// =========================================================
// PINOS DOS ENCODERS DE PITCH
// =========================================================

const int pinosPitch[8][2] = {
  {13, 14},
  {16, 17},
  {18, 19},
  {21, 22},
  {23, 4},
  {15, 5},
  {32, 33},
  {34, 35} //com resistor 10k
};

// Encoder BPM
const int PIN_BPM_A = 36; //pino sp (com resistor10k)
const int PIN_BPM_B = 3; //pino rxd ou rx

// =========================================================
// VARIÁVEIS GLOBAIS
// =========================================================

uint8_t valoresPitch[8] = {0, 0, 0, 0, 0, 0, 0, 0};

volatile uint16_t bpmAtual = 120;
volatile uint8_t stepAtual = 0;

// =========================================================
// OBJETOS
// =========================================================

ESP32Encoder pitchEncoders[8];

AiEsp32RotaryEncoder bpmEncoder(
  PIN_BPM_A,
  PIN_BPM_B,
  -1,
  -1,
  4
);

hw_timer_t *timer = NULL;

// =========================================================
// INTERRUPÇÃO DO TIMER
// =========================================================

void IRAM_ATTR onTimer() {
  uint8_t step = stepAtual;
  uint8_t pitch = valoresPitch[step];
  uint8_t dacValor = pitch * 21;

  dacWrite(PIN_DAC_CV, dacValor);

  digitalWrite(PIN_CLOCK, HIGH);

  if (pitch > 0) {
    digitalWrite(PIN_GATE, HIGH);
  }

//  delayMicroseconds(10000);

  digitalWrite(PIN_CLOCK, LOW);
  digitalWrite(PIN_GATE, LOW);

  stepAtual = (stepAtual + 1) % 8;
}

// =========================================================
// ISR DO ENCODER BPM
// =========================================================

void IRAM_ATTR isr_bpm() {
  bpmEncoder.readEncoder_ISR();
}

// =========================================================
// SETUP
// =========================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_GATE, OUTPUT);
  digitalWrite(PIN_CLOCK, LOW);
  digitalWrite(PIN_GATE, LOW);

  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  for (int i = 0; i < 8; i++) {
    pitchEncoders[i].attachSingleEdge(pinosPitch[i][0], pinosPitch[i][1]);
    pitchEncoders[i].setFilter(1023);
    pitchEncoders[i].setCount(0);
  }

  bpmEncoder.begin();
  bpmEncoder.setup(isr_bpm);
  bpmEncoder.setBoundaries(30, 250, false);
  bpmEncoder.setEncoderValue(120);

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);

  uint64_t usPorBatida = 60000000ULL / bpmAtual;
  timerAlarm(timer, usPorBatida, true, 0);

  Serial.println("Sequenciador iniciado");
}

// =========================================================
// LOOP PRINCIPAL
// =========================================================

void loop() {
  bool houveMudanca = false;

  if (bpmEncoder.encoderChanged()) {
    bpmAtual = bpmEncoder.readEncoder();

    uint64_t usPorBatida = 60000000ULL / bpmAtual;
    timerAlarm(timer, usPorBatida, true, 0);

    houveMudanca = true;
  }

  for (int i = 0; i < 8; i++) {
    int32_t contagem = pitchEncoders[i].getCount();

    if (contagem < 0) {
      pitchEncoders[i].setCount(0);
      contagem = 0;
    } else if (contagem > 12) {
      pitchEncoders[i].setCount(12);
      contagem = 12;
    }

    uint8_t novoValor = (uint8_t)contagem;

    if (novoValor != valoresPitch[i]) {
      valoresPitch[i] = novoValor;
      houveMudanca = true;
    }
  }

  if (houveMudanca) {
    Serial.println();
    Serial.println("========== SEQUENCIADOR ==========");

    Serial.print("BPM: ");
    Serial.println(bpmAtual);

    Serial.print("CLOCK GPIO: ");
    Serial.println(PIN_CLOCK);

    Serial.print("GATE GPIO: ");
    Serial.println(PIN_GATE);

    Serial.println();

    for (int i = 0; i < 8; i++) {
      Serial.print("STEP ");
      Serial.print(i + 1);
      Serial.print(" -> PITCH: ");
      Serial.print(valoresPitch[i]);

      if (valoresPitch[i] > 0) {
        Serial.print(" | GATE: ON");
      } else {
        Serial.print(" | GATE: OFF");
      }

      Serial.print(" | DAC: ");
      Serial.print(valoresPitch[i] * 21);
      Serial.println();
    }

    Serial.println("=================================");
  }

  delay(2);
}
