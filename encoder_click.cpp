#include "rotaryDecoderClick.h"
#include "config.h"

rotaryDecoderClick rd(MUX_BPM_CLICK_I2C_ADDR); // endereço I2C do PCF8575

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
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return;
    }
    if (!rd.begin(1, 2, 0xFFF8)) // A=P01, B=P02, botões P03..P17
    {
        Serial.println("PCF8575 nao encontrado");
        while (1)
            ;
    }

    rd.setActiveLow(true);  // botao liga o pino ao GND
    rd.setDebounce(5);      // ms
    rd.setStepsPerClick(4); // passos por detent do encoder
    rd.setValue((int32_t)MIN_BPM);

    pinMode(MUX_BPM_INTERRUPT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MUX_BPM_INTERRUPT_PIN), changed, FALLING);
}

bool encoder_click_update()
{
    if (newTickFlag && MUX_BPM_CLICK_ENABLED == 1)
    {
        rd.update();
        newTickFlag = false;
        return true;
    }
    return false;
}

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
    return rd.isOn((uint8_t)(i + 3))
}

bool is_bpm_active()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return false;
    }
    // pino 11 por:
    //      pinos 1 e 2 - rotação;
    //      pinos 3, 4, 5, 6, 7, 8, 9 e 10 - enable dos steps;
    //      pinto 11 - enable do BPM
    return rd.isOn(BPM_CLICK_PORT_NUMBER);
}

int32_t get_bpm_value()
{
    if (MUX_BPM_CLICK_ENABLED == 0)
    {
        return MIN_BPM;
    }

    return constrain(
        rd.getValue(),
        (int32_t)MIN_BPM,
        (int32_t)MAX_BPM);
}