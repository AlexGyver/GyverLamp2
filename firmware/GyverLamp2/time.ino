void setupTime() {
  ntp.setUpdateInterval(NTP_UPD_PRD / 2 * 60000ul);   // меньше в два раза, ибо апдейт вручную
  ntp.setTimeOffset((cfg.GMT - 13) * 3600);
  ntp.setPoolServerName(NTPserver);
  if (cfg.WiFimode) {
    // если подключены - запрашиваем время с сервера
    ntp.begin();
    if (ntp.update() && !gotNTP) {
      gotNTP = true;
      DEBUGLN("Got ntp");
    }
  }
}

// сохраняет счёт времени после обрыва связи
void timeTicker() {
  static timerMillis tmr(10, true);
  if (tmr.isReady()) {
    updateTime();                               // обновляем время
    sendTimeToSlaves();                         // отправляем время слейвам
    trnd.update(now.hour, now.min, now.sec);    // обновляем рандомайзер
    if (gotNTP || gotTime) checkWorkTime();     // проверяем расписание, если знаем время
  }
}

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

void updateTime() {
  if (cfg.WiFimode && WiFi.status() == WL_CONNECTED) {  // если вайфай подключен
    now.sec = ntp.getSeconds();
    now.min = ntp.getMinutes();
    now.hour = ntp.getHours();
    now.day = ntp.getDay();   // вс 0, сб 6
    now.weekMs = now.getWeekS() * 1000ul + ntp.getMillis();
    now.setMs(ntp.getMillis());
    if (now.min % NTP_UPD_PRD == 0 && now.sec == 0) {
      // берём время с интернета каждую NTP_UPD_PRD минуту, ставим флаг что данные с NTP получены, значит мы онлайн
      if (ntp.update() && !gotNTP) gotNTP = true;
    }    
  } else {          // если нет
    now.tick();     // тикаем своим счётчиком
  }
  if (gotNTP || gotTime) checkDawn();   // рассвет, если знаем точное время
}

void sendTimeToSlaves() {
  if (!cfg.WiFimode) {  // если мы AP
    static byte prevSec = 0;
    if (prevSec != now.sec) {       // новая секунда
      prevSec = now.sec;
      if (now.min % 5 == 0 && now.sec == 0) sendTime(); // ровно каждые 5 мин отправляем время
    }
  }
}

void checkDawn() {
  if (now.sec == 0 && dawn.state[now.day] && !dawnTmr.running()) {    // рассвет включен но не запущен
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

void sendTime() {  
  char reply[25];
  mString packet(reply, sizeof(reply));
  packet.clear();
  packet = packet + GL_KEY + ",0," + now.day + ',' + now.hour + ',' + now.min + ',' + now.sec;

  DEBUG("Sending time: ");
  DEBUGLN(reply);
  sendUDP(reply);
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
