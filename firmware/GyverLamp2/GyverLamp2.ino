/*
  Версия 0.19b
  Минимальная версия приложения 1.17!!!
  Почищен мусор, оптимизация, повышена стабильность и производительность
  Мигает теперь 16 светиков
  Снова переделана сетевая политика, упрощён и сильно ускорен парсинг
  Изменены пределы по светодиодам, что сильно увеличило производительность
  Выключенная (программно) лампа не принимает сервисные команды кроме команды включиться
  Добавлены часы, в том числе в рассвет
  Slave работает со светомузыкой сам, если не получает данные с мастера

  Версия 0.18b
  Уменьшена чувствительность хлопков
  Увеличена плавность светомузыки
  Переделана сетевая политика
  Микрофон и датчик света опрашивает только мастер и отсылает данные слейвам своей группы
  4 клика - включить первый режим
  Отправка точного времени на лампу в режиме АР для работы рассвета и синхронизации эффектов

  Версия 0.17b
  Автосмена отключается 30 сек во время настройки режимов
  Убрана кнопка upload в режимах
  Лампа чуть мигает при получении данных
  Кастом палитра работает на огне 2020
  Вкл выкл двумя хлопками
  Плавное выключение
  Починил рассвет

  Версия 0.16b
  Исправлен масштаб огня 2020
  Фикс невыключения рассвета

  Версия 0.14b
  Мелкие баги
  Вернул искры огню
  Добавлены палитры
  Добавлен огонь 2020

  Версия 0.13b
  Улучшена стабильность

  Версия 0.12b
  Мелкие исправления

  Версия 0.11b
  Добавлен редактор палитр
  Исправлены мелкие баги в эффектах
  Переподключение к роутеру после сброса сети
  Настройка ориентации матрицы из приложения
  Переработан эффект "Частицы"
  Добавлена скорость огня
  Переключение на новый/выбранный режим при редактировании
  Отправка времени из сервиса (для АР)
  Выключение по таймеру теперь плавное
  Добавлен рассвет

  TODO:
  плавная смена режимов
  Mqtt?
  Базовый пак
  Эффект погода https://it4it.club/topic/40-esp8266-i-parsing-pogodyi-s-openweathermap/
*/

// ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ! ВНИМАНИЕ!
// ДЛЯ КОМПИЛЯЦИИ ПРОШИВКИ ПОД NODEMCU/WEMOS/ESP01/ESP12 ВЫБИРАТЬ
// Инструменты/Плата Generic ESP8266
// Инструменты/Flash Size 4MB (FS:2MB OTA)
// При прошивке с других прошивок лампы поставить: Инструменты/Erase Flash/All Flash Contents
// ESP core 2.7.4+ http://arduino.esp8266.com/stable/package_esp8266com_index.json
// FastLED 3.4.0+ https://github.com/FastLED/FastLED/releases

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
#define STRIP_PIN 2         // пин ленты GPIO2 (D4 на wemos/node)
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
#define GL_VERSION 19       // код версии прошивки
#define EE_TOUT 30000       // таймаут сохранения епром после изменения, мс
#define DEBUG_SERIAL        // закомментируй чтобы выключить отладку (скорость 115200)
#define EE_KEY 55           // ключ сброса WiFi (измени для сброса всех настроек)
#define NTP_UPD_PRD 5       // период обновления времени с NTP сервера, минут
//#define SKIP_WIFI         // пропустить подключение к вафле (для отладки)

// ------------ БИЛДЕР -------------
//#define MAX_LEDS 900

// esp01
//#define BTN_PIN 0
//#define STRIP_PIN 2
//#define USE_ADC 0

// GL2 module
//#define STRIP_PIN 5     // GPIO5 на gl module (D1 на wemos/node)

// ---------- БИБЛИОТЕКИ -----------
#define FASTLED_ALLOW_INTERRUPTS 0
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
IPAddress deviceIP;
NTPClient ntp(ntpUDP);
CRGB leds[MAX_LEDS];
Time now;
Button btn(BTN_PIN);
timerMillis EEtmr(EE_TOUT), turnoffTmr, connTmr(120000ul), dawnTmr, holdPresTmr(30000ul), blinkTmr(300);
timerMillis effTmr(30, true);
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
#ifdef DEBUG_SERIAL
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
}
