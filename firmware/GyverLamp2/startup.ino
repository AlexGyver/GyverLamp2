void checkButton() {
#if (USE_BTN == 1)
  DEBUGLN(cfg.WiFimode ? "local mode" : "AP mode");
  if (btn.isHold()) {          // кнопка зажата
    FastLED.clear();
    byte count = 0;
    bool state = 0;

    while (btn.state()) {     // пока зажата кнопка
      fill_solid(leds, constrain(count, 0, 8), CRGB::Red);
      count++;
      if (count == 9) {               // на счёт 9 поднимаем яркость и флаг
        FastLED.setBrightness(120);
        state = 1;
      } else if (count == 16) {       // на счёт 16 опускаем флаг выходим
        state = 0;
        break;
      }
      FastLED.show();
      delay(300);
    }
    if (state) {
      DEBUGLN("change mode");
      cfg.WiFimode = !cfg.WiFimode;
      EEPROM.put(0, cfg);
      EEPROM.commit();
      delay(100);
      ESP.restart();
    }
  }
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();
#endif
}

void checkGroup() {
  fill_solid(leds, cfg.group, (cfg.WiFimode) ? (CRGB::Blue) : (CRGB::Green));
  FastLED.show();
  uint32_t tmr = millis();
  bool flag = 0;
  while (millis() - tmr < 3000) {
#if (USE_BTN == 1)
    btn.tick();
    if (btn.isClick()) {
      if (++cfg.group > 10) cfg.group = 1;
      FastLED.clear();
      fill_solid(leds, cfg.group, (cfg.WiFimode) ? (CRGB::Blue) : (CRGB::Green));
      FastLED.show();
      flag = 1;
      tmr = millis();
    }
    if (btn.isHold()) {
      return;
    }
#endif
    yield();
  }
  if (flag) {
    EEPROM.put(0, cfg);
    EEPROM.commit();
  }
  DEBUG("group: ");
  DEBUGLN(cfg.group);
  DEBUG("role: ");
  DEBUGLN(cfg.role);
}

void startStrip() {
  FastLED.addLeds<STRIP_CHIP, STRIP_PIN, STRIP_COLOR>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, 500);
  FastLED.setBrightness(50);
  FastLED.show();
}

void showRGB() {
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  FastLED.show();
  FastLED.clear();
  delay(1500);
}

void startWiFi() {
  if (!cfg.WiFimode) setupAP();   // режим точки доступа
  else setupLocal();              // подключаемся к точке

  DEBUG("UDP port: ");
  DEBUGLN(8888);
  Udp.begin(8888);
  FastLED.clear();
  FastLED.show();
}

void setupAP() {
  blink8(CRGB::Yellow);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP(AP_NameChar, WiFiPassword);
  server.begin();
  DEBUGLN("Setting AP Mode");
  DEBUG("AP IP: ");
  DEBUGLN(WiFi.softAPIP());
  delay(500);
}

void setupLocal() {
  if (cfg.ssid[0] == NULL && cfg.pass[0] == NULL) {
    DEBUGLN("WiFi not configured");
    setupAP();
  } else {
    DEBUGLN("Connecting to AP...");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    uint32_t tmr = millis();
    bool connect = false;
    int8_t count = 0, dir = 1;
    byte failCount = 0;
    while (1) {
      WiFi.begin(cfg.ssid, cfg.pass);
      while (millis() - tmr < 10000) {
        if (WiFi.status() == WL_CONNECTED) {
          connect = true;
          break;
        }
        FastLED.clear();
        leds[count] = CRGB::Yellow;
        FastLED.show();
        count += dir;
        if (count >= 7 || count <= 0) dir *= -1;
        delay(50);
      }
      if (connect) {
        blink8(CRGB::Green);
        server.begin();
        DEBUG("Connected! Local IP: ");
        DEBUGLN(WiFi.localIP());
        delay(500);
        return;
      } else {
        DEBUGLN("Failed!");
        blink8(CRGB::Red);
        failCount++;
        tmr = millis();
        if (failCount >= 3) {
          setupAP();
          return;
          /*DEBUGLN("Reboot to AP!");
            cfg.WiFimode = 0;
            EE_updCfg();
            delay(100);
            ESP.restart();*/
        }
      }
    }
  }
}

void checkUpdate() {
  if (cfg.update) {
    if (cfg.version != GL_VERSION) {
      cfg.version = GL_VERSION;
      blink8(CRGB::Cyan);
      DEBUG("Update to");
      DEBUGLN(GL_VERSION);
    } else {
      blink8(CRGB::Blue);
      DEBUG("Update to current");
    }
    cfg.update = 0;
  }
}
