// LOLIN(WEMOS) D1 R2 & mini
// ESP core 2.7.4+ http://arduino.esp8266.com/stable/package_esp8266com_index.json
// FastLED 3.4.0+ https://github.com/FastLED/FastLED/releases

// ---------- Настройки -----------
#define GL_KEY "GL"         // ключ сети

// ------------ Кнопка -------------
#define BTN_PIN 4           // пин кнопки GPIO4 (D2). Или 0 для схемы с ESP-01 !!
#define USE_BTN 1           // 1 использовать кнопку, 0 нет

// ------------ Лента -------------
#define STRIP_PIN 2         // пин ленты GPIO2 (D4)
#define MAX_LEDS 512        // макс. светодиодов
#define STRIP_CHIP WS2812   // чип ленты
#define STRIP_COLOR GRB     // порядок цветов в ленте
#define STRIP_VOLT 5        // напряжение ленты, V
#define CONNECTION_ANGLE 0  // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 0   // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
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
#define MIC_VCC D6          // питание микрофона
#define PHOT_VCC D5         // питание фоторезистора
#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
//#define DEBUG_SERIAL        // закомментируй чтобы выключить отладку (скорость 115200)
#define EE_KEY 42           // ключ сброса WiFi (измени для сброса всех настроек)
#define NTP_UPD_PRD 5       // период обновления времени с NTP сервера, минут

// ---------- БИБЛИОТЕКИ -----------
#include "data.h"         // данные
#include "Time.h"         // часы
#include "TimeRandom.h"   // случайные числа по времени
#include "FastRandom.h"   // быстрый рандом
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

// ------------------- ДАТА --------------------
Config cfg;
Preset preset[MAX_PRESETS];
Dawn dawn;
WiFiServer server(80);
WiFiUDP Udp;
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);
CRGB leds[MAX_LEDS];
Time now;
Button btn(BTN_PIN);
timerMillis EEtmr(EE_TOUT), turnoffTmr;
TimeRandom trnd;
VolAnalyzer vol(A0), low, high;
FastFilter phot;

byte btnClicks = 0, brTicks = 0;
unsigned char matrixValue[11][16];
bool gotNTP = false;

// ------------------- SETUP --------------------
void setup() {
  memset(matrixValue, 0, sizeof(matrixValue));
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  DEBUGLN();
#endif
  EEPROM.begin(512);    // старт епром
  startStrip();         // показываем РГБ
  btn.setLevel(digitalRead(BTN_PIN));   // смотрим что за кнопка
  EE_startup();         // читаем епром
  checkGroup();         // показываем или меняем адрес
  checkButton();        // проверяем кнопку на удержание
  startWiFi();          // старт вайфай
  setupTime();          // выставляем время
  setupADC();           // настраиваем анализ
  presetRotation(true); // форсировать смену режима
}

void loop() {
  timeTicker();       // обновляем время
  yield();
  parsing();          // ловим данные
  yield();
  checkEEupdate();    // сохраняем епром
  presetRotation(0);  // смена режимов по расписанию
  effectsRoutine();   // мигаем
  yield();
  button();           // проверяем кнопку
  checkAnalog();      // чтение звука и датчика
}
