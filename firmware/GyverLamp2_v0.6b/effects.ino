void effectsRoutine() {
  static timerMillis effTmr(30, true);
  if (cfg.state && effTmr.isReady()) {
    FastLED.setBrightness(getBright());

    int thisLength = getLength();
    byte thisScale = getScale();
    int thisWidth = (cfg.deviceType > 1) ? cfg.width : 1;

    switch (CUR_PRES.effect) {
      case 1: // =================================== ПЕРЛИН ===================================
        FastLED.clear();
        if (cfg.deviceType > 1) {
          FOR_j(0, cfg.length) {
            FOR_i(0, cfg.width) {
              leds[getPix(i, j)] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                                    inoise8(
                                                      i * (thisScale / 5) - cfg.width * (thisScale / 5) / 2,
                                                      j * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                                      (now.weekMs >> 1) * CUR_PRES.speed / 255),
                                                    255, LINEARBLEND);
            }
          }

        } else {
          FOR_i(0, cfg.length) {
            leds[i] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1],
                                       inoise8(i * (thisScale / 5) - cfg.length * (thisScale / 5) / 2,
                                               (now.weekMs >> 1) * CUR_PRES.speed / 255),
                                       255, LINEARBLEND);
          }
        }
        break;
      case 2: // ==================================== ЦВЕТ ====================================
        FastLED.clear();
        {
          fill_solid(leds, cfg.length * thisWidth, CHSV(CUR_PRES.color, thisScale, CUR_PRES.min));
          CRGB thisColor = CHSV(CUR_PRES.color, thisScale, CUR_PRES.max);
          if (CUR_PRES.fromCenter) {
            fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
            fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
          } else {
            fillStrip(0, thisLength, thisColor);
          }
        }
        break;
      case 3: // ================================= СМЕНА ЦВЕТА =================================
        FastLED.clear();
        {
          CRGB thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], (now.weekMs >> 5) * CUR_PRES.speed / 255, CUR_PRES.min, LINEARBLEND);
          fill_solid(leds, cfg.length * thisWidth, thisColor);
          thisColor = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], (now.weekMs >> 5) * CUR_PRES.speed / 255, CUR_PRES.max, LINEARBLEND);
          if (CUR_PRES.fromCenter) {
            fillStrip(cfg.length / 2, cfg.length / 2 + thisLength / 2, thisColor);
            fillStrip(cfg.length / 2 - thisLength / 2, cfg.length / 2, thisColor);
          } else {
            fillStrip(0, thisLength, thisColor);
          }
        }
        break;
      case 4: // ================================== ГРАДИЕНТ ==================================
        FastLED.clear();
        if (CUR_PRES.fromCenter) {
          FOR_i(cfg.length / 2, cfg.length) {
            byte bright = 255;
            if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < cfg.length / 2 + thisLength / 2) ? (CUR_PRES.max) : (CUR_PRES.min);
            CRGB thisColor = ColorFromPalette(
                               paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                               (i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128),
                               bright, LINEARBLEND);
            if (cfg.deviceType > 1) fillRow(i, thisColor);
            else leds[i] = thisColor;
          }
          if (cfg.deviceType > 1) FOR_i(0, cfg.length / 2) fillRow(i, leds[(cfg.length - i)*cfg.width - 1]);
          else FOR_i(0, cfg.length / 2) leds[i] = leds[cfg.length - i - 1];

        } else {
          FOR_i(0, cfg.length) {
            byte bright = 255;
            if (CUR_PRES.soundReact == GL_REACT_LEN) bright = (i < thisLength) ? (CUR_PRES.max) : (CUR_PRES.min);
            CRGB thisColor = ColorFromPalette(
                               paletteArr[CUR_PRES.palette - 1],   // (x*1.9 + 25) / 255 - быстрый мап 0..255 в 0.1..2
                               (i * (thisScale * 1.9 + 25) / cfg.length) + ((now.weekMs >> 3) * (CUR_PRES.speed - 128) / 128),
                               bright, LINEARBLEND);
            if (cfg.deviceType > 1) fillRow(i, thisColor);
            else leds[i] = thisColor;
          }
        }
        break;
      case 5: // =================================== ЧАСТИЦЫ ===================================
        FastLED.clear();
        if (cfg.deviceType > 1) {
          uint16_t rndVal = 0;
          FOR_i(0, thisScale / 8) {
            int thisY = inoise16(i * 100000000ul + (now.weekMs << 5) * CUR_PRES.speed / 255);
            thisY = map(thisY, 20000, 45000, 0, cfg.length);
            int thisX = inoise16(i * 100000000ul + 2000000000ul + (now.weekMs << 5) * CUR_PRES.speed / 255);
            thisX = map(thisX, 20000, 45000, 0, cfg.width);
            rndVal = rndVal * 2053 + 13849;     // random2053 алгоритм

            if (thisY >= 0 && thisY < cfg.length && thisX >= 0 && thisX < cfg.width)
              leds[getPix(thisX, thisY)] = CHSV(CUR_PRES.rnd ? rndVal : CUR_PRES.color, 255, 255);
          }
        } else {
          uint16_t rndVal = 0;
          FOR_i(0, thisScale / 8) {
            int thisPos = inoise16(i * 100000000ul + (now.weekMs << 5) * CUR_PRES.speed / 255);
            thisPos = map(thisPos, 20000, 45000, 0, cfg.length);
            rndVal = rndVal * 2053 + 13849;     // random2053 алгоритм
            if (thisPos >= 0 && thisPos < cfg.length) leds[thisPos] = CHSV(CUR_PRES.rnd ? rndVal : CUR_PRES.color, 255, 255);
          }
        }
        break;
      case 6: // ==================================== ОГОНЬ ====================================
        FastLED.clear();
        {
          if (cfg.deviceType > 1) {         // 2D огонь
            fireRoutine();
          } else {                          // 1D огонь
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
        }
        break;
      case 7: // ================================== КОНФЕТТИ ==================================
        FOR_i(0, thisScale >> 3) {
          byte x = random(0, cfg.length * cfg.width);
          if (leds[x] == CRGB(0, 0, 0)) leds[x] = CHSV(CUR_PRES.rnd ? random(0, 255) : CUR_PRES.color, 255, 255);
        }
        FOR_i(0, cfg.length * cfg.width) {
          if (leds[i].r >= 10 || leds[i].g >= 10 || leds[i].b >= 10) leds[i].fadeToBlackBy(CUR_PRES.speed / 2);
          else leds[i] = 0;
        }
        break;
    }

    // выводим нажатия кнопки
    if (btnClicks > 0) fill_solid(leds, btnClicks, CRGB::White);
    if (brTicks > 0) fill_solid(leds, brTicks, CRGB::Cyan);
    FastLED.show();
  }
}

byte getBright() {
  int maxBr = cfg.bright;
  byte fadeBr = 255;
  if (CUR_PRES.fadeBright) fadeBr = CUR_PRES.bright; // ограничен вручную

  if (cfg.adcMode == GL_ADC_BRI || cfg.adcMode == GL_ADC_BOTH) {    // ----> датчик света
    maxBr = constrain(phot.getFil(), cfg.minLight, cfg.maxLight);
    maxBr = map(maxBr, cfg.minLight, cfg.maxLight, cfg.minBright, cfg.maxBright);
  } else if (cfg.adcMode > 2 &&                      // ----> ацп мик
             CUR_PRES.soundMode > 1 &&               // светомузыка вкл
             CUR_PRES.soundReact == GL_REACT_BRI) {  // режим яркости
    fadeBr = mapFF(getSoundVol(), CUR_PRES.min, CUR_PRES.max);
  }
  return scaleFF(maxBr, fadeBr);
}

int getLength() {
  if (cfg.adcMode > 2                           // ацп мик
      && CUR_PRES.soundMode > 1                 // светомузыка вкл
      && CUR_PRES.soundReact == GL_REACT_LEN    // режим длины
     ) return mapFF(getSoundVol(), 0, cfg.length);
  else return cfg.length;
}

byte getScale() {
  if (cfg.adcMode > 2                           // ацп мик
      && CUR_PRES.soundMode > 1                 // светомузыка вкл
      && CUR_PRES.soundReact == GL_REACT_SCL    // режим масштаба
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

// получить номер пикселя в ленте по координатам
uint16_t getPix(int x, int y) {
  if ( !(y & 1) || (cfg.deviceType - 2) ) return (y * cfg.width + x);  // если чётная строка
  else return (y * cfg.width + cfg.width - x - 1);          // если нечётная строка
}
/*
   целочисленный мап
   y = ( (y1 - y2) * x + (x1y2 - x2y1) ) / (x1-x2)
   y = ( (y2 - y1) * x + 255 * y1 ) / 255
  (x + 128) / 255  -> 0.5-2
  (x*5 + 51) / 255  -> 0.2-5
  (x*1.9 + 25) / 255  -> 0.1-1
*/
