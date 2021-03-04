void effectsRoutine() {
  static byte prevEff = 255;
  if (!effTmr.isReady()) return;

  if (dawnTmr.running()) {
    fill_solid(leds, MAX_LEDS, ColorFromPalette(HeatColors_p, dawnTmr.getLength8(), scaleFF(dawnTmr.getLength8(), dawn.bright), LINEARBLEND));
    drawClock(cfg.length / 2 - 4, 100, 0);
    FastLED.show();
    if (dawnTmr.isReady()) {
      dawnTmr.stop();
      FastLED.clear();
      FastLED.show();
    }
    return;
  }

  if (!cfg.state) return;
  int thisLength = getLength();
  byte thisScale = getScale();
  byte thisBright = getBright();

  if (musicMode() || briMode()) {    // музыка или яркость
    if (cfg.role) {         // мастер отправляет
      static uint32_t tmr = 0;
      if ((millis() - tmr >= musicMode() ? 60 : 1000) && millis() - udpTmr >= 1000) {
        sendUDP(7, thisLength, thisScale, thisBright);
        tmr = millis();
      }
    } else {                // слейв получает
      if (millis() - gotADCtmr < 4000) {     // есть сигнал с мастера
        thisLength = udpLength;
        thisScale = udpScale;
        thisBright = udpBright;
      }
    }
  }

  if (turnoffTmr.running()) thisBright = scaleFF(thisBright, 255 - turnoffTmr.getLength8());
  else if (blinkTmr.runningStop()) thisBright = scaleFF(thisBright, blinkTmr.getLength8());
  if (turnoffTmr.isReady()) {
    turnoffTmr.stop();
    setPower(0);
    return;
  }
  FastLED.setBrightness(thisBright);

  if (prevEff != CUR_PRES.effect) {   // смена эффекта
    FastLED.clear();
    prevEff = CUR_PRES.effect;
    loading = true;
  }
  yield();

  // =================================================== ЭФФЕКТЫ ===================================================
  switch (CUR_PRES.effect) {
    case 1: // =================================== ПЕРЛИН ===================================
      if (cfg.deviceType > 1) {
        FOR_j(0, cfg.length) {
          FOR_i(0, cfg.width) {
            setPix(i, j, ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                          scalePal(inoise8(
                                              i * (thisScale / 5) - cfg.width * (thisScale / 5) / 2,
                                              j * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                              (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                          255, LINEARBLEND));
          }
        }

      } else {
        FOR_i(0, cfg.length) {
          leds[i] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                     scalePal(inoise8(i * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                              (now.weekMs >> 1) * CUR_PRES.speed / 255)),
                                     255, LINEARBLEND);
        }
      }
      break;

    case 2: // ==================================== ЦВЕТ ====================================
      {
        fill_solid(leds, cfg.length * cfg.width, CHSV(CUR_PRES.color, thisScale, 30));
        CRGB thisColor = CHSV(CUR_PRES.color, thisScale, thisBright);
        if (CUR_PRES.fromCenter) {
          fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
          fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
        } else {
          fillStrip(0, thisLength, thisColor);
        }
      }
      break;

    case 3: // ================================= СМЕНА ЦВЕТА =================================
      {
        CRGB thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), 10, LINEARBLEND);
        fill_solid(leds, cfg.length * cfg.width, thisColor);
        thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal((now.weekMs >> 5) * CUR_PRES.speed / 255), thisBright, LINEARBLEND);
        if (CUR_PRES.fromCenter) {
          fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
          fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
        } else {
          fillStrip(0, thisLength, thisColor);
        }
      }
      break;

    case 4: // ================================== ГРАДИЕНТ ==================================
      if (CUR_PRES.fromCenter) {
        FOR_i(cfg.length / 2, cfg.length) {
          byte bright = 255;
          if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < cfg.length / 2 + thisLength / 2) ? (thisBright) : (10);
          CRGB thisColor = ColorFromPalette(
                             paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                             scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                             bright, LINEARBLEND);
          if (cfg.deviceType > 1) fillRow(i, thisColor);
          else leds[i] = thisColor;
        }
        if (cfg.deviceType > 1) FOR_i(0, cfg.length / 2) fillRow(i, leds[(cfg.length - i)*cfg.width - 1]);
        else FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];

      } else {
        FOR_i(0, cfg.length) {
          byte bright = 255;
          if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < thisLength) ? (thisBright) : (10);
          CRGB thisColor = ColorFromPalette(
                             paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                             scalePal((i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128)),
                             bright, LINEARBLEND);
          if (cfg.deviceType > 1) fillRow(i, thisColor);
          else leds[i] = thisColor;
        }
      }
      break;

    case 5: // =================================== ЧАСТИЦЫ ===================================
      FOR_i(0, cfg.length * cfg.width) leds[i].fadeToBlackBy(70);
      {
        uint16_t rndVal = 0;
        byte amount = (thisScale >> 3) + 1;
        FOR_i(0, amount) {
          rndVal = rndVal * 2053 + 13849;     // random2053 алгоритм
          int homeX = inoise16(i * 100000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
          homeX = map(homeX, 15000, 50000, 0, cfg.length);
          int offsX = inoise8(i * 2500 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
          offsX = cfg.length / 2 * offsX / 128;
          int thisX = homeX + offsX;

          if (cfg.deviceType > 1) {
            int homeY = inoise16(i * 100000000ul + 2000000000ul + (now.weekMs << 3) * CUR_PRES.speed / 255);
            homeY = map(homeY, 15000, 50000, 0, cfg.width);
            int offsY = inoise8(i * 2500 + 30000 + (now.weekMs >> 1) * CUR_PRES.speed / 255) - 128;
            offsY = cfg.length / 2 * offsY / 128;
            int thisY = homeY + offsY;
            setPix(thisX, thisY, CUR_PRES.fromPal ?
                   ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                   CHSV(CUR_PRES.color, 255, 255)
                  );
          } else {
            setLED(thisX, CUR_PRES.fromPal ?
                   ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                   CHSV(CUR_PRES.color, 255, 255)
                  );
          }
        }
      }
      break;

    case 6: // ==================================== ОГОНЬ ====================================
      if (cfg.deviceType > 1) {         // 2D огонь
        fireRoutine(CUR_PRES.speed / 2);
      } else {                          // 1D огонь
        FastLED.clear();
        static byte heat[MAX_LEDS];
        CRGBPalette16 gPal;
        if (CUR_PRES.color < 5) gPal = HeatColors_p;
        else gPal = CRGBPalette16(CRGB::Black, CHSV(CUR_PRES.color, 255, 255), CRGB::White);
        if (CUR_PRES.fromCenter) thisLength /= 2;

        for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
        for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
        if (random8() < 120 ) {
          int y = random8(7);
          heat[y] = qadd8(heat[y], random8(160, 255));
        }
        if (CUR_PRES.fromCenter) {
          for (int j = 0; j < thisLength; j++) leds[cfg.length / 2 + j] = ColorFromPalette(gPal, scale8(heat[j], 240));
          FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
        } else {
          for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(gPal, scale8(heat[j], 240));
        }
      }
      break;

    case 7: // ==================================== ОГОНЬ 2020 ====================================
      FastLED.clear();
      if (cfg.deviceType > 1) {         // 2D огонь
        fire2020(CUR_PRES.scale, thisLength);
      } else {                          // 1D огонь
        static byte heat[MAX_LEDS];
        if (CUR_PRES.fromCenter) thisLength /= 2;

        for (int i = 0; i < thisLength; i++) heat[i] = qsub8(heat[i], random8(0, ((((255 - thisScale) / 2 + 20) * 10) / thisLength) + 2));
        for (int k = thisLength - 1; k >= 2; k--) heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
        if (random8() < 120 ) {
          int y = random8(7);
          heat[y] = qadd8(heat[y], random8(160, 255));
        }
        if (CUR_PRES.fromCenter) {
          for (int j = 0; j < thisLength; j++) leds[cfg.length / 2 + j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
          FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];
        } else {
          for (int j = 0; j < thisLength; j++) leds[j] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scale8(heat[j], 240));
        }
      }
      break;

    case 8: // ================================== КОНФЕТТИ ==================================
      {
        byte amount = (thisScale >> 3) + 1;
        FOR_i(0, amount) {
          int x = random(0, cfg.length * cfg.width);
          if (leds[x] == CRGB(0, 0, 0)) leds[x] = CUR_PRES.fromPal ?
                                                    ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(i * 255 / amount), 255, LINEARBLEND) :
                                                    CHSV(CUR_PRES.color, 255, 255);
        }
        FOR_i(0, cfg.length * cfg.width) {
          if (leds[i].r >= 10 || leds[i].g >= 10 || leds[i].b >= 10) leds[i].fadeToBlackBy(CUR_PRES.speed / 2 + 1);
          else leds[i] = 0;
        }
      }
      break;
    case 9: // =================================== СМЕРЧ ===================================
      FastLED.clear();
      FOR_k(0, (thisScale >> 5) + 1) {
        FOR_i(0, cfg.length) {
          //byte thisPos = inoise8(i * 10 - (now.weekMs >> 1) * CUR_PRES.speed / 255, k * 10000);
          byte thisPos = inoise8(i * 10 + k * 10000, (now.weekMs >> 1) * CUR_PRES.speed / 255);
          thisPos = map(thisPos, 50, 200, 0, cfg.width);
          byte scale = 3;
          FOR_j(0, scale) {
            CRGB color = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(j * 255 / scale), 255, LINEARBLEND);
            color.fadeToBlackBy(j * 255 / scale);
            if (j == 0) {
              setPixOverlap(thisPos, i, color);
            } else {
              setPixOverlap(thisPos - j, i, color);
              setPixOverlap(thisPos + j, i, color);
            }
          }
        }
      }
      break;

    case 10: // =================================== ЧАСЫ ===================================
      FastLED.clear();
      drawClock(mapFF(CUR_PRES.scale, 0, cfg.length - 7), (CUR_PRES.speed < 10) ? 0 : (255 - CUR_PRES.speed), CHSV(CUR_PRES.color, 255, 255));
      break;

    case 11: // ================================= ПОГОДА ==================================

      break;

  }

  if (CUR_PRES.advMode == GL_ADV_CLOCK && CUR_PRES.effect != 9) drawClock(mapFF(CUR_PRES.scale, 0, cfg.length - 7), 100, 0);
  // выводим нажатия кнопки
  if (btnClicks > 0) fill_solid(leds, btnClicks, CRGB::White);
  if (brTicks > 0) fill_solid(leds, brTicks, CRGB::Cyan);
  yield();
  FastLED.show();
}

