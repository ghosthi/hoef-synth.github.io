#include "rotaryDecoderClick.h"

#define INTERRUPT_PIN 34
#define MIN_BPM 60
#define MAX_BPM 300

rotaryDecoderClick rd(0x20); // endereço I2C do PCF8575

void changed()
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

void encoder_click_setup()
{
    if (!rd.begin(1, 2, 0xFFF8)) // A=P01, B=P02, botões P03..P17
    {
        Serial.println("PCF8575 nao encontrado");
        while (1)
            ;
    }

    rd.setActiveLow(true);  // botao liga o pino ao GND
    rd.setDebounce(5);      // ms
    rd.setStepsPerClick(4); // passos por detent do encoder

    pinMode(INTERRUPT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), changed, FALLING);
}

bool encoder_click_update()
{
    if (newTickFlag)
    {
        rd.update();
        newTickFlag = false;
        return true;
    }
    return false;
}

bool is_step_active(uint8_t i)
{
    if (i > 8 || i < 0)
    {
        Serial.print("\nValor do parametro i: ");
        Serial.print(i);
        Serial.print(" para encoder_click_get_switch inválido. Deve ser entre 1 e 8!\n");
        while (1)
            ;
    }
    return rd.isOn((uint8_t)(i + 3))
}

bool is_bpm_active()
{
    return rd.isOn(11);
}

int32_t get_bpm_value()
{
    return constrain(
        rd.getValue(),
        (int32_t)MIN_BPM,
        (int32_t)MAX_BPM);
}