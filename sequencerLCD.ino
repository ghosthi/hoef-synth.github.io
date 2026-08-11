#include <TFT_eSPI.h>
#include <SPI.h>
#include <Arduino.h>

TFT_eSPI tft = TFT_eSPI();

//definicao de pinos:
const int PIN_DAC_CV = 25;   // Saída CV
const int PIN_CLOCK = 26;    // Clock
const int PIN_GATE = 27;     // Gate




//variaveis globais
volatile uint8_t steps[8] = {0, 1, 2, 3, 4, 5, 6, 7};
volatile uint16_t BPM = 300;


// Handlers para referência das tarefas
TaskHandle_t entrada;
TaskHandle_t saida;



//funcoes

//funçao p converter de int pro valor de saida do DAC
float outputVoltage(uint8_t stepInt){
  return (stepInt * 0,275);
}



// Código executado pelo Núcleo 1
void codigoSaida(void * pvParameters) {
  for (;;) {


    //colocando os steps na tela
    tft.drawNumber(steps[0], 12, 5, 2);
    tft.drawNumber(steps[1], 30, 5, 2);
    tft.drawNumber(steps[2], 48, 5, 2);
    tft.drawNumber(steps[3], 66, 5, 2);
    tft.drawNumber(steps[4], 84, 5, 2);
    tft.drawNumber(steps[5], 102, 5, 2);
    tft.drawNumber(steps[6], 120, 5, 2);
    tft.drawNumber(steps[7], 138, 5, 2);
    
    //colocando o BPM na tela
    tft.drawString("BPM : ", 12, 40, 4);
    tft.drawNumber(BPM, 90, 40, 4);


    //aqui vao as partes da saída dos pinos clock, cvout e gate
    //dac.setVoltage(outputVoltage(steps[1]));


    vTaskDelay(100 / portTICK_PERIOD_MS); // Pausa de 0,1 segundo
  }
}





// Código executado pelo Núcleo 0
void codigoEntrada(void * pvParameters) {
  for (;;) {
    Serial.print("Tarefa 2 executando no núcleo: ");
    Serial.println(xPortGetCoreID());
    
    vTaskDelay(500 / portTICK_PERIOD_MS); // Pausa de 0.5 segundo
  }
}





void setup() {
  Serial.begin(115200);

  // setando o LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x188B);
  tft.setTextColor(0xFF40, 0x188B);

  // Cria a tarefa fixada no Núcleo 0
  xTaskCreatePinnedToCore(
    codigoEntrada,   /* Função que implementa a tarefa */
    "Task0",         /* Nome da tarefa */
    20000,           /* Tamanho da pilha (stack size em bytes) */
    NULL,            /* Parâmetro passado para a função */
    1,               /* Prioridade da tarefa (0 é a menor) */
    &entrada,        /* 3. CORRIGIDO: Passando o Handle da tarefa, não a função */
    1                /* Núcleo onde a tarefa será executada (0) */
  );

  // Cria a tarefa fixada no Núcleo 1
  xTaskCreatePinnedToCore(
    codigoSaida,     /* Função que implementa a tarefa */
    "saidas",        /* Nome da tarefa */
    20000,           /* Tamanho da pilha */
    NULL,            /* Parâmetro passado para a função */
    1,               /* Prioridade da tarefa */
    &saida,          /* 3. CORRIGIDO: Passando o Handle da tarefa, não a função */
    0                /* Núcleo onde a tarefa será executada (1) */
  );
}

void loop() {
  // Como as tarefas acima rodam em loops infinitos próprios,
  // a tarefa do loop() principal pode ser deletada para liberar memória.
  vTaskDelete(NULL);
 }