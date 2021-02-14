bool EEcfgFlag = false;
bool EEdawnFlag = false;
bool EEpresetFlag = false;

void EE_startup() {
  // старт епром
  if (EEPROM.read(511) != EE_KEY) {
    EEPROM.write(511, EE_KEY);
    EEPROM.put(0, cfg);
    EEPROM.put(sizeof(cfg), dawn);
    EEPROM.put(sizeof(cfg) + sizeof(dawn), preset);
    EEPROM.commit();
    blink8(CRGB::Pink);
    DEBUGLN("First start");
  }
  EEPROM.get(0, cfg);
  EEPROM.get(sizeof(cfg), dawn);
  EEPROM.get(sizeof(cfg) + sizeof(dawn), preset);

  // запускаем всё
  //trnd.setChannel(cfg.group);
  FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, cfg.maxCur * 100);
}

void EE_updateCfg() {
  EEcfgFlag = true;
  EEtmr.restart();
}
void EE_updateDawn() {
  EEdawnFlag = true;
  EEtmr.restart();
}
void EE_updatePreset() {
  EEpresetFlag = true;
  EEtmr.restart();
}
void checkEEupdate() {
  if (EEtmr.isReady()) {
    if (EEcfgFlag || EEdawnFlag || EEpresetFlag) {
      if (EEcfgFlag) {
        EEcfgFlag = false;
        EEPROM.put(0, cfg);
        DEBUGLN("save cfg");
      }
      if (EEdawnFlag) {
        EEdawnFlag = false;
        EEPROM.put(sizeof(cfg), dawn);
        DEBUGLN("save dawn");
      }
      if (EEpresetFlag) {
        EEpresetFlag = false;
        EEPROM.put(sizeof(cfg) + sizeof(dawn), preset);
        DEBUGLN("save preset");
      }
      EEPROM.commit();
    }
    EEtmr.stop();
  }
}

void EE_updCfgRst() {
  EE_updCfg();
  delay(100);
  ESP.restart();
}
void EE_updCfg() {
  EEPROM.put(0, cfg);
  EEPROM.commit();
}
