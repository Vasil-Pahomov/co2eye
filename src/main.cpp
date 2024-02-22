#include <Arduino.h>
#include <NeoPixelBus.h>
#include <GyverTM1637.h>

#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
WiFiManager wifiManager;
WiFiUDP ntpUdp;
NTPClient ntpClient(ntpUdp, 3600);
GyverTM1637 disp(D2, D1);

#define BIGHTNESS_HYSTERESIS_PERCENT 10L
#define HYST_HI(val) (long)(val*(100L+BIGHTNESS_HYSTERESIS_PERCENT)/100)
#define HYST_LO(val) (long)(val*(100L-BIGHTNESS_HYSTERESIS_PERCENT)/100)

void setup() {
  Serial.begin(115200);
  wifiManager.autoConnect();
  ntpClient.begin();
  disp.clear();
  disp.brightness(7);
}

unsigned long lastMsNTP, lastMsSec;

int limits[] = {0, 30, 60, 300, 500, 700, 800, 900};

int currBrightness = 0;

void loop() {
  lastMsSec = millis();
  disp.point(POINT_ON);
  if (!ntpClient.isTimeSet() || (millis() - lastMsNTP > 60000)) {
    ntpClient.update();
    Serial.print('!');
    lastMsNTP = millis();
  }
  int startSec = ntpClient.getSeconds();
  if (ntpClient.isTimeSet()) {
    disp.displayClockScroll(ntpClient.getHours(), ntpClient.getMinutes(), 100);
  } else {
    disp.displayByte(_dash, _dash, _dash, _dash);
  }
  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '0' && c <= '7') {
      disp.brightness(c - '0'); 
    } else if (c == ' ') {

    }
  }

  int lumAcc = 0, lumAccCnt = 0;

  while (true)
  {
    disp.point(((millis() - lastMsSec) > 500) ? POINT_OFF : POINT_ON );
    if (ntpClient.getSeconds() != startSec) break;
    delay(100);
    lumAcc += analogRead(A0);
    lumAccCnt++;
  }

  if (lumAccCnt != 0) { lumAcc = lumAcc / lumAccCnt; }
  Serial.print(lumAcc);
  Serial.print(':');
  Serial.print(currBrightness);
  Serial.print("->");
  
  int lumIdx = 0;
  bool lumOnMargin = false;
  while ((lumIdx < (int)sizeof(limits)/sizeof(int)) && (lumAcc >= limits[lumIdx])) lumIdx ++;
  if (lumAcc)
  lumIdx--;
  Serial.print('(');Serial.print(lumIdx);Serial.print(')');

  if (lumIdx == currBrightness - 2) {
    currBrightness--;
  } else if (lumIdx == currBrightness - 1) {
    //do nothing - hysteresis
  } else
  {
    currBrightness = lumIdx;
  }
  disp.brightness(currBrightness);
  Serial.print(currBrightness);

  Serial.println();

}
