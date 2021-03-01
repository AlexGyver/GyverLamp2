// ============= Огонь 2020 ===============
// (c) SottNick
//сильно по мотивам https://pastebin.com/RG0QGzfK
//Perlin noise fire procedure by Yaroslaw Turbin
//https://www.reddit.com/r/FastLED/comments/hgu16i/my_fire_effect_implementation_based_on_perlin/

void fire2020(byte scale, int len) {
  static uint8_t deltaValue;
  static uint8_t deltaHue;
  static uint8_t step;
  static uint8_t shiftHue[50];
  static float trackingObjectPosX[100];
  static float trackingObjectPosY[100];
  static uint16_t ff_x, ff_y, ff_z;


  if (loading) {
    loading = false;
    //deltaValue = (((scale - 1U) % 11U + 1U) << 4U) - 8U; // ширина языков пламени (масштаб шума Перлина)
    deltaValue = map(scale, 0, 255, 8, 168);
    deltaHue = map(deltaValue, 8U, 168U, 8U, 84U); // высота языков пламени должна уменьшаться не так быстро, как ширина
    step = map(255U - deltaValue, 87U, 247U, 4U, 32U); // вероятность смещения искорки по оси ИКС
    for (uint8_t j = 0; j < cfg.length; j++) {
      shiftHue[j] = (cfg.length - 1 - j) * 255 / (cfg.length - 1); // init colorfade table
    }

    for (uint8_t i = 0; i < cfg.width / 8; i++) {
      trackingObjectPosY[i] = random8(cfg.length);
      trackingObjectPosX[i] = random8(cfg.width);
    }
  }
  for (uint8_t i = 0; i < cfg.width; i++) {
    for (uint8_t j = 0; j < len; j++) {
      leds[getPix(i, len - 1U - j)] = ColorFromPalette(paletteArr[CUR_PRES.palette - 1], scalePal(qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j])), 255U);
    }
  }

  //вставляем искорки из отдельного массива
  for (uint8_t i = 0; i < cfg.width / 8; i++) {
    if (trackingObjectPosY[i] > 3U) {
      leds[getPix(trackingObjectPosX[i], trackingObjectPosY[i])] = leds[getPix(trackingObjectPosX[i], 3U)];
      leds[getPix(trackingObjectPosX[i], trackingObjectPosY[i])].fadeToBlackBy( trackingObjectPosY[i] * 2U );
    }
    trackingObjectPosY[i]++;
    if (trackingObjectPosY[i] >= len) {
      trackingObjectPosY[i] = random8(4U);
      trackingObjectPosX[i] = random8(cfg.width);
    }
    if (!random8(step))
      trackingObjectPosX[i] = (cfg.width + (uint8_t)trackingObjectPosX[i] + 1U - random8(3U)) % cfg.width;
  }
  ff_y++;
  if (ff_y & 0x01) ff_z++;
}
