void parsing() {
  if (Udp.parsePacket()) {
    static uint32_t tmr = 0;
    static char buf[UDP_TX_PACKET_MAX_SIZE + 1];

    int n = Udp.read(buf, UDP_TX_PACKET_MAX_SIZE);
    if (millis() - tmr < 500) return;  // принимаем посылки не чаще 2 раз в секунду
    tmr = millis();

    buf[n] = NULL;
    DEBUGLN(buf);   // пакет вида <ключ>,<канал>,<тип>,<дата1>,<дата2>...

    byte keyLen = strchr(buf, ',') - buf;     // indexof
    if (strncmp(buf, GL_KEY, keyLen)) return; // не наш ключ

    byte data[MAX_PRESETS * PRES_SIZE + keyLen];
    memset(data, 0, MAX_PRESETS * PRES_SIZE + keyLen);
    int count = 0;
    char *str, *p = buf + keyLen;  // сдвиг до даты
    char *ssid, *pass;
    while ((str = strtok_r(p, ",", &p)) != NULL) {
      data[count++] = atoi(str);
      if (count == 4) ssid = str;
      if (count == 5) pass = str;
    }

    // широковещательный запрос времени для local устройств в сети AP лампы
    if (data[0] == 0 && cfg.WiFimode && !gotNTP) {
      now.hour = data[1];
      now.min = data[2];
      now.setMs(0);
    }

    if (data[0] != cfg.group) return;     // не наш адрес, выходим

    switch (data[1]) {  // тип 0 - control, 1 - config, 2 - effects, 3 - dawn
      case 0: DEBUGLN("Control");
        switch (data[2]) {
          case 0: setPower(0); break;                     // выкл
          case 1: setPower(1); break;                     // вкл
          case 2: cfg.minLight = phot.getRaw(); break;    // мин яркость
          case 3: cfg.maxLight = phot.getRaw(); break;    // макс яркость
          case 4: changePreset(-1); break;                // пред пресет
          case 5: changePreset(1); break;                 // след пресет
          case 6: setPreset(data[3] - 1); break;          // конкретный пресет data[3]
          case 7: cfg.WiFimode = data[3]; EE_updCfgRst(); break;  // смена режима WiFi
          case 8: cfg.role = data[3]; break;              // смена роли
          case 9: cfg.group = data[3]; break;             // смена группы
          case 10:                                        // установка настроек WiFi
            strcpy(cfg.ssid, ssid);
            strcpy(cfg.pass, pass);
            break;
          case 11: EE_updCfgRst(); break;                 // рестарт
          case 12: if (gotNTP) {                          // OTA обновление, если есть интернет
              cfg.update = 1;
              EE_updCfg();
              delay(100);
              FastLED.clear();
              FastLED.show();
              char OTA[60];
              strcpy(OTA, OTAhost);
              strcpy(OTA + strlen(OTAhost), OTAfile[data[3]]);
              ESPhttpUpdate.update(OTA);
            } break;
          case 13:                                        // выключить через
            if (data[3] == 0) turnoffTmr.stop();
            else {
              turnoffTmr.setInterval((uint32_t)data[3] * 60000ul);
              turnoffTmr.restart();
            }
            break;
        }
        EE_updCfg();
        break;

      case 1: DEBUGLN("Config");
        FOR_i(0, CFG_SIZE) {
          *((byte*)&cfg + i) = data[i + 2];   // загоняем в структуру
        }
        cfg.mTurn = data[21];
        cfg.length = data[17] | (data[16] << 8);  // склеиваем
        cfg.width = data[20] | (data[19] << 8);   // склеиваем

        if (cfg.length > MAX_LEDS) cfg.length = MAX_LEDS;
        if (cfg.deviceType == GL_TYPE_STRIP) cfg.width = 1;
        if (cfg.length * cfg.width > MAX_LEDS) cfg.width = MAX_LEDS / cfg.length;
        ntp.setTimeOffset((cfg.GMT - 13) * 3600);
        ntp.setPoolServerName(NTPservers[cfg.NTP - 1]);
        FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, cfg.maxCur * 100);
        if (cfg.adcMode == GL_ADC_BRI) switchToPhot();
        else if (cfg.adcMode == GL_ADC_MIC) switchToMic();
        else disableADC();
        EE_updCfg();
        break;

      case 2: DEBUGLN("Preset");
        cfg.presetAmount = data[2];   // кол-во режимов
        FOR_j(0, cfg.presetAmount) {
          FOR_i(0, PRES_SIZE) {
            *((byte*)&preset + j * PRES_SIZE + i) = data[j * PRES_SIZE + i + 3]; // загоняем в структуру
          }
        }
        EE_updatePreset();
        presetRotation(true); // форсировать смену режима
        break;

      case 3: DEBUGLN("Dawn");
        FOR_i(0, (2 + 3 * 7)) {
          *((byte*)&dawn + i) = data[i + 2]; // загоняем в структуру
        }
        EE_updateDawn();
        break;

      case 4: DEBUGLN("From master");
        if (cfg.role == GL_SLAVE) {
          switch (data[2]) {
            case 0: setPower(data[3]); break;     // вкл выкл
            case 1: setPreset(data[3]); break;    // пресет
            case 2: cfg.bright = data[3]; break;  // яркость
          }
          EE_updateCfg();
        }
        break;
    }
    FastLED.clear();    // на всякий случай

  }
}

void sendToSlaves(byte data1, byte data2) {
  if (cfg.role == GL_MASTER) {
    IPAddress ip = WiFi.localIP();
    ip[3] = 255;
    char reply[20] = GL_KEY;
    byte keylen = strlen(GL_KEY);
    reply[keylen++] = ',';
    reply[keylen++] = cfg.group + '0';
    reply[keylen++] = ',';
    reply[keylen++] = '4';
    reply[keylen++] = ',';
    reply[keylen++] = data1 + '0';
    reply[keylen++] = ',';
    itoa(data2, reply + (keylen++), DEC);

    DEBUG("Sending: ");
    DEBUGLN(reply);

    FOR_i(0, 3) {
      Udp.beginPacket(ip, 8888);
      Udp.write(reply);
      Udp.endPacket();
      delay(10);
    }
  }
}
