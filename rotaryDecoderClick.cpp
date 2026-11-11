//
//    FILE: rotaryDecoderClick.cpp
// VERSION: 0.2.0
// PURPOSE: Biblioteca Arduino para PCF8575:
//          1 encoder rotativo + cliques (botoes) nos demais pinos
//    BASE: rotaryDecoder8 de Rob Tillaart
//          https://github.com/RobTillaart/rotaryDecoder8

#include "rotaryDecoderClick.h"

/////////////////////////////////////////////////////
//
//  CONSTRUTOR / BEGIN
//
rotaryDecoderClick::rotaryDecoderClick(const uint8_t address, TwoWire *wire)
{
  _address = address;
  _wire = wire;
}

bool rotaryDecoderClick::begin(uint8_t pinA, uint8_t pinB,
                               uint16_t buttonMask, bool initPins)
{
  if (pinA >= ROTDEC_CLICK_PINS)
    return false;
  if (pinB >= ROTDEC_CLICK_PINS)
    return false;
  if (pinA == pinB)
    return false;

  _pinA = pinA;
  _pinB = pinB;
  _buttonMask = buttonMask;
  _updateMasks();

  if (!isConnected())
    return false;

  //  PCF8575: pino em HIGH = entrada (quasi-bidirecional com pull-up fraco)
  if (initPins)
  {
    _writeValue = 0xFFFF;
    if (!write16(0xFFFF))
      return false;
  }

  readInitialState();
  return (_error == ROTDEC_CLICK_OK);
}

bool rotaryDecoderClick::isConnected()
{
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}

/////////////////////////////////////////////////////
//
//  CONFIGURACAO DOS BOTOES
//
void rotaryDecoderClick::setButtonMask(uint16_t mask)
{
  _buttonMask = mask;
  _updateMasks();
}

uint16_t rotaryDecoderClick::getButtonMask()
{
  return _buttonMask;
}

void rotaryDecoderClick::setActiveLow(bool activeLow)
{
  _activeLow = activeLow;
}

bool rotaryDecoderClick::getActiveLow()
{
  return _activeLow;
}

void rotaryDecoderClick::setDebounce(uint16_t ms)
{
  _debounce = ms;
}

uint16_t rotaryDecoderClick::getDebounce()
{
  return _debounce;
}

void rotaryDecoderClick::setToggleMask(uint16_t mask)
{
  _toggleMask = mask;
}

uint16_t rotaryDecoderClick::getToggleMask()
{
  return _toggleMask & _buttonMask;
}

/////////////////////////////////////////////////////
//
//  ENCODER
//
void rotaryDecoderClick::setDirection(uint8_t dir)
{
  _direction = dir ? 1 : 0;
}

uint8_t rotaryDecoderClick::getDirection()
{
  return _direction;
}

bool rotaryDecoderClick::setStepsPerClick(uint8_t spc)
{
  if (spc == 0)
    return false;
  _stepsPerClick = spc;
  return true;
}

uint8_t rotaryDecoderClick::getStepsPerClick()
{
  return _stepsPerClick;
}

int32_t rotaryDecoderClick::getValue()
{
  if (_direction)
    return -_encoder;
  return _encoder;
}

void rotaryDecoderClick::setValue(int32_t value)
{
  _encoder = _direction ? -value : value;
}

int32_t rotaryDecoderClick::getEncoderClicks()
{
  return getValue() / _stepsPerClick;
}

void rotaryDecoderClick::setEncoderClicks(int32_t clicks)
{
  setValue(clicks * _stepsPerClick);
}

void rotaryDecoderClick::reset()
{
  _encoder = 0;
  readInitialState(); //  atualiza a posicao anterior de A/B
}

uint8_t rotaryDecoderClick::getLastPosition()
{
  return _lastPos;
}

/////////////////////////////////////////////////////
//
//  LEITURA / ATUALIZACAO
//
uint16_t rotaryDecoderClick::readInitialState()
{
  uint16_t raw = read16();
  if (_error != ROTDEC_CLICK_OK)
    return raw;

  _lastValue = raw;
  _lastPos = ((raw >> _pinA) & 1) | (((raw >> _pinB) & 1) << 1);
  _stable = _candidate = _pressedBits(raw);
  return raw;
}

bool rotaryDecoderClick::checkChange()
{
  uint16_t raw = read16();
  if (_error != ROTDEC_CLICK_OK)
    return false;
  return (_lastValue != raw);
}

bool rotaryDecoderClick::update()
{
  uint16_t raw = read16();
  if (_error != ROTDEC_CLICK_OK)
    return false;

  //  o debounce depende de millis(), entao os botoes sao
  //  processados mesmo que o valor bruto nao tenha mudado.
  bool changed = _updateEncoder(raw);
  if (_updateButtons(raw))
    changed = true;
  _lastValue = raw;
  return changed;
}

/////////////////////////////////////////////////////
//
//  BOTOES
//
bool rotaryDecoderClick::isPressed(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  return (_stable & (1U << pin)) != 0;
}

uint16_t rotaryDecoderClick::getPressedMask()
{
  return _stable;
}

bool rotaryDecoderClick::wasPressed(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t bit = 1U << pin;
  bool ev = (_pressedEvents & bit) != 0;
  _pressedEvents &= ~bit;
  return ev;
}

bool rotaryDecoderClick::wasReleased(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t bit = 1U << pin;
  bool ev = (_releasedEvents & bit) != 0;
  _releasedEvents &= ~bit;
  return ev;
}

uint16_t rotaryDecoderClick::getPressedEvents(bool clear)
{
  uint16_t ev = _pressedEvents;
  if (clear)
    _pressedEvents = 0;
  return ev;
}

uint16_t rotaryDecoderClick::getReleasedEvents(bool clear)
{
  uint16_t ev = _releasedEvents;
  if (clear)
    _releasedEvents = 0;
  return ev;
}

uint16_t rotaryDecoderClick::getButtonClicks(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return 0;
  return _clickCount[pin];
}

bool rotaryDecoderClick::setButtonClicks(uint8_t pin, uint16_t value)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  _clickCount[pin] = value;
  return true;
}

void rotaryDecoderClick::resetButtonClicks()
{
  for (uint8_t i = 0; i < ROTDEC_CLICK_PINS; i++)
  {
    _clickCount[i] = 0;
  }
}

/////////////////////////////////////////////////////
//
//  SWITCHES ON/OFF
//
bool rotaryDecoderClick::isOn(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  return (_switchState & (1U << pin)) != 0;
}

uint16_t rotaryDecoderClick::getSwitchMask()
{
  return _switchState;
}

bool rotaryDecoderClick::setSwitch(uint8_t pin, bool on)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t bit = 1U << pin;
  if (!(_buttonMask & bit))
    return false; //  so vale para botoes
  if (on)
    _switchState |= bit;
  else
    _switchState &= ~bit;
  return true;
}

bool rotaryDecoderClick::toggleSwitch(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t bit = 1U << pin;
  if (!(_buttonMask & bit))
    return false;
  _switchState ^= bit;
  return true;
}

void rotaryDecoderClick::setSwitchMask(uint16_t mask)
{
  _switchState = mask & _buttonMask;
}

void rotaryDecoderClick::resetSwitches()
{
  _switchState = 0;
}

bool rotaryDecoderClick::wasSwitched(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t bit = 1U << pin;
  bool ev = (_switchEvents & bit) != 0;
  _switchEvents &= ~bit;
  return ev;
}

uint16_t rotaryDecoderClick::getSwitchEvents(bool clear)
{
  uint16_t ev = _switchEvents;
  if (clear)
    _switchEvents = 0;
  return ev;
}

/////////////////////////////////////////////////////
//
//  READ - WRITE interface
//
uint8_t rotaryDecoderClick::read1(uint8_t pin)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return LOW;
  uint16_t raw = read16();
  return (raw & (1U << pin)) ? HIGH : LOW;
}

bool rotaryDecoderClick::write1(uint8_t pin, uint8_t value)
{
  if (pin >= ROTDEC_CLICK_PINS)
    return false;
  uint16_t mask = 1U << pin;
  //  usa o valor "sombra" e nao a leitura: um botao pressionado
  //  leria 0 e acabaria sendo forcado a LOW (perdendo o modo entrada).
  uint16_t tmp = _writeValue;
  if (value == LOW)
    tmp &= ~mask;
  else
    tmp |= mask;
  return write16(tmp);
}

uint16_t rotaryDecoderClick::read16()
{
  if (_wire->requestFrom(_address, (uint8_t)2) != 2)
  {
    _error = ROTDEC_CLICK_ERR_I2C;
    return _lastValue;
  }
  _error = ROTDEC_CLICK_OK;
  uint16_t a = _wire->read(); //  LSB
  uint16_t b = _wire->read(); //  MSB
  return (b << 8) | a;
}

bool rotaryDecoderClick::write16(uint16_t bitMask)
{
  //  pinos de entrada (encoder + botoes) sempre ficam em HIGH
  _writeValue = bitMask | _inputMask;
  _wire->beginTransmission(_address);
  _wire->write(_writeValue & 0xFF); //  LSB
  _wire->write(_writeValue >> 8);   //  MSB
  bool ok = (_wire->endTransmission() == 0);
  _error = ok ? ROTDEC_CLICK_OK : ROTDEC_CLICK_ERR_I2C;
  return ok;
}

int8_t rotaryDecoderClick::lastError()
{
  return _error;
}

/////////////////////////////////////////////////////
//
//  PROTECTED
//
bool rotaryDecoderClick::_updateEncoder(uint16_t raw)
{
  //  bit0 = A, bit1 = B (mesma convencao da rotaryDecoder8)
  uint8_t currentPos = ((raw >> _pinA) & 1) | (((raw >> _pinB) & 1) << 1);
  if (currentPos == _lastPos)
    return false;

  uint8_t change = (_lastPos << 2) | currentPos;
  _lastPos = currentPos;

  switch (change)
  {
  case 0b0001:
  case 0b0111:
  case 0b1110:
  case 0b1000:
    _encoder++;
    return true;

  case 0b0010:
  case 0b0100:
  case 0b1101:
  case 0b1011:
    _encoder--;
    return true;

  //  0b0011, 0b0110, 0b1001, 0b1100 => salto de 2 passos,
  //  direcao indecidivel: ignorado (igual a rotaryDecoder8::update).
  default:
    break;
  }
  return false;
}

bool rotaryDecoderClick::_updateButtons(uint16_t raw)
{
  uint16_t pressed = _pressedBits(raw);

  //  caminho rapido: nada mudou e nenhum debounce pendente
  if ((pressed == _candidate) && (_candidate == _stable))
    return false;

  uint16_t now = (uint16_t)millis();
  bool changed = false;
  uint16_t bit = 1;

  for (uint8_t i = 0; i < ROTDEC_CLICK_PINS; i++, bit <<= 1)
  {
    if (!(_buttonMask & bit))
      continue;

    //  leitura mudou => reinicia o tempo de debounce desse botao
    if ((pressed & bit) != (_candidate & bit))
    {
      _candidate ^= bit;
      _stamp[i] = now;
    }

    //  candidato estavel por tempo suficiente => confirma
    if ((_candidate & bit) != (_stable & bit))
    {
      if ((uint16_t)(now - _stamp[i]) >= _debounce)
      {
        _stable ^= bit;
        if (_stable & bit)
        {
          _clickCount[i]++;
          _pressedEvents |= bit;
          //  switch ON/OFF: cada clique inverte o estado
          if (_toggleMask & bit)
          {
            _switchState ^= bit;
            _switchEvents |= bit;
          }
        }
        else
        {
          _releasedEvents |= bit;
        }
        changed = true;
      }
    }
  }
  return changed;
}

//  bit i = 1 => botao i esta pressionado (leitura bruta, sem debounce)
uint16_t rotaryDecoderClick::_pressedBits(uint16_t raw)
{
  if (_activeLow)
    raw = ~raw;
  return raw & _buttonMask;
}

void rotaryDecoderClick::_updateMasks()
{
  _encoderMask = (1U << _pinA) | (1U << _pinB);
  _buttonMask &= ~_encoderMask;
  _inputMask = _encoderMask | _buttonMask;

  //  descarta estados de pinos que deixaram de ser botoes
  _stable &= _buttonMask;
  _candidate &= _buttonMask;
  _pressedEvents &= _buttonMask;
  _releasedEvents &= _buttonMask;
  _switchState &= _buttonMask;
  _switchEvents &= _buttonMask;
}

//  -- END OF FILE --