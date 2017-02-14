
#include <SoftwareWire.h>
#include "ssd1306_lib.h"

SSD1306 screen;

void setup() {
  // put your setup code here, to run once:
  SSD1306_Init(&screen, 2, 3);
  while(SSD1306_Loop(&screen)) delayMicroseconds(100);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  SSD1306_Display(&screen);
  while(SSD1306_Loop(&screen)) delayMicroseconds(100);
  Serial.println("done");
  while(1) {}
}
