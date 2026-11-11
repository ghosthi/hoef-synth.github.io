// padrao
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <vector>
// instaladas
#include "PCF8575.h"
#include "TFT_eSPI.h"
#include "Adafruit_MCP4725.h"
#include "rotaryDecoder8.h"
// proprias
#include "dac.h"
#include "encoder_click.h"
#include "lcd.h"
#include "encoder.h"

#define CLK_GATE_ENABLED 0

const int DEBUG = 1;
const int PIN_CLOCK = 26; // Clock
const int PIN_GATE = 27;  // Gate

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
const int NUM_STEPS = 8;
// shared data
int32_t steps[NUM_STEPS] = {0};
// output only data
int32_t output_steps[NUM_STEPS] = {0};
int current_step = 0;
bool halfBeat = false;
bool updated = false;

volatile bool dacUpdate = false;
volatile bool output_flag = false;

// Handlers para referência das tarefas
TaskHandle_t entrada;
TaskHandle_t saida;

hw_timer_t *timer = NULL;

SemaphoreHandle_t i2cMutex;

void outputClockGate()
{
  if (!output_flag || CLK_GATE_ENABLED == 0)
  {
    return;
  }
  if (!halfBeat)
  {
    debug();
    dacUpdate = true;

    digitalWrite(PIN_CLOCK, HIGH);
    digitalWrite(PIN_GATE, is_step_active(current_step) ? HIGH : LOW);

    halfBeat = true;
  }
  else
  {
    digitalWrite(PIN_CLOCK, LOW);
    digitalWrite(PIN_GATE, LOW);

    current_step = (current_step + 1) % NUM_STEPS;

    halfBeat = false;
  }

  output_flag = false;
}

void IRAM_ATTR outputFlag()
{
  output_flag = true;
}

void debug()
{
  if (DEBUG == 0)
  {
    return;
  }
  Serial.print("Steps: ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(output_steps[i]);
    Serial.print(" ");
  }
  Serial.print("\n");

  Serial.print(" BPM: ");
  Serial.print(get_bpm_value());
  Serial.println();
}

// copia o shared para output_steps
void update_output_steps_value()
{
  portENTER_CRITICAL(&mux);
  memcpy(output_steps, steps, sizeof(steps));
  portEXIT_CRITICAL(&mux);
}

void update_steps_value()
{
  int32_t tmp[NUM_STEPS];
  for (uint8_t i = 0; i < NUM_STEPS; i++)
  {
    tmp[i] = mux_encoder_get_values(i);
  }

  portENTER_CRITICAL(&mux);
  memcpy(steps, tmp, sizeof(steps));
  portEXIT_CRITICAL(&mux);
}

// Código executado pelo Núcleo 1
void output_task(void *pvParameters)
{
  int bpm = get_bpm_value();
  int loop_bpm = bpm;
  while (true)
  {
    update_output_steps_value();
    if (is_bpm_active())
    {
      loop_bpm = get_bpm_value();
    }
    outputClockGate(); // atualiza dacUpdate quando output_flag == TRUE && halfBeat == FALSE
    if (dacUpdate)
    {
      dacUpdate = false;
      dac_output(output_steps[current_step] / 12.0f);
    }

    if (loop_bpm != bpm)
    {
      bpm = loop_bpm;

      uint64_t usPorBatida = 60000000ULL / bpm;
      uint64_t usPorMeioBeat = usPorBatida / 2;

      timerAlarm(timer, usPorMeioBeat, true, 0);
    }
    if (updated)
    {
      lcd_output(output_steps, bpm, NUM_STEPS);
    }
    vTaskDelay(50 / portTICK_PERIOD_MS); // Pausa de 0,5 segundo
  }
}

// Código executado pelo Núcleo 0
void input_task(void *pvParameters)
{
  while (true)
  {
    updated = mux_encoder_update();
    if (updated)
    {
      update_steps_value();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Pausa de 0.5 segundo
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.setClock(400000);

  i2cMutex = xSemaphoreCreateMutex();

  lcd_setup();
  mux_encoder_setup();
  dac_setup();
  encoder_click_setup();

  if (CLK_GATE_ENABLED == 1)
  {
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_GATE, OUTPUT);
    digitalWrite(PIN_CLOCK, LOW);
    digitalWrite(PIN_GATE, LOW);
  }

  Serial.print("SETUP DONE\n");

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &outputFlag);
  uint64_t usPorBatida = 60000000ULL / get_bpm_value();
  uint64_t usPorMeioBeat = usPorBatida / 2;

  timerAlarm(timer, usPorMeioBeat, true, 0);

  // Cria a tarefa fixada no Núcleo 0
  xTaskCreatePinnedToCore(
      input_task, // Função que implementa a tarefa //
      "input",    // Nome da tarefa //
      20000,      // Tamanho da pilha (stack size em bytes) //
      NULL,       // Parâmetro passado para a função //
      1,          // Prioridade da tarefa (0 é a menor) //
      &entrada,   // 3. CORRIGIDO: Passando o Handle da tarefa, não a função //
      0           // Núcleo onde a tarefa será executada (0) //
  );

  // Cria a tarefa fixada no Núcleo 1
  xTaskCreatePinnedToCore(
      output_task, // Função que implementa a tarefa //
      "output",    // Nome da tarefa //
      20000,       // Tamanho da pilha //
      NULL,        // Parâmetro passado para a função //
      1,           // Prioridade da tarefa //
      &saida,      // 3. CORRIGIDO: Passando o Handle da tarefa, não a função //
      1            // Núcleo onde a tarefa será executada (1) //
  );
}

// Como as tarefas acima rodam em loops infinitos próprios,
// a tarefa do loop() principal pode ser deletada para liberar memória.
void loop()
{
  vTaskDelete(NULL);
}