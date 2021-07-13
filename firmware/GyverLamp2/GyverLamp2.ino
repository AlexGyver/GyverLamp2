/*
  ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ!
  ДЛЯ КОМПИЛЯЦИИ ПРОШИВКИ ПОД NODEMCU/WEMOS/ESP01/ESP12 ВЫБИРАТЬ
  Инструменты / Плата Generic ESP8266
  Инструменты / Flash Size 4MB (FS:2MB OTA)
  CPU Frequency / 160 MHz (рекомендуется для стабильности светомузыки!!!)
  При прошивке с других прошивок лампы поставить: Инструменты/Erase Flash/All Flash Contents
  ESP core 2.7.4+ http://arduino.esp8266.com/stable/package_esp8266com_index.json
  FastLED 3.4.0+ https://github.com/FastLED/FastLED/releases
*/

/*
  Версия 0.23b
  Поправлена яркость рассвета
  Компилится на версии ядра esp v3

  TODO:
  Upload -> Применить
  Длина огня в светомуз?
  Плавная смена режимов
  Mqtt
  Базовый пак
  Поддержка куба
  Погода https://it4it.club/topic/40-esp8266-i-parsing-pogodyi-s-openweathermap/
*/

// ---------- Настройки -----------
#define GL_KEY "GL"         // ключ сети

// ------------ Кнопка -------------
#define BTN_PIN 4           // пин кнопки GPIO4 (D2 на wemos/node), 0 для схемы с ESP-01
#define USE_BTN 1           // 1 использовать кнопку, 0 нет

// ------------- АЦП --------------
#define USE_ADC 1           // можно выпилить АЦП
#define USE_CLAP 1          // два хлопка в ладоши вкл выкл лампу
#define MIC_VCC 12          // питание микрофона GPIO12 (D6 на wemos/node)
#define PHOT_VCC 14         // питание фоторезистора GPIO14 (D5 на wemos/node)

// ------------ Лента -------------
#define STRIP_PIN 2         // пин ленты GPIO2 (D4 на wemos/node), GPIO5 (D1) для module
#define MAX_LEDS 300        // макс. светодиодов
#define STRIP_CHIP WS2812   // чип ленты
#define STRIP_COLOR GRB     // порядок цветов в ленте
#define STRIP_VOLT 5        // напряжение ленты, V
/*
  WS2811, GBR, 12V
  WS2812, GRB, 5V
  WS2813, GRB, 5V
  WS2815, GRB, 12V
  WS2818, RGB, 12V
*/

// ------------ WiFi AP ------------
const char AP_NameChar[] = "GyverLamp2";
const char WiFiPassword[] = "12345678";

// ------------ Прочее -------------
#define GL_VERSION 23       // код версии прошивки
#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
#define DEBUG_SERIAL_LAMP   // закомментируй чтобы выключить отладку (скорость 115200)
#define EE_KEY 56           // ключ сброса eeprom
#define NTP_UPD_PRD 5       // период обновления времени с NTP сервера, минут
//#define SKIP_WIFI         // пропустить подключение к вафле (для отладки)

// ------------ БИЛДЕР -------------
#define GL_BUILD 0  // 0: com 300, 1: com 900, 2: esp1 300, 3: esp1 900, 4: module 300, 5: module 900

#if (GL_BUILD == 0)
#elif (GL_BUILD == 1)
#define MAX_LEDS 900
#elif (GL_BUILD == 2)
#define MAX_LEDS 300
#define BTN_PIN 0
#define STRIP_PIN 2
#define USE_ADC 0
#elif (GL_BUILD == 3)
#define MAX_LEDS 900
#define BTN_PIN 0
#define STRIP_PIN 2
#define USE_ADC 0
#elif (GL_BUILD == 4)
#define MAX_LEDS 300
#define STRIP_PIN 5
#elif (GL_BUILD == 5)
#define MAX_LEDS 900
#define STRIP_PIN 5
#endif

// ---------- БИБЛИОТЕКИ -----------
//#define FASTLED_ALLOW_INTERRUPTS 0
#include "data.h"         // данные
#include "Time.h"         // часы
#include "timeRandom.h"   // случайные числа по времени
//#include "fastRandom.h"   // быстрый рандом
#include "Button.h"       // библа кнопки
#include "palettes.h"     // палитры
#include "NTPClient-Gyver.h"  // сервер времени (модиф)
#include "timerMillis.h"  // таймер миллис
#include "VolAnalyzer.h"  // анализатор громкости
#include "FFT_C.h"        // фурье
#include <FastLED.h>      // лента
#include <ESP8266WiFi.h>  // базовая либа есп
#include <WiFiUdp.h>      // общение по UDP
#include <EEPROM.h>       // епром
#include "ESP8266httpUpdate.h"  // OTA
#include "mString.h"      // стринг билдер
#include "Clap.h"         // обработка хлопков

// ------------------- ДАТА --------------------
Config cfg;
Preset preset[MAX_PRESETS];
Dawn dawn;
Palette pal;
WiFiServer server(80);
WiFiUDP Udp;
WiFiUDP ntpUDP;
IPAddress broadIP;
NTPClient ntp(ntpUDP);
CRGB leds[MAX_LEDS];
Time now;
Button btn(BTN_PIN);
timerMillis EEtmr(EE_TOUT), turnoffTmr, connTmr(120000ul), dawnTmr, holdPresTmr(30000ul), blinkTmr(300);
timerMillis effTmr(30, true), onlineTmr(500, true), postDawn(10 * 60000ul);
TimeRandom trnd;
VolAnalyzer vol(A0), low, high;
FastFilter phot;
Clap clap;

uint16_t portNum;
uint32_t udpTmr = 0, gotADCtmr = 0;
byte btnClicks = 0, brTicks = 0;
unsigned char matrixValue[11][16];
bool gotNTP = false, gotTime = false;
bool loading = true;
int udpLength = 0;
byte udpScale = 0, udpBright = 0;

// ------------------- SETUP --------------------
void setup() {
  misc();
  delay(2000);          // ждём старта есп
#ifdef DEBUG_SERIAL_LAMP
  Serial.begin(115200);
  DEBUGLN();
#endif
  startStrip();         // старт ленты
  btn.setLevel(digitalRead(BTN_PIN));   // смотрим что за кнопка
  EE_startup();         // читаем епром
#ifndef SKIP_WIFI
  checkUpdate();        // индикация было ли обновление
  showRGB();            // показываем ргб
  checkGroup();         // показываем или меняем адрес
  checkButton();        // проверяем кнопку на удержание
  startWiFi();          // старт вайфай
  setupTime();          // выставляем время
#endif
  setupADC();           // настраиваем анализ
  presetRotation(true); // форсировать смену режима
}

void loop() {
  timeTicker();       // обновляем время
  yield();
#ifndef SKIP_WIFI
  tryReconnect();     // пробуем переподключиться если WiFi упал
  yield();
  parsing();          // ловим данные
  yield();
#endif
  checkEEupdate();    // сохраняем епром
  presetRotation(0);  // смена режимов по расписанию
  effectsRoutine();   // мигаем
  yield();
  button();           // проверяем кнопку
  checkAnalog();      // чтение звука и датчика
  yield();
  iAmOnline();
}