// ====================================================================================================================
bool musicMode() {
  return ((cfg.adcMode == GL_ADC_MIC || cfg.adcMode == GL_ADC_BOTH) && (CUR_PRES.advMode > 1 && CUR_PRES.advMode <= 4));
}
bool briMode() {
  return (cfg.adcMode == GL_ADC_BRI || cfg.adcMode == GL_ADC_BOTH);
}

byte getBright() {
  int maxBr = cfg.bright;   // макс яркость из конфига
  byte fadeBr = 255;
  if (CUR_PRES.fadeBright) fadeBr = CUR_PRES.bright; // ограничен вручную

  if (briMode()) {    // ----> датчик света или оба
    maxBr = constrain(phot.getFil(), cfg.minLight, cfg.maxLight);
    if (cfg.minLight != cfg.maxLight)
      maxBr = map(maxBr, cfg.minLight, cfg.maxLight, cfg.minBright, cfg.maxBright);
  }
  if (musicMode() &&                          // светомузыка вкл
      CUR_PRES.soundReact == GL_REACT_BRI) {  // режим яркости
    fadeBr = mapFF(getSoundVol(), CUR_PRES.min, CUR_PRES.max);  // громкость в 0-255
  }
  return scaleFF(maxBr, fadeBr);
}

int getLength() {
  if (musicMode()                             // светомузыка вкл
      && CUR_PRES.soundReact == GL_REACT_LEN  // режим длины
     ) //return mapFF(getSoundVol(), 0, cfg.length);
    return mapFF(getSoundVol(), scaleFF(cfg.length, CUR_PRES.min), scaleFF(cfg.length, CUR_PRES.max));
  else return cfg.length;
}

