#define CLICKS_TOUT 800

void button() {
#if (USE_BTN == 1)
  static bool flag = 0, holdFlag = 0, brDir = 0;
  static timerMillis stepTmr(80, true);
  static uint32_t tmr = 0;

  btn.tick();

  if (btn.isClick()) {
    btnClicks++;
    tmr = millis();
  }
  if (btnClicks > 0 && millis() - tmr > CLICKS_TOUT) {
    DEBUG("clicks: ");
    DEBUGLN(btnClicks);
    switch (btnClicks) {
      case 1:
        controlHandler(!cfg.state);
        break;
      case 2:
        changePreset(1);
        sendToSlaves(1, cfg.curPreset);
        break;
      case 3:
        changePreset(-1);
        sendToSlaves(1, cfg.curPreset);
        break;
      case 4:
        setPreset(0);
        sendToSlaves(1, cfg.curPreset);
        break;
      case 5:
        cfg.role = 0;
        blink16(CRGB::DarkSlateBlue);
        break;
      case 6:
        cfg.role = 1;
        blink16(CRGB::Maroon);
        break;
    }
    EE_updateCfg();
    btnClicks = 0;
  }

  if (cfg.state && btn.isHold()) {
    if (stepTmr.isReady()) {
      holdFlag = true;
      int temp = cfg.bright;
      temp += brDir ? 5 : -5;
      temp = constrain(temp, 0, 255);
      cfg.bright = temp;
      brTicks = cfg.bright / 25;
    }
  } else {
    if (holdFlag) {
      holdFlag = false;
      brDir = !brDir;
      brTicks = 0;
      DEBUG("Bright set to: ");
      DEBUGLN(cfg.bright);
      sendToSlaves(2, cfg.bright);
      EE_updateCfg();
    }
  }
#endif
}
