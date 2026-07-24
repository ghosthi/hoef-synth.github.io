#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI ();

void setup() {

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x188B);
  tft.setTextColor(0xFF40, 0x188B);
  

}

void loop() {

  int i = 0;
  int j = 10;
  int vetor[9] = {0,1,2,3,4,5,6,7,8};

  for(i = 1; i < 9; i++){
    vetor[i] = rand()%9 + 1;
    tft.drawNumber(vetor[i], j, 5, 2);
    j = j + 18;
    delay(750);

    tft.drawNumber(j, 60, 40, 4);
    }
  

}