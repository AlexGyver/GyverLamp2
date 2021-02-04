#pragma once
#define BTN_DEB 100
#define BTN_HOLD 800

// (пин, инверт), инверт 1 - для pullup, 0 - для pulldown

class Button {
  public:
    Button (byte pin) : _pin(pin) {
      pinMode(_pin, INPUT_PULLUP);
    }
    void setLevel(bool inv) {
      _inv = inv;
    }
    void tick() {
      uint32_t deb = millis() - _tmr;
      if (state()) {
        if (_flag && deb > BTN_HOLD) _hold = 1;
        if (!_flag && deb > BTN_DEB) _flag = 1;
      } else {
        if (_flag) {
          _flag = _hold = 0;
          if (deb < BTN_HOLD) _click = 1;
        }
        _tmr = millis();
      }
    }
    bool state() {
      return (digitalRead(_pin) ^ _inv);
    }
    bool isHold() {
      return _hold;
    }
    bool isClick() {
      if (_click) {
        _click = 0;
        return 1;
      } return 0;
    }
  private:
    const byte _pin;
    bool _inv = 1;
    uint32_t _tmr = 0;
    bool _flag = 0, _click = 0, _hold = 0;
};
