#pragma once
//
//    FILE: rotaryDecoderClick.h
// VERSION: 0.2.0
// PURPOSE: Biblioteca Arduino para PCF8575:
//          - 1 encoder rotativo (pinos A e B configuraveis)
//          - cliques (botoes) nos demais pinos, com debounce e contadores
//          - cada clique pode funcionar como SWITCH ON/OFF (toggle)
//          (a rotacao dos outros encoders e tratada em outro arquivo)
//    BASE: rotaryDecoder8 de Rob Tillaart
//          https://github.com/RobTillaart/rotaryDecoder8
//
//  NUMERACAO DOS PINOS (indice de bit usado em toda a biblioteca):
//
//     P00..P07  ->  0..7
//     P10..P17  ->  8..15
//
//  PADRAO (alteravel em begin()):
//     Encoder:  A = P01 (1), B = P02 (2)
//     Botoes:   P03..P17 (mascara 0xFFF8)
//     P00 nao e usado.
//

#include "Arduino.h"
#include "Wire.h"

#define ROTARY_DECODER_CLICK_LIB_VERSION (F("0.2.0"))

const uint8_t ROTDEC_CLICK_PINS = 16;
const uint16_t ROTDEC_CLICK_DEFAULT_MASK = 0xFFF8; //  P03..P17

const int8_t ROTDEC_CLICK_OK = 0;
const int8_t ROTDEC_CLICK_ERR_I2C = -1;

class rotaryDecoderClick
{
public:
  explicit rotaryDecoderClick(const uint8_t address, TwoWire *wire = &Wire);

  //  pinA, pinB    = indices (0..15) do encoder.
  //  buttonMask    = bit i em 1 => pino i e um botao (clique).
  //                  os pinos A e B sao removidos da mascara automaticamente.
  //  initPins      = true => escreve 0xFFFF no PCF8575 (necessario para os
  //                  pinos funcionarem como entrada).
  //  nota: begin() NAO zera os contadores (use reset()).
  //  retorna false se parametros invalidos ou dispositivo nao responde.
  bool begin(uint8_t pinA = 1, uint8_t pinB = 2,
             uint16_t buttonMask = ROTDEC_CLICK_DEFAULT_MASK,
             bool initPins = true);
  bool isConnected();

  //  CONFIGURACAO DOS BOTOES
  void setButtonMask(uint16_t mask); //  A e B sao removidos da mascara
  uint16_t getButtonMask();
  //  true  = botao liga o pino ao GND (nivel LOW = pressionado)  [padrao]
  //  false = botao liga o pino ao VCC (nivel HIGH = pressionado)
  void setActiveLow(bool activeLow = true);
  bool getActiveLow();
  //  tempo de debounce em ms (0 = sem debounce, padrao 5).
  //  update() precisa ser chamado com frequencia para o debounce funcionar.
  void setDebounce(uint16_t ms);
  uint16_t getDebounce();
  //  quais botoes funcionam como switch ON/OFF (toggle a cada clique).
  //  bit i em 1 => cada clique no pino i inverte o estado ON/OFF.
  //  padrao: 0xFFFF (todos os botoes). Botoes fora da mascara continuam
  //  "momentaneos" (so isPressed / eventos / contador).
  void setToggleMask(uint16_t mask);
  uint16_t getToggleMask(); //  ja filtrada pela mascara dos botoes

  //  ENCODER
  //  0 = CW, outro valor = CCW. Nao zera o contador.
  void setDirection(uint8_t dir = 0);
  uint8_t getDirection();
  //  passos (transicoes de quadratura) por clique mecanico (detent). Padrao 1.
  bool setStepsPerClick(uint8_t spc); //  false se spc == 0
  uint8_t getStepsPerClick();

  int32_t getValue(); //  contador de passos
  void setValue(int32_t value = 0);
  int32_t getEncoderClicks(); //  getValue() / stepsPerClick
  void setEncoderClicks(int32_t clicks);
  void reset();              //  zera o contador do encoder
  uint8_t getLastPosition(); //  debug: estado anterior de A/B

