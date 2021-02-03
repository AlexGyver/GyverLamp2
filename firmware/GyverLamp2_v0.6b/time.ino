void setupTime() {
  ntp.setUpdateInterval(NTP_UPD_PRD / 2 * 60000ul);   // меньше в два раза, ибо апдейт вручную
  ntp.setTimeOffset((cfg.GMT - 13) * 3600);
  ntp.setPoolServerName(NTPservers[cfg.NTP - 1]);
  if (cfg.WiFimode) {
    // если подключены - запрашиваем время с сервера
    ntp.begin();
    if (ntp.update() && !gotNTP) gotNTP = true;
  }
}

// сохраняет счёт времени после обрыва связи
void timeTicker() {
  static timerMillis tmr(10, true);
  if (tmr.isReady()) {
    updateTime();                               // обновляем время
    sendTimeToSlaves();                         // отправляем время слейвам
    trnd.update(now.hour, now.min, now.sec);    // обновляем рандомайзер
    if (gotNTP) checkWorkTime();                // проверяем расписание, если подключены к Интернет
    checkTurnoff();                             // проверяем таймер отключения
  }
}

void updateTime() {
  if (cfg.WiFimode && WiFi.status() == WL_CONNECTED) {  // если вайфай подключен
    now.sec = ntp.getSeconds();
    now.min = ntp.getMinutes();
    now.hour = ntp.getHours();
    now.day = ntp.getDay();
    now.day = (now.day == 0) ? 6 : (now.day - 1);   // перевод из вс0 в пн0
    now.weekMs = now.getWeekS() * 1000ul + ntp.getMillis();
    now.setMs(ntp.getMillis());
    if (now.min % NTP_UPD_PRD == 0 && now.sec == 0) {
      // берём время с интернета каждую NTP_UPD_PRD минуту, ставим флаг что данные с NTP получены, значит мы онлайн
      if (ntp.update() && !gotNTP) gotNTP = true;
    }
  } else {          // если нет
    now.tick();     // тикаем своим счётчиком
  }
}

void sendTimeToSlaves() {
  if (!cfg.WiFimode) {              // если мы AP
    static byte prevSec = 0;
    if (prevSec != now.sec) {       // новая секунда
      prevSec = now.sec;
      if (now.min % 1 == 0 && now.sec == 0) sendTime(); // ровно каждые 5 мин отправляем время
    }
  }
}

void checkTurnoff() {
  if (turnoffTmr.isReady()) {
    turnoffTmr.stop();
    setPower(0);
  }
}

void checkWorkTime() {
  if (!isWorkTime(now.hour, cfg.workFrom, cfg.workTo)) {
    if (cfg.state) {
      cfg.state = false;
      FastLED.clear();
      FastLED.show();
    }
  } else {
    if (!cfg.state && !cfg.manualOff) {
      cfg.state = true;
    }
  }
}

void sendTime() {
  IPAddress ip = WiFi.localIP();
  ip[3] = 255;
  char reply[20] = GL_KEY;
  byte keylen = strlen(GL_KEY);
  reply[keylen++] = ',';
  reply[keylen++] = 0 + '0';
  reply[keylen++] = ',';
  char hours[4];
  itoa(now.hour, hours, DEC);
  strncpy(reply + keylen, hours, 3);
  keylen += strlen(hours);
  reply[keylen++] = ',';
  char mins[4];
  itoa(now.min, mins, DEC);
  strncpy(reply + keylen, mins, 3);
  keylen += strlen(mins);
  reply[keylen++] = NULL;

  DEBUG("Sending time: ");
  DEBUGLN(reply);
  Udp.beginPacket(ip, 8888);
  Udp.write(reply);
  Udp.endPacket();
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
