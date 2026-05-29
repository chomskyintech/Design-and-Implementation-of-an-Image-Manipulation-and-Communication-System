#include <SoftwareSerial.h>
SoftwareSerial BT(2, 3);

void setup() {
  Serial.begin(9600);
  BT.begin(9600);  // AT mode runs at 38400
}

void loop() {
  if (Serial.available()) BT.write(Serial.read());
  if (BT.available()) Serial.write(BT.read());
}