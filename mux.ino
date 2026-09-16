#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <PCF8575.h>

const ADDR_MUXES = {0x20, 0x21};
const MUX_PINS = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

Adafruit_MCP4725 dac;
PCF8575 mux0 = new PCF8575(ADDR_MUXES[0]);
PCF8575 mux1 = new PCF8575(ADDR_MUXES[1]);

void mux_setup() {
  Wire.begin(ADDR_MUXES[0], ADDR_MUXES[1]);
  // init pinos MUXES como INPUT
  for(int i = 0; i < 17; i++) {
    mux0.pinMode(MUX_PINS[i], INPUT);
    mux1.pinMode(MUX_PINS[i], INPUT);
  }

  bool mux0Working = mux0.begin();
  bool mux1Working = mux1.begin();

  if(!mux0Working || !mux1Working) {
    Serial.println('erro no setup do mux');
    Serial.println(sprintf('mux 0 status: %s', mux0Working ? 'ok' : 'falha'));
    Serial.println(sprintf('mux 1 status: %s', mux1Working ? 'ok' : 'falha'));
    exit(1);
  }
}

void dac_setup() {
  // init pinos DAC
  @TODO;
  if(!dac.begin(0x62)) {
    exit(1);
  }
}

void setup() {
  Serial.begin(115200);
  mux_setup();
  dac_setup();
}

void dac_output(uint32_t valor)
{
  dac.setVoltage(valor, false);
}

void loop() {
  

}
