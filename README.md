# Arduino Bluetooth Image Manipulation System

An embedded systems project based on the Arduino Uno that allows users to display, manipulate, and wirelessly transmit asymmetrical letter images between two devices using Bluetooth communication.

---

## Overview

This project implements a real-time embedded system using two Arduino Uno boards connected through HC-05 Bluetooth modules in a master-slave configuration. Users can manipulate letters on an SSD1306 OLED display using an analogue joystick and transmit the image data wirelessly between devices.

The system demonstrates:

* Embedded systems integration
* Bluetooth serial communication
* OLED graphics rendering
* Joystick input processing
* Real-time image manipulation
* Structured packet-based communication

---

## Features

* Display asymmetrical letters on a 128x64 OLED screen
* Real-time joystick-based movement control
* Wireless bidirectional Bluetooth communication
* Master-slave HC-05 configuration
* Structured ASCII packet transmission
* Deadzone filtering for stable joystick input
* Hold gestures for letter selection
* Lightweight bitmap image storage

---

## Hardware Used

* Arduino Uno (x2)
* SSD1306 OLED Display (128x64)
* HC-05 Bluetooth Module (x2)
* Analogue Joystick Module
* Breadboard and jumper wires

---

## Software & Libraries

Developed using the Arduino IDE with the following libraries:

```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
```

---

## Communication Protocol

The system uses a structured ASCII packet format:

```text
(letter,x,y,angle)
```

Example:

```text
(F,56,24,90)
```

Each packet contains:

* Letter identity
* X coordinate
* Y coordinate
* Rotation angle

---

## System Architecture

### Master Unit

* Controls local image manipulation
* Sends image data to slave device
* Receives modified data back from slave

### Slave Unit

* Receives image data from master
* Allows further manipulation
* Sends updated data back to master

Both systems use bidirectional UART communication through HC-05 modules configured using AT commands.

---

## Joystick Controls

### Normal Mode

* Move the displayed letter across the OLED screen

### Hold Mode

* Hold joystick/button for additional actions
* Cycle through available letters
* Trigger Bluetooth transmission

Supported letters:

```text
F, G, J, K, L, P, Q, R
```

---

## Key Embedded Systems Concepts

* UART Communication
* I2C Protocol
* ADC Input Processing
* Memory Optimisation using PROGMEM
* Real-Time Input Handling
* Bitmap Rendering
* Event-Based User Interaction

---

## Testing

The following subsystems were tested independently and together:

* OLED display rendering
* Bluetooth packet transmission
* Joystick responsiveness
* Bidirectional communication
* Packet parsing and validation

The system successfully transmitted and displayed image data between both devices with consistent results.

---

## Challenges Faced

* HC-05 modules occasionally remaining stuck in AT mode
* Limited Arduino Uno SRAM (2KB)
* Bluetooth compatibility issues with iPhone devices
* Reliable UART communication between modules

To overcome SRAM limitations, bitmap data was stored in program memory using `PROGMEM`.

---

## Future Improvements

* Add Bluetooth Low Energy (BLE)
* Implement display timeout/sleep mode
* Add image rotation support
* Improve power optimisation
* Expand bitmap/image library

---

## Conclusion

This project demonstrates the integration of multiple embedded hardware and software components into a fully functional wireless image manipulation system. It highlights practical applications of serial communication, graphical rendering, and real-time embedded programming on low-cost microcontroller hardware.

---

## Author

Taha 

