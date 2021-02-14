// 0.10
// исправлена обработка ключа
// добавлена совместимость с nodemcu
// поворот матрицы
// обновление прошивок для разных схем
// исправлен цвет огня
// индикация обновления при запуске

// мигает 8:
// красным - не смог подключиться к АР
// зелёным - смог подключиться к АР
// жёлтым - создал свою АП
// бирюзовым - успешно обновился на новую версию
// синим - обновился на ту же версию
// розовым - сброс всех настроек (первый запуск)

// Generic ESP8266, 4MB (FS:2MB OTA)
// ESP core 2.7.4+ http://arduino.esp8266.com/stable/package_esp8266com_index.json
// FastLED 3.4.0+ https://github.com/FastLED/FastLED/releases

// ---------- Настройки -----------
#define GL_KEY "GL"         // ключ сети

// ------------ Кнопка -------------
#define BTN_PIN 4           // пин кнопки GPIO4 (D2 на wemos/node), 0 для схемы с ESP-01
#define USE_BTN 1           // 1 использовать кнопку, 0 нет

// ------------- АЦП --------------
#define USE_ADC 1           // можно выпилить АЦП
#define MIC_VCC 12          // питание микрофона GPIO12 (D6 на wemos/node)
#define PHOT_VCC 14         // питание фоторезистора GPIO14 (D5 на wemos/node)

// ------------ Лента -------------
#define STRIP_PIN 2         // пин ленты GPIO2 (D4 на wemos/node)
#define MAX_LEDS 600        // макс. светодиодов
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
#define GL_VERSION 010
#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
#define DEBUG_SERIAL        // закомментируй чтобы выключить отладку (скорость 115200)
#define EE_KEY 44           // ключ сброса WiFi (измени для сброса всех настроек)
#define NTP_UPD_PRD 5       // период обновления времени с NTP сервера, минут

// ------------ БИЛДЕР -------------
//#define MAX_LEDS 1200

// esp01
//#define BTN_PIN 0
//#define STRIP_PIN 2
//#define USE_ADC 0

// GL2 module
//#define STRIP_PIN 5     // GPIO5 на gl module (D1 на wemos/node)

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
void blink8(CRGB color);

// ------------------- SETUP --------------------
void setup() {
  delay(800);
  memset(matrixValue, 0, sizeof(matrixValue));
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  DEBUGLN();
#endif
  EEPROM.begin(512);    // старт епром
  startStrip();         // старт ленты  
  btn.setLevel(digitalRead(BTN_PIN));   // смотрим что за кнопка
  EE_startup();         // читаем епром
  checkUpdate();        // индикация было ли обновление
  showRGB();            // показываем ргб  
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
