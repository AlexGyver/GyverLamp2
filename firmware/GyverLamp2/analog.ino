#if (USE_ADC == 1)
void setupADC() {
  low.setDt(0);
  low.setPeriod(0);
  low.setWindow(0);
  high.setDt(0);
  high.setPeriod(0);
  high.setWindow(0);

  vol.setVolK(20);
  low.setVolK(20);
  high.setVolK(20);

  vol.setTrsh(50);
  low.setTrsh(50);
  high.setTrsh(50);

  vol.setVolMin(0);
  low.setVolMin(0);
  high.setVolMin(0);

  vol.setVolMax(255);
  low.setVolMax(255);
  high.setVolMax(255);

  phot.setDt(80);
  phot.setK(31);
}


void checkAnalog() {
  if (cfg.state) {
    switch (cfg.adcMode) {
      case GL_ADC_NONE: break;
      case GL_ADC_BRI: checkPhot(); break;
      case GL_ADC_MIC: checkMusic(); break;
      case GL_ADC_BOTH:
        {
          static timerMillis tmr(1000, 1);
          if (tmr.isReady()) {
            switchToPhot();
            phot.setRaw(analogRead(A0));
            switchToMic();
          } else {
            checkMusic();
          }
          phot.compute();
        }
        break;
    }
  }
}

void checkMusic() {
  if (CUR_PRES.soundMode > 1) {
    if (CUR_PRES.soundMode == GL_MUS_VOL) {   // громкость
      vol.tick();
    } else {                                  // частоты
      int raw[FFT_SIZE], spectr[FFT_SIZE];
      for (int i = 0; i < FFT_SIZE; i++) raw[i] = analogRead(A0);
      FFT(raw, spectr);
      int low_raw = 0;
      int high_raw = 0;
      for (int i = 0; i < FFT_SIZE / 2; i++) {
        spectr[i] = (spectr[i] * (i + 2)) >> 1;
        if (i < 2) low_raw += spectr[i];
        else high_raw += spectr[i];
      }
      low.tick(low_raw);
      high.tick(high_raw);
    }
  }
}

void checkPhot() {
  static timerMillis tmr(1000, true);
  if (tmr.isReady()) phot.setRaw(analogRead(A0));
  phot.compute();
}

byte getSoundVol() {
  switch (CUR_PRES.soundMode) {
    case GL_MUS_VOL: return vol.getVol();
    case GL_MUS_LOW: return low.getVol();
    case GL_MUS_HIGH: return high.getVol();
  }
  return 0;
}

void switchToMic() {
  digitalWrite(PHOT_VCC, 0);
  pinMode(PHOT_VCC, INPUT);
  pinMode(MIC_VCC, OUTPUT);
  digitalWrite(MIC_VCC, 1);
}
void switchToPhot() {
  digitalWrite(MIC_VCC, 0);
  pinMode(MIC_VCC, INPUT);
  pinMode(PHOT_VCC, OUTPUT);
  digitalWrite(PHOT_VCC, 1);
}
void disableADC() {
  digitalWrite(PHOT_VCC, 0);
  pinMode(PHOT_VCC, INPUT);
  digitalWrite(MIC_VCC, 0);
  pinMode(MIC_VCC, INPUT);
}
#else
void setupADC() {}
void checkAnalog() {}
void checkMusic() {}
void checkPhot() {}
byte getSoundVol() {
  return 0;
}
void switchToMic() {}
void switchToPhot() {}
void disableADC() {}
#endif