  //  LEITURA / ATUALIZACAO
  //  le o PCF8575 e atualiza o encoder e os botoes.
  //  retorna true se o encoder andou ou algum botao mudou de estado (apos debounce).
  bool update();
  //  true se o valor bruto lido difere da ultima leitura (nao atualiza nada).
  bool checkChange();
  //  le o estado atual sem gerar eventos (use apos begin()).
  uint16_t readInitialState();

  //  BOTOES (pin = 0..15)
  bool isPressed(uint8_t pin); //  estado estavel (com debounce)
  uint16_t getPressedMask();   //  bit i = 1 => pino i pressionado

  //  eventos "travados": ficam 1 ate serem lidos (a leitura limpa o evento)
  bool wasPressed(uint8_t pin);  //  houve um clique (pressionou) ?
  bool wasReleased(uint8_t pin); //  houve soltura ?
  uint16_t getPressedEvents(bool clear = true);
  uint16_t getReleasedEvents(bool clear = true);

  //  contador de cliques (conta ao pressionar). 16 bits => volta a 0 em 65535.
  uint16_t getButtonClicks(uint8_t pin);
  bool setButtonClicks(uint8_t pin, uint16_t value = 0);
  void resetButtonClicks(); //  zera todos

  //  SWITCHES ON/OFF (pin = 0..15)
  //  cada clique (pressionamento confirmado) inverte o estado do switch.
  //  todos comecam em OFF. begin() nao altera o estado dos switches.
  bool isOn(uint8_t pin);               //  true = ON
  uint16_t getSwitchMask();             //  bit i = 1 => switch i esta ON
  bool setSwitch(uint8_t pin, bool on); //  forca ON/OFF (nao gera evento)
  bool toggleSwitch(uint8_t pin);       //  inverte por software (nao gera evento)
  void setSwitchMask(uint16_t mask);    //  define todos de uma vez (ex: restaurar da EEPROM)
  void resetSwitches();                 //  todos OFF

  //  eventos "travados": houve mudanca por clique fisico desde a ultima leitura?
  //  apos o evento, use isOn(pin) para saber se ficou ON ou OFF.
  bool wasSwitched(uint8_t pin); //  a leitura limpa o evento
  uint16_t getSwitchEvents(bool clear = true);

  //  READ - WRITE interface (para pinos que nao sao encoder/botao, ex: P00)
  //  write16/write1 nunca derrubam para LOW os pinos de entrada.
  uint8_t read1(uint8_t pin);
  bool write1(uint8_t pin, uint8_t value);
  uint16_t read16();
  bool write16(uint16_t bitMask);

  //  ROTDEC_CLICK_OK ou ROTDEC_CLICK_ERR_I2C (resultado do ultimo read16/write16)
  int8_t lastError();

protected:
  bool _updateEncoder(uint16_t raw);
  bool _updateButtons(uint16_t raw);
  uint16_t _pressedBits(uint16_t raw);
  void _updateMasks();

  uint8_t _address;
  TwoWire *_wire;
  int8_t _error = ROTDEC_CLICK_OK;

  //  configuracao
  uint8_t _pinA = 1;
  uint8_t _pinB = 2;
  uint16_t _encoderMask = 0x0006;
  uint16_t _buttonMask = ROTDEC_CLICK_DEFAULT_MASK;
  uint16_t _inputMask = 0xFFFE;
  bool _activeLow = true;
  uint16_t _debounce = 5;

  //  encoder
  int32_t _encoder = 0;
  uint8_t _lastPos = 0;
  uint8_t _direction = 0;
  uint8_t _stepsPerClick = 1;

  //  botoes
  uint16_t _stable = 0;    //  estado apos debounce
  uint16_t _candidate = 0; //  estado em debounce
  uint16_t _pressedEvents = 0;
  uint16_t _releasedEvents = 0;
  uint16_t _toggleMask = 0xFFFF; //  botoes que viram switch
  uint16_t _switchState = 0;     //  1 = ON
  uint16_t _switchEvents = 0;
  uint16_t _stamp[ROTDEC_CLICK_PINS] = {0}; //  millis() & 0xFFFF
  uint16_t _clickCount[ROTDEC_CLICK_PINS] = {0};

  //  I2C
  uint16_t _lastValue = 0;
  uint16_t _writeValue = 0xFFFF;
};

//  -- END OF FILE --