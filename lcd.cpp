
#include "TFT_eSPI.h"
#include <SPI.h>
#include <Arduino.h>

TFT_eSPI tft = TFT_eSPI();

void lcd_output(long steps[], int bpm, int num_steps)
{
    for (size_t i = 0; i < num_steps; i++)
    {
        tft.drawNumber(steps[0], 12 + (18 * i), 5, 2);
    }

    tft.drawString("BPM : ", 12, 40, 4);
    tft.drawNumber(bpm, 90, 40, 4);
}

void lcd_setup()
{
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(0x188B);
    tft.setTextColor(0xFF40, 0x188B);
}