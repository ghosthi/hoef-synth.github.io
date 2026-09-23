// padrao
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <vector>
// instaladas
#include "PCF8575.h"
#include "TFT_eSPI.h"
#include "Adafruit_MCP4725.h"
// proprias
#include "dac.h"
#include "mux.h"
#include "lcd.h"

const int MAX_STEPS = 16;
const int PIN_CLOCK = 26; // Clock
const int PIN_GATE = 27;  // Gate

int NUM_STEPS = get_num_steps();
long steps[MAX_STEPS] = {0};
int current_step = 0;
bool halfBeat = false;

volatile bool dacUpdate = false;

// Handlers para referência das tarefas
TaskHandle_t entrada;
TaskHandle_t saida;

hw_timer_t *timer = NULL;

// Código executado pelo Núcleo 1
void output_task(void *pvParameters)
{
  int bpm = get_bpm();
  int loop_bpm = bpm;
  while (true)
  {
    loop_bpm = get_bpm();
    get_steps(steps); // seta os steps por referencia

    if (dacUpdate)
    {
      dacUpdate = false;
      dac_output(step_to_volt(steps[current_step]));
    }

    if (loop_bpm != bpm)
    {
      bpm = loop_bpm;

      uint64_t usPorBatida = 60000000ULL / bpm;
      uint64_t usPorMeioBeat = usPorBatida / 2;

      timerAlarm(timer, usPorMeioBeat, true, 0);
    }
    lcd_output(steps, bpm, NUM_STEPS);
    vTaskDelay(10 / portTICK_PERIOD_MS); // Pausa de 0,1 segundo
  }
}

void IRAM_ATTR outputDacClockGate()
{
  if (!halfBeat)
  {
    uint8_t step = current_step;
    dacUpdate = true;

    digitalWrite(PIN_CLOCK, HIGH);

    if (encoder_enabled(current_step))
    {
      digitalWrite(PIN_GATE, HIGH);
    }
    else
    {
      digitalWrite(PIN_GATE, LOW);
    }

    halfBeat = true;
  }
  else
  {
    digitalWrite(PIN_CLOCK, LOW);
    digitalWrite(PIN_GATE, LOW);

    current_step = (current_step + 1) % NUM_STEPS;

    halfBeat = false;
  }
}

// Código executado pelo Núcleo 0
void input_task(void *pvParameters)
{
  while (true)
  {
    update_encoder_values();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Pausa de 0.5 segundo
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(400000);

  lcd_setup();
  mux_setup();
  dac_setup();

  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_GATE, OUTPUT);
  digitalWrite(PIN_CLOCK, LOW);
  digitalWrite(PIN_GATE, LOW);

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &outputDacClockGate);
  uint64_t usPorBatida = 60000000ULL / get_bpm();
  uint64_t usPorMeioBeat = usPorBatida / 2;

  timerAlarm(timer, usPorMeioBeat, true, 0);

  // Cria a tarefa fixada no Núcleo 0
  xTaskCreatePinnedToCore(
      input_task, /* Função que implementa a tarefa */
      "input",    /* Nome da tarefa */
      20000,      /* Tamanho da pilha (stack size em bytes) */
      NULL,       /* Parâmetro passado para a função */
      1,          /* Prioridade da tarefa (0 é a menor) */
      &entrada,   /* 3. CORRIGIDO: Passando o Handle da tarefa, não a função */
      0           /* Núcleo onde a tarefa será executada (0) */
  );

  // Cria a tarefa fixada no Núcleo 1
  xTaskCreatePinnedToCore(
      output_task, /* Função que implementa a tarefa */
      "output",    /* Nome da tarefa */
      20000,       /* Tamanho da pilha */
      NULL,        /* Parâmetro passado para a função */
      1,           /* Prioridade da tarefa */
      &saida,      /* 3. CORRIGIDO: Passando o Handle da tarefa, não a função */
      1            /* Núcleo onde a tarefa será executada (1) */
  );
}

void loop()
{
  // Como as tarefas acima rodam em loops infinitos próprios,
  // a tarefa do loop() principal pode ser deletada para liberar memória.
  vTaskDelete(NULL);
}