// Using an ESP32 and PCA9685 servo driver board. 
// This code has been cleaned up and should be the only one used. 
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "index_html.h"
#include "arm_movement.h"
#include "arm_calculations.h"

/* 
Your SSID and Password to be put here. 
Once the code is uploaded to the ESP32, check the serial for the IP address.
Paste this address into your browser to access the controls. Usable on any device that has a browser. 
*/

String ssid = "YOUR_SSID";
String password = "YOUR_PASSWORD";

// Constants that are used when moving the arm
struct MovementConstants {
  int8_t translationalChange = 15;
  int8_t wristPitchChange = 10;
  int8_t yawChange = 10;
  int16_t timeBetweenMovementsDelay = 250;
  int16_t gripDelay = 1000;
  int16_t releaseDelay = 500;
};

void printStatus();
void functionMovementButtonPressed(Position &position);

WiFiServer server(80);
MovementConstants movementConstants;
Position homePosition, position1, position2, position3, position4;

const int MOVEMENT_SPEED = 65; // MOVEMENT_SPEED, range = [0, 100]
bool allowPrintStatus = false; // If changed to true, information about where the arm currently is will be printed to the serial
bool saveButtonPressed = false;


void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  pwm.begin();
  pwm.setPWMFreq(50);
  WiFi.begin(ssid, password);

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Initial conditions
  homePosition.targetX = 150, homePosition.targetY = 100, homePosition.yawAngle = 90, homePosition.wristPitch = 0;
  lastPosition = currentPosition = homePosition;
  position1.targetX = 275, position1.targetY = -40, position1.yawAngle = 140, position1.wristPitch = 0;
  position2.targetX = 155, position2.targetY = -20, position2.yawAngle = 90, position2.wristPitch = -100;
  position3.targetX = 170, position3.targetY = -65, position3.yawAngle = 180, position3.wristPitch = -100;
  position4.targetX = 150, position4.targetY = 235, position4.yawAngle = 30, position4.wristPitch = 20;

  stopGripper();
  moveArm(homePosition, lastPosition, MOVEMENT_SPEED);
  printStatus();
}


void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  // Read from buffer
  String request = client.readStringUntil('\n');

  // Match the request with one of the following movements
  if (request.indexOf("GET /LEFT") >= 0) {
    currentPosition.yawAngle += movementConstants.yawChange;
  } else if (request.indexOf("GET /RIGHT") >= 0) {
    currentPosition.yawAngle -= movementConstants.yawChange;
  } else if (request.indexOf("GET /CLOSE") >= 0) {
    closeGripper();
  } else if (request.indexOf("GET /OPEN") >= 0) {
    openGripper();
  } else if (request.indexOf("GET /STOP") >= 0) {
    stopGripper();
  } else if (request.indexOf("GET /UP") >= 0) {
    currentPosition.targetY += movementConstants.translationalChange;
  } else if (request.indexOf("GET /DOWN") >= 0) {
    currentPosition.targetY -= movementConstants.translationalChange;
  } else if (request.indexOf("GET /BACKWARD") >= 0) {
    currentPosition.targetX -= movementConstants.translationalChange;
  } else if (request.indexOf("GET /FORWARD") >= 0) {
    currentPosition.targetX += movementConstants.translationalChange;
  } else if (request.indexOf("GET /PITCHUP") >= 0) {
    currentPosition.wristPitch += movementConstants.wristPitchChange;
  } else if (request.indexOf("GET /PITCHDOWN") >= 0) {
    currentPosition.wristPitch -= movementConstants.wristPitchChange;
  } else if (request.indexOf("GET /HOME") >= 0) {
    currentPosition = homePosition;
  } else if (request.indexOf("GET /POSITION1") >= 0) {
    functionMovementButtonPressed(position1);
  } else if (request.indexOf("GET /POSITION2") >= 0) {
    functionMovementButtonPressed(position2);
  } else if (request.indexOf("GET /POSITION3") >= 0) {
    functionMovementButtonPressed(position3);
  } else if (request.indexOf("GET /POSITION4") >= 0) {
    functionMovementButtonPressed(position4);
  } else if (request.indexOf("GET /SAVE") >= 0) {
    if (saveButtonPressed) {
      saveButtonPressed = false;
    } else {
      saveButtonPressed = true;
    }
  }

  float armAngles[3];
  calculateAngles(currentPosition.targetX, currentPosition.targetY, currentPosition.wristPitch, armAngles);

  // Check if the arm is in range before moving
  if (isValidTarget(armAngles) && currentPosition != lastPosition && 
      currentPosition.yawAngle <= constants.servoMaxAngle && currentPosition.yawAngle >= constants.servoMinAngle) {
    moveArm(currentPosition, lastPosition, MOVEMENT_SPEED);
    printStatus();
    lastPosition = currentPosition;
  } else {
    currentPosition = lastPosition;
  }

  // This is the beginning of the ChatGPT code
  // Prompt used is contained in the header file:
  if (request.indexOf("GET / ") >= 0) {
    client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n");

    client.print(INDEX_HTML);
  }
  else {
    client.print(
      "HTTP/1.1 204 No Content\r\n"
      "Connection: close\r\n\r\n"
    );
  }
  // This is the end of the ChatGPT code
  client.stop();
}

// Prints to the serial what the current position is
void printStatus() {
  if (allowPrintStatus) {
    Serial.print("Target = (");
    Serial.print(currentPosition.targetX);
    Serial.print(", ");
    Serial.print(currentPosition.targetY);
    Serial.print(") | ");
    Serial.print("Yaw angle = ");
    Serial.print(currentPosition.yawAngle);
    Serial.print(" | ");
    Serial.print("Wrist pitch = ");
    Serial.println(currentPosition.wristPitch);
  }
}

// Changes position if needed based on whether the save button has been pressed
void functionMovementButtonPressed(Position &position) {
  if (saveButtonPressed) {
    position = currentPosition;
    saveButtonPressed = false;
  } else {
    currentPosition = position;
  }
}

