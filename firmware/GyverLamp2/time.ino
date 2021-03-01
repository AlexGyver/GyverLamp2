void setupTime() {
  ntp.setUpdateInterval(NTP_UPD_PRD * 60000ul);
  ntp.setTimeOffset((cfg.GMT - 13) * 3600);
  ntp.setPoolServerName(NTPserver);
  if (cfg.WiFimode && !connTmr.running()) {     // если успешно подключились к WiFi
    ntp.begin();
    if (ntp.update()) gotNTP = true;
  }
}

// основной тикер времени
void timeTicker() {
  static timerMillis tmr(30, true);
  if (tmr.isReady()) {
    if (cfg.WiFimode && WiFi.status() == WL_CONNECTED && !connTmr.running()) {  // если вайфай подключен и это не попытка переподключиться
      now.sec = ntp.getSeconds();
      now.min = ntp.getMinutes();
      now.hour = ntp.getHours();
      now.day = ntp.getDay();   // вс 0, сб 6
      now.weekMs = now.getWeekS() * 1000ul + ntp.getMillis();
      now.setMs(ntp.getMillis());
      if (ntp.update()) gotNTP = true;
    } else {          // если вайфай не подключен
      now.tick();     // тикаем своим счётчиком
    }

    static byte prevSec = 0;
    if (prevSec != now.sec) {                   // новая секунда
      prevSec = now.sec;
      trnd.update(now.hour, now.min, now.sec);  // обновляем рандомайзер

      if (now.sec == 0) {                       // новая минута
        if (now.min % 5 == 0) sendTimeToLocals();  // отправляем время каждые 5 мин
        if (gotNTP || gotTime) {                // если знаем точное время
          checkWorkTime();                      // проверяем расписание
          checkDawn();                          // и рассвет
        }
      }
    }
  }
}

void sendTimeToLocals() {
  if (!cfg.WiFimode) sendUDP(6, now.day, now.hour, now.min);   // мы - АР
}

// установка времени с мобилы
void setTime(byte day, byte hour, byte min, byte sec) {
  if (!cfg.WiFimode || !gotNTP) {  // если мы AP или не получили NTP
    now.day = day;
    now.hour = hour;
    now.min = min;
    now.sec = sec;
    now.setMs(0);
    gotTime = true;
  }
}

void checkDawn() {
  if (dawn.state[now.day] && !dawnTmr.running()) {    // рассвет включен но не запущен
    int dawnMinute = dawn.hour[now.day] * 60 + dawn.minute[now.day] - dawn.time;
    if (dawnMinute < 0) dawnMinute += 1440;
    if (dawnMinute == now.hour * 60 + now.min) {
      DEBUG("dawn start ");
      DEBUGLN(dawn.time * 60000ul);
      dawnTmr.setInterval(dawn.time * 60000ul);
      dawnTmr.restart();
      FastLED.setBrightness(255);
    }
  }
}

void checkWorkTime() {
  static byte prevState = 2;  // для первого запуска
  byte curState = isWorkTime(now.hour, cfg.workFrom, cfg.workTo);
  if (prevState != curState) {    // переключение расписания
    prevState = curState;
    if (curState && !cfg.state && !cfg.manualOff) fade(1);  // нужно включить, а лампа выключена и не выключалась вручную
    if (!curState && cfg.state) fade(0);                    // нужно выключить, а лампа включена
  }
}

bool isWorkTime(byte t, byte from, byte to) {
  if (from == to) return 1;
  else if (from < to) {
    if (t >= from && t < to) return 1;
    else return 0;
  } else {
    if (t >= from || t < to) return 1;
    else return 0;
  }
}
