bool EEcfgFlag = false;
bool EEdawnFlag = false;
bool EEpresetFlag = false;
bool EEpalFlag = false;

void EE_startup() {
  // старт епром
  EEPROM.begin(1000);   // старт епром
  delay(100);
  if (EEPROM.read(0) != EE_KEY) {
    EEPROM.write(0, EE_KEY);
    EEPROM.put(1, cfg);
    EEPROM.put(sizeof(cfg) + 1, dawn);
    EEPROM.put(sizeof(cfg) + sizeof(dawn) + 1, pal);
    EEPROM.put(sizeof(cfg) + sizeof(dawn) + sizeof(pal) + 1, preset);
    EEPROM.commit();
    blink16(CRGB::Magenta);
    DEBUGLN("First start");
  }
  EEPROM.get(1, cfg);
  EEPROM.get(sizeof(cfg) + 1, dawn);
  EEPROM.get(sizeof(cfg) + sizeof(dawn) + 1, pal);
  EEPROM.get(sizeof(cfg) + sizeof(dawn) + sizeof(pal) + 1, preset);

  DEBUG("EEPR size: ");
  DEBUGLN(sizeof(cfg) + sizeof(dawn) + sizeof(pal) + sizeof(preset) + 1);

  // запускаем всё
  FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, cfg.maxCur * 100);
  updPal();
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
void EE_updatePal() {
  EEpalFlag = true;
  EEtmr.restart();
}
void checkEEupdate() {
  if (EEtmr.isReady()) {
    if (EEcfgFlag || EEdawnFlag || EEpresetFlag) {
      if (EEcfgFlag) {
        EEcfgFlag = false;
        EEPROM.put(1, cfg);
        DEBUGLN("save cfg");
      }
      if (EEdawnFlag) {
        EEdawnFlag = false;
        EEPROM.put(sizeof(cfg) + 1, dawn);
        DEBUGLN("save dawn");
      }
      if (EEpalFlag) {
        EEpalFlag = false;
        EEPROM.put(sizeof(cfg) + sizeof(dawn) + 1, pal);
        DEBUGLN("save pal");
      }
      if (EEpresetFlag) {
        EEpresetFlag = false;
        EEPROM.put(sizeof(cfg) + sizeof(dawn) + sizeof(pal) + 1, preset);
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
  EEPROM.put(1, cfg);
  EEPROM.commit();
}
