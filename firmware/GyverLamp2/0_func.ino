void sendUDP(char *data) {
  IPAddress ip = WiFi.localIP();
  ip[3] = 255;
  Udp.beginPacket(ip, 50000 + cfg.group);
  Udp.write(data);
  Udp.endPacket();
}
void restartUDP() {
  DEBUG("UDP port: ");
  DEBUGLN(50000 + cfg.group);
  Udp.stop();
  Udp.begin(50000 + cfg.group);
}
void blink8(CRGB color) {
  FOR_i(0, 3) {
    fill_solid(leds, 8, color);
    FastLED.show();
    delay(300);
    FastLED.clear();
    FastLED.show();
    delay(300);
  }
}
