#include <Arduino.h>
#include <BluetoothTonePlayer.h>

// Wiring:
// HC-05 TX -> Arduino pin 2 (software RX)
// HC-05 RX -> Arduino pin 3 (software TX)  (make sure to use voltage divider to HC-05 RX if Arduino is 5V)
// Speaker + -> Arduino pin 5 (PWM capable), Speaker - -> GND

BluetoothTonePlayer player(2, 3, 5, 60000); // rx, tx, speakerPin, inactivity timeout (ms)

void setup() {
  Serial.begin(115200);
  player.begin(9600);
  Serial.println("BluetoothTonePlayer demo ready.");
  Serial.println("Commands:");
  Serial.println("TONE:<freq>");
  Serial.println("TONE:<freq>:<dur_ms>");
  Serial.println("MELODY:<note,dur;...> (example: MELODY:C4,300;D4,300;E4,600)");
  Serial.println("FX:BEEP | FX:ALARM | FX:UP | FX:DOWN");
  Serial.println("VOLUME:<0-10>");
  Serial.println("SAVE:<id>:<melody>");
  Serial.println("PLAY:<id>");
  Serial.println("STOP");
}

void loop() {
  player.update();
  // other app code can run here
}
