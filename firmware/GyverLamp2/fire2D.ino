const unsigned char valueMask[11][16] PROGMEM = {
  {8  , 0  , 0  , 0  , 0  , 0  , 0  , 8   , 8 , 0  , 0  , 0  , 0  , 0  , 0  , 8  },
  {16 , 0  , 0  , 0  , 0  , 0  , 0  , 16 , 16 , 0  , 0  , 0  , 0  , 0  , 0  , 16 },
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255},
  {255, 220, 185, 150, 150, 185, 220, 255, 255, 220, 185, 150, 150, 185, 220, 255},
};
const unsigned char hueMask[11][16] PROGMEM = {
  {8 , 16, 32, 36, 36, 32, 16, 8 , 8 , 16, 32, 36, 36, 32, 16, 8 },
  {5 , 14, 29, 31, 31, 29, 14, 5 , 5 , 14, 29, 31, 31, 29, 14, 5 },
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 },
  {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
};

byte fireLine[100];

void fireRoutine() {
  shiftUp();
  FOR_i(0, cfg.width) fireLine[i] = random(64, 255);
  drawFrame(30);
}

void shiftUp() {
  for (int y = cfg.length - 1; y > 0; y--) {
    for (int x = 0; x < cfg.width; x++) {
      int newX = x;
      if (x > 15) newX = x - 15;
      if (y > 10) continue;
      matrixValue[y][newX] = matrixValue[y - 1][newX];
    }
  }

  for (int x = 0; x < cfg.width; x++) {
    int newX = x;
    if (x > 15) newX = x - 15;
    matrixValue[0][newX] = fireLine[newX];
  }
}

void drawFrame(int pcnt) {
  int nextv;
  for (int y = cfg.length - 1; y > 0; y--) {
    for (byte x = 0; x < cfg.width; x++) {
      int newX = x;
      if (x > 15) newX = x - 15;
      if (y < 11) {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&(valueMask[y][newX]));

        leds[getPix(x, y)] = CHSV(
                               CUR_PRES.color + pgm_read_byte(&(hueMask[y][newX])), // H
                               255, // S
                               (uint8_t)max(0, nextv) // V
                             );
      }
    }
  }

  for (int x = 0; x < cfg.width; x++) {
    int newX = x;
    if (x > 15) newX = x - 15;
    leds[getPix(newX, 0)] = CHSV(
                              CUR_PRES.color + pgm_read_byte(&(hueMask[0][newX])), // H
                              255,           // S
                              (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * fireLine[newX]) / 100.0) // V
                            );
  }
}
