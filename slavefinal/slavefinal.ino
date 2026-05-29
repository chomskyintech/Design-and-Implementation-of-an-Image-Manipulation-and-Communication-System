#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define JOY_X A0
#define JOY_Y A1
#define SPEED 2
#define DEADZONE 80

#define BT_RX 2
#define BT_TX 3
#define HOLD_THRESHOLD 800

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial BT(BT_RX, BT_TX);

int centreX, centreY;

// RECEIVED LETTER — this is what slave works with
char currentLetter = ' ';  // starts empty, fills when master sends
int letX = 56, letY = 24;
bool letterReceived = false;  // nothing to show until master sends

// AVAILABLE LETTERS FOR CYCLING
const char letters[] = {'F','G','J','K','L','P','Q','R'};
const int totalLetters = 8;

// HOLD TIMERS
unsigned long downHoldStart  = 0;
unsigned long rightHoldStart = 0;
bool downTriggered  = false;
bool rightTriggered = false;

// STATUS
String statusText = "Waiting...";
String inputBuffer = "";

// SEND DATA BACK TO MASTER
void sendData() {
  if (!letterReceived) {
    statusText = "No letter!";
    return;
  }
  String msg = "(";
  msg += currentLetter;
  msg += ",";
  msg += String(letX);
  msg += ",";
  msg += String(letY);
  msg += ",0)";
  BT.println(msg);
  statusText = "Sent!";
}

// PARSE INCOMING PACKET FROM MASTER
void parseIncoming(String line) {
  line.trim();
  if (!line.startsWith("(") || !line.endsWith(")")) {
    statusText = "Bad packet";
    return;
  }
  line.remove(0, 1);
  line.remove(line.length() - 1);

  int p1 = line.indexOf(',');
  int p2 = line.indexOf(',', p1 + 1);
  int p3 = line.indexOf(',', p2 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0) {
    statusText = "Bad format";
    return;
  }

  // Take the letter and position from master
  currentLetter = line.charAt(0);
  letX = line.substring(p1 + 1, p2).toInt();
  letY = line.substring(p2 + 1, p3).toInt();

  letX = constrain(letX, 0, SCREEN_WIDTH  - 12);
  letY = constrain(letY, 0, SCREEN_HEIGHT - 16);

  letterReceived = true;
  statusText = "Got:";
  statusText += currentLetter;
}

// FIND NEXT LETTER IN ARRAY
int findLetterIndex(char c) {
  for (int i = 0; i < totalLetters; i++) {
    if (letters[i] == c) return i;
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  centreX = analogRead(JOY_X);
  centreY = analogRead(JOY_Y);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("OLED not found!"));
      while (true);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println(F("Keep joystick still"));
  display.setCursor(25, 35);
  display.println(F("Calibrating..."));
  display.display();
  delay(1500);
  statusText = "Waiting...";
}

void loop() {
  int jx = analogRead(JOY_X) - centreX;
  int jy = analogRead(JOY_Y) - centreY;
  unsigned long now = millis();

  if (abs(jx) < DEADZONE) jx = 0;
  if (abs(jy) < DEADZONE) jy = 0;

  // Only allow joystick control after letter is received
  if (letterReceived) {

    // HOLD RIGHT -> CYCLE LETTER
    if (jx > DEADZONE) {
      if (rightHoldStart == 0) rightHoldStart = now;
      else if ((now - rightHoldStart >= HOLD_THRESHOLD) && !rightTriggered) {
        int idx = findLetterIndex(currentLetter);
        currentLetter = letters[(idx + 1) % totalLetters];
        rightTriggered = true;
        statusText = "Letter:";
        statusText += currentLetter;
      }
    } else {
      rightHoldStart = 0;
      rightTriggered = false;
    }

    // HOLD DOWN -> SEND BACK TO MASTER
    if (jy < -DEADZONE) {
      if (downHoldStart == 0) downHoldStart = now;
      else if ((now - downHoldStart >= HOLD_THRESHOLD) && !downTriggered) {
        sendData();
        downTriggered = true;
      }
    } else {
      downHoldStart = 0;
      downTriggered = false;
    }

    // NORMAL MOVEMENT
    bool holdActive = (jx > DEADZONE) || (jy < -DEADZONE);
    if (!holdActive) {
      if (jx > 0) letX += SPEED;
      if (jx < 0) letX -= SPEED;
      if (jy > 0) letY += SPEED;
      if (jy < 0) letY -= SPEED;
    }

    letX = constrain(letX, 0, SCREEN_WIDTH  - 12);
    letY = constrain(letY, 0, SCREEN_HEIGHT - 16);
  }

  // DISPLAY
  display.clearDisplay();

  if (!letterReceived) {
    // Show waiting message until master sends something
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 28);
    display.print(F("Waiting for master"));
  } else {
    // Show the letter
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(letX, letY);
    display.print(currentLetter);

    // Coordinates bottom left
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print(F("X:")); display.print(letX);
    display.print(F(" Y:")); display.print(letY);
  }

  // Status bottom right
  display.setTextSize(1);
  display.setCursor(70, 56);
  display.print(statusText);

  display.display();
  delay(30);

  // BLUETOOTH RECEIVE
  while (BT.available()) {
    char c = BT.read();
    if (c == '\n') {
      parseIncoming(inputBuffer);
      inputBuffer = "";
    } else if (c != '\r') {
      inputBuffer += c;
    }
  }
}