byte getScale() {
  if (musicMode()                                                 // светомузыка вкл
      && CUR_PRES.soundReact == GL_REACT_SCL                      // режим масштаба
     ) return mapFF(getSoundVol(), CUR_PRES.min, CUR_PRES.max);
  else return CUR_PRES.scale;
}

void fillStrip(int from, int to, CRGB color) {
  if (cfg.deviceType > 1) {
    FOR_i(from, to) {
      FOR_j(0, cfg.width) leds[getPix(j, i)] = color;
    }
  } else {
    FOR_i(from, to) leds[i] = color;
  }
}

void fillRow(int row, CRGB color) {
  FOR_i(cfg.width * row, cfg.width * (row + 1)) leds[i] = color;
}

void updPal() {
  for (int i = 0; i < 16; i++) {
    paletteArr[0][i] = CRGB(pal.strip[i * 3], pal.strip[i * 3 + 1], pal.strip[i * 3 + 2]);
  }
  if (pal.size < 16) paletteArr[0][pal.size] = paletteArr[0][0];
}

byte scalePal(byte val) {
  if (CUR_PRES.palette == 1) val = val * pal.size / 16;
  return val;
}

void setPix(int x, int y, CRGB color) {
  if (y >= 0 && y < cfg.length && x >= 0 && x < cfg.width) leds[getPix(x, y)] = color;
}
void setPixOverlap(int x, int y, CRGB color) {
  if (y < 0) y += cfg.length;
  if (x < 0) x += cfg.width;
  if (y >= cfg.length) y -= cfg.length;
  if (x >= cfg.width) x -= cfg.width;
  setPix(x, y, color);
}
void setLED(int x, CRGB color) {
  if (x >= 0 && x < cfg.length) leds[x] = color;
}
uint32_t getPixColor(int x, int y) {
  int thisPix = getPix(x, y);
  if (thisPix < 0 || thisPix >= MAX_LEDS) return 0;
  return (((uint32_t)leds[thisPix].r << 16) | ((long)leds[thisPix].g << 8 ) | (long)leds[thisPix].b);
}

// получить номер пикселя в ленте по координатам
uint16_t getPix(int x, int y) {
  int matrixW;
  if (cfg.matrix == 2 || cfg.matrix == 4 || cfg.matrix == 6 || cfg.matrix == 8)  matrixW = cfg.length;
  else matrixW = cfg.width;
  int thisX, thisY;
  switch (cfg.matrix) {
    case 1: thisX = x;                    thisY = y;                    break;
    case 2: thisX = y;                    thisY = x;                    break;
    case 3: thisX = x;                    thisY = (cfg.length - y - 1); break;
    case 4: thisX = (cfg.length - y - 1); thisY = x;                    break;
    case 5: thisX = (cfg.width - x - 1);  thisY = (cfg.length - y - 1); break;
    case 6: thisX = (cfg.length - y - 1); thisY = (cfg.width - x - 1);  break;
    case 7: thisX = (cfg.width - x - 1);  thisY = y;                    break;
    case 8: thisX = y;                    thisY = (cfg.width - x - 1);  break;
  }

  if ( !(thisY & 1) || (cfg.deviceType - 2) ) return (thisY * matrixW + thisX);   // чётная строка
  else return (thisY * matrixW + matrixW - thisX - 1);                            // нечётная строка
}
/*
   целочисленный мап
   y = ( (y1 - y2) * x + (x1y2 - x2y1) ) / (x1-x2)
   y = ( (y2 - y1) * x + 255 * y1 ) / 255
  (x + 128) / 255  -> 0.5-2
  (x*5 + 51) / 255  -> 0.2-5
  (x*1.9 + 25) / 255  -> 0.1-1
*/
