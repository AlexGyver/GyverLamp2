void presetRotation(bool force) {
  if (cfg.rotation && (now.newMin() || force)) {   // если автосмена и новая минута    
    if (cfg.rotRnd) {                   // случайная
      cfg.curPreset = trnd.fromMin(cfg.rotPeriod, cfg.presetAmount);
      DEBUG("Rnd changed to ");
      DEBUGLN(cfg.curPreset);
    } else {                            // по порядку
      cfg.curPreset = ((trnd.getMin() / cfg.rotPeriod) % cfg.presetAmount);
      DEBUG("In order changed to ");
      DEBUGLN(cfg.curPreset);
    }
  }
}

void changePreset(int dir) {
  if (!cfg.rotation) {    // ручная смена
    cfg.curPreset += dir;
    if (cfg.curPreset >= cfg.presetAmount) cfg.curPreset = 0;
    if (cfg.curPreset < 0) cfg.curPreset = cfg.presetAmount - 1;
    DEBUG("Preset changed to ");
    DEBUGLN(cfg.curPreset);
  }
}

void setPreset(byte pres) {
  if (!cfg.rotation) {    // ручная смена
    cfg.curPreset = constrain(pres, 0, cfg.presetAmount - 1);
    DEBUG("Preset set to ");
    DEBUGLN(cfg.curPreset);
  }
}

void controlHandler(bool state) {
  if (turnoffTmr.running()) {
    turnoffTmr.stop();
    FastLED.clear();
    DEBUGLN("stop off timer");
    return;
  }
  if (dawnTmr.running()) {
    dawnTmr.stop();
    FastLED.clear();
    DEBUGLN("stop dawn timer");
    return;
  }  
  if (state) cfg.manualOff = 0;
  if (cfg.state && !state) cfg.manualOff = 1;
  setPower(state);
}

void setPower(bool state) {
  cfg.state = state;
  if (!state) {
    delay(100);     // чтобы пролететь мин. частоту обновления
    FastLED.clear();
    FastLED.show();
  }
  sendToSlaves(0, cfg.state);
  DEBUGLN(state ? "Power on" : "Power off");
}
