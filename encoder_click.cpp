#include "rotaryDecoderClick.h"
#include "config.h"

rotaryDecoderClick rd(MUX_BPM_CLICK_I2C_ADDR); // endereço I2C do PCF8575
volatile bool newTickFlag_bpm = false;
extern SemaphoreHandle_t i2cMutex;

bool is_step_active(uint8_t i)
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return true;
    }
    if (i > 8 || i < 0)
    {
        Serial.print("\nValor do parametro i: ");
        Serial.print(i);
        Serial.print(" para encoder_click_get_switch inválido. Deve ser entre 1 e 8!\n");
        while (1)
            ;
    }
    // clicks 1, 2, 3, 4, 5 e 6, 7 e 8 nos bits 3..10
    return rd.isOn((uint8_t)(i + 2));
}

int32_t get_bpm_value()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return MIN_BPM;
    }

    return constrain(
        rd.getEncoderClicks(),
        (int32_t)MIN_BPM,
        (int32_t)MAX_BPM);
}

bool is_bpm_active()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return false;
    }
    // pino 11 por:
    //      pinos 0 e 1 - rotação;
    //      pinos 2, 3, 4, 5, 6, 7, 10, 11 - enable dos steps;
    //      pino 12 - enable do BPM (indexado em 1 então port 13)
    return rd.isOn(BPM_CLICK_PORT_NUMBER);
}

void bpmClickdebug()
{
    if (DEBUG == 0)
    {
        return;
    }
    Serial.print("Active: ");
    for (int i = 0; i < 8; i++)
    {
        if (is_step_active(i))
        {
            Serial.print("1 ");
        }
        else
        {
            Serial.print("0 ");
        }
    }
    Serial.print("\n BPM: ");
    Serial.print(is_bpm_active() ? "(ativo) " : "(inativo)");
    Serial.println(rd.getEncoderClicks());
}

void IRAM_ATTR changed()
{
    const int IRQ_PULSES_PER_TICK = 1;
    static int count = 0;
    count++;
    if (count == IRQ_PULSES_PER_TICK)
    {
        count = 0;
        newTickFlag_bpm = true;
    }
}

void encoder_click_setup()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return;
    }
    if (!rd.begin(0, 1, 0x07FC)) // A=P00, B=P01, botões P03..P17
    {
        Serial.println("PCF8575 nao encontrado");
        while (1)
            ;
    }

    rd.setActiveLow(true);  // botao liga o pino ao GND
    rd.setDebounce(5);      // ms
    rd.setStepsPerClick(4); // passos por detent do encoder
    rd.setValue((int32_t)MIN_BPM);
    rd.resetSwitches(true);

    pinMode(MUX_BPM_INTERRUPT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MUX_BPM_INTERRUPT_PIN), changed, FALLING);
    Serial.print("SETUP MUX CLICK OK\n");
}

bool encoder_click_update()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return false;
    }
    if (!newTickFlag_bpm && !rd.isDebouncing())
    {
        return false;
    }

    newTickFlag_bpm = false;

    xSemaphoreTake(i2cMutex, portMAX_DELAY);
    rd.update();
    xSemaphoreGive(i2cMutex);
    if (rd.getEncoderClicks() < (int32_t)MIN_BPM)
    {
        rd.setEncoderClicks(MIN_BPM);
    }
    else if (rd.getEncoderClicks() > (int32_t)MAX_BPM)
    {
        rd.setEncoderClicks(MAX_BPM);
    }
    newTickFlag_bpm = false;
    bpmClickdebug();
    return true;
}
