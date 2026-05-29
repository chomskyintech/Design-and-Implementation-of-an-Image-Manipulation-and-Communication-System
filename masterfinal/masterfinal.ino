#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define JOY_X A0
#define JOY_Y A1
#define JOY_SW 4        // joystick button pin
#define SPEED 2
#define DEADZONE 80

#define BT_RX 2
#define BT_TX 3
#define HOLD_THRESHOLD 800

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial BT(BT_RX, BT_TX);

int centreX, centreY;
int letX = 56, letY = 24;

// AVAILABLE LETTERS
const char letters[] = {'F','G','J','K','L','P','Q','R'};
const int totalLetters = 8;
int currentLetter = 0;

// HOLD TIMERS FOR LETTER CHANGE
unsigned long letterHoldStart = 0;
bool letterTriggered = false;

// BUTTON STATE
bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool buttonHeld = false;

// STATUS
String statusText = "Ready";
String inputBuffer = "";

// SEND DATA TO SLAVE
void sendData() {
  String msg = "(";
  msg += letters[currentLetter];
  msg += ",";
  msg += String(letX);
  msg += ",";
  msg += String(letY);
  msg += ",0)";
  BT.println(msg);
  statusText = "Sent!";
}

// PARSE INCOMING PACKET FROM SLAVE
void parseIncoming(String line) {
  line.trim();
  if (!line.startsWith("(") || !line.endsWith(")")) {
    statusText = "Bad pkt";
    return;
  }
  line.remove(0, 1);
  line.remove(line.length() - 1);

  int p1 = line.indexOf(',');
  int p2 = line.indexOf(',', p1 + 1);
  int p3 = line.indexOf(',', p2 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0) {
    statusText = "Bad fmt";
    return;
  }

  char receivedChar = line.charAt(0);
  for (int i = 0; i < totalLetters; i++) {
    if (letters[i] == receivedChar) {
      currentLetter = i;
      break;
    }
  }

  letX = constrain(line.substring(p1 + 1, p2).toInt(), 0, SCREEN_WIDTH  - 12);
  letY = constrain(line.substring(p2 + 1, p3).toInt(), 0, SCREEN_HEIGHT - 16);

  statusText = "Recv:";
  statusText += receivedChar;
}

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(JOY_SW, INPUT_PULLUP);  // button uses internal pullup

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
  statusText = "Ready";
}

void loop() {
  int jx = analogRead(JOY_X) - centreX;
  int jy = analogRead(JOY_Y) - centreY;
  unsigned long now = millis();
  bool buttonState = digitalRead(JOY_SW);  // LOW when pressed

  if (abs(jx) < DEADZONE) jx = 0;
  if (abs(jy) < DEADZONE) jy = 0;

  // DETECT BUTTON PRESS AND HOLD
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Button just pressed
    buttonPressTime = now;
    buttonHeld = false;
  }

  if (buttonState == LOW && (now - buttonPressTime >= HOLD_THRESHOLD)) {
    // Button is being held
    buttonHeld = true;
  }

  if (buttonState == HIGH && lastButtonState == LOW) {
    // Button just released
    if (!buttonHeld) {
      // Short press -> SEND
      sendData();
    }
    buttonHeld = false;
    letterTriggered = false;
  }

  lastButtonState = buttonState;

  // BUTTON HELD + JOYSTICK -> CHANGE LETTER
  if (buttonHeld) {
    statusText = "Btn held...";

    if ((jx > DEADZONE || jx < -DEADZONE || jy > DEADZONE || jy < -DEADZONE) && !letterTriggered) {
      if (jx > DEADZONE) {
        // Hold button + RIGHT -> next letter
        currentLetter = (currentLetter + 1) % totalLetters;
      } else if (jx < -DEADZONE) {
        // Hold button + LEFT -> previous letter
        currentLetter = (currentLetter - 1 + totalLetters) % totalLetters;
      } else if (jy > DEADZONE || jy < -DEADZONE) {
        // Hold button + UP or DOWN -> next letter (same as right)
        currentLetter = (currentLetter + 1) % totalLetters;
      }
      letterTriggered = true;
      statusText = "Letter:";
      statusText += letters[currentLetter];
    }

    // Reset trigger when joystick returns to centre
    if (jx == 0 && jy == 0) {
      letterTriggered = false;
    }

  } else {
    // NORMAL MOVEMENT — full freedom in all directions
    if (jx > 0) letX += SPEED;
    if (jx < 0) letX -= SPEED;
    if (jy > 0) letY += SPEED;
    if (jy < 0) letY -= SPEED;
  }

  letX = constrain(letX, 0, SCREEN_WIDTH  - 12);
  letY = constrain(letY, 0, SCREEN_HEIGHT - 16);

  // DISPLAY
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(letX, letY);
  display.print(letters[currentLetter]);

  // Coordinates bottom left
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(F("X:")); display.print(letX);
  display.print(F(" Y:")); display.print(letY);

  // Status bottom right
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
