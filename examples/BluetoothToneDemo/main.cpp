#include <Arduino.h>
#include "BluetoothTonePlayer.h"

BluetoothTonePlayer player(2,3,5,60000);

void setup() {
  Serial.begin(115200);
  player.begin(9600);
  Serial.println("PlatformIO main.cpp example running.");
}

void loop() {
  player.update();
}
