class timerMillis {
  public:
    timerMillis() {}
    timerMillis(uint32_t interval, bool active = false) {
      _interval = interval;
      reset();
      if (active) restart();
      else stop();
    }
    void setInterval(uint32_t interval) {
      _interval = (interval == 0) ? 1 : interval;
    }
    boolean isReady() {
      if (_active && millis() - _tmr >= _interval) {
        //_tmr += _interval;
        reset();
        return true;
      }
      return false;
    }
    void reset() {
      _tmr = millis();
    }
    void restart() {
      reset();
      _active = true;
    }
    void stop() {
      _active = false;
    }

  private:
    uint32_t _tmr = 0;
    uint32_t _interval = 0;
    boolean _active = false;
};
