/*
  Using an ESP32 and PCA9685 servo driver board, control the arm by pasting the IP address given in the Serial
  after uploading the code.
*/ 
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

struct Position {
  float x, y, yaw, pitch;
};

void execute_target();
bool is_valid(float theta[]);
float angleToPulse(float angle);
void printStatus();
void smoothMotion(float start, float end, int servo);
void smoothMotionForTheta(float start1, float end1, float start2, float end2, float start3, float end3);
void clearSerial();
void goToPosition(float targetX, float targetY, float yawAngle, float handPitch, bool reverse=false);

// Enter SSID and password here
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

Position home, pos1, pos2, pos3, pos4;
WiFiServer server(80);
Adafruit_PWMServoDriver pwm(0x40);

const int yawServo = 0, servo1 = 1, servo2 = 2, servo3 = 3, gripperServo = 4; // Initialize servo channels
const float TABLE = -98.08, L1 = 137.4, L2 = 85.8, L3 = 103.3, FOOT_LENGTH = 116.46, FOOT_HEIGHT = 15.2; // Initialize float constants
const int INNER_HUB_RADIUS = 45, INNER_HUB_HEIGHT = 27, SERVO_MIN = 150, SERVO_MAX = 600, SERVO_STOP = 85, change = 15, handThetaChange = 10, yawChange = 10; // Initialize int constants

float target[2] = {150,100}, target_copy[2] = {0,0}, theta[3] = {0,0,0}, hand_theta = 0, yaw_angle = 90;
float old_theta1 = 103.581, old_theta2 = 126.605, old_theta3 = 113.024;
char direction;
int invalid = 0, movementDelay = 250, gripDelay = 1000, releaseDelay = 500;
bool allowPrintStatus = true, saving = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21,22);
  pwm.begin();
  pwm.setPWMFreq(50);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  // Initial conditions
  home.x = 150, pos1.x = 275, pos2.x = 155, pos3.x = 295, pos4.x = 150;
  home.y = 100, pos1.y = -40, pos2.y = -20, pos3.y = -20, pos4.y = 205;
  home.yaw = 90, pos1.yaw = 140, pos2.yaw = 90, pos3.yaw = 80, pos4.yaw = 140;
  home.pitch = 0, pos1.pitch = 0, pos2.pitch = -100, pos3.pitch = -30, pos4.pitch = 0;

  // Set to home position
  goToPosition(150,100,90,0);
  pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = "";
  unsigned long timeout = millis() + 300;

  while (client.connected() && millis() < timeout) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n') break;
    }
  }
  
  
  if (request.indexOf("GET /LEFT") >= 0) {
    yaw_angle += yawChange;
      if (yaw_angle <= 180 && yaw_angle >= 0) {
        smoothMotion(yaw_angle - yawChange, yaw_angle, yawServo);
      } else {
        yaw_angle -= yawChange;
      }
      printStatus();
  }
  else if (request.indexOf("GET /RIGHT") >= 0) {
    yaw_angle -= yawChange;
      if (yaw_angle <= 180 && yaw_angle >= 0) {
        smoothMotion(yaw_angle + yawChange, yaw_angle, yawServo);
      } else {
        yaw_angle += yawChange;
      }
      printStatus();
  }
  else if (request.indexOf("GET /CLOSE") >= 0) {
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP + 20));
  }
  else if (request.indexOf("GET /OPEN") >= 0) {
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP - 20));
  }
  else if (request.indexOf("GET /STOP") >= 0) {
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
  }
  else if (request.indexOf("GET /UP") >= 0) {
    target[1] += change;
      execute_target();       
      if (invalid == 1) {
        target[1] -= change; // Undo if the result is invalid
      }
  }
  else if (request.indexOf("GET /DOWN") >= 0) {
    target[1] -= change;
      execute_target();
      if (invalid == 1) {
        target[1] += change;
      }
  }
  else if (request.indexOf("GET /BACKWARD") >= 0) {
    target[0] -= change;
      execute_target();
      if (invalid == 1) {
        target[0] += change;
      }
  }
  else if (request.indexOf("GET /FORWARD") >= 0) {
    target[0] += change;
      execute_target();
      if (invalid == 1) {
        target[0] -= change;
      }
  }
  else if (request.indexOf("GET /PITCHUP") >= 0) {
    hand_theta += handThetaChange;
      execute_target();
      if (invalid == 1) {
        hand_theta -= handThetaChange;
      }
  } 
  else if (request.indexOf("GET /PITCHDOWN") >= 0) {
    hand_theta -= handThetaChange;
      execute_target();
      if (invalid == 1) {
        hand_theta += handThetaChange;
      }
  }
  else if (request.indexOf("GET /HOME") >= 0) {
    goToPosition(home.x,home.y,home.yaw,home.pitch,true);
  }
  else if (request.indexOf("GET /POSITION1") >= 0) {
    if (saving) {
      pos1.x = target[0];
      pos1.y = target[1];
      pos1.yaw = yaw_angle;
      pos1.pitch = hand_theta;
      saving = false;
    }
    goToPosition(pos1.x,pos1.y,pos1.yaw,pos1.pitch);
  }
  else if (request.indexOf("GET /POSITION2") >= 0) {
    if (saving) {
      pos2.x = target[0];
      pos2.y = target[1];
      pos2.yaw = yaw_angle;
      pos2.pitch = hand_theta;
      saving = false;
    }
    goToPosition(pos2.x,pos2.y,pos2.yaw,pos2.pitch);
  }
  else if (request.indexOf("GET /POSITION3") >= 0) {
    if (saving) {
      pos3.x = target[0];
      pos3.y = target[1];
      pos3.yaw = yaw_angle;
      pos3.pitch = hand_theta;
      saving = false;
    }
    goToPosition(pos3.x,pos3.y,pos3.yaw,pos3.pitch);
  }
  else if (request.indexOf("GET /POSITION4") >= 0) {
    if (saving) {
      pos4.x = target[0];
      pos4.y = target[1];
      pos4.yaw = yaw_angle;
      pos4.pitch = hand_theta;
      saving = false;
    }
    goToPosition(pos4.x,pos4.y,pos4.yaw,pos4.pitch);
  } else if (request.indexOf("GET /POSITION5") >= 0) {    // Show off degrees of freedom
    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
    delay(movementDelay);

    goToPosition(225,100,90,0); // Forward
    delay(movementDelay);

    goToPosition(150,100,90,0); // Backward
    delay(movementDelay);

    goToPosition(210,100,90,0); // Forward again
    delay(movementDelay);

    goToPosition(210,175,90,0); // Up
    delay(movementDelay);

    goToPosition(210,55,90,0); // Down
    delay(movementDelay);

    goToPosition(210,70,90,20); // Pitch up
    delay(movementDelay);

    goToPosition(210,55,90,-20); // Pitch down
    delay(movementDelay);

    goToPosition(150,250,0,30); // Yaw right
    delay(1000);

    // Grip
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP + 20));
    delay(gripDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(150,100,180,0,true); // Yaw left
    //delay(movementDelay);

    goToPosition(170,-65,180,-100); // Place down
    delay(movementDelay);

    // Release
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP - 20));
    delay(releaseDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
  } else if (request.indexOf("GET /POSITION6") >= 0) {    // Complete set cycle
    goToPosition(home.x,home.y,home.yaw,home.pitch,true);// Home
    delay(movementDelay);

    goToPosition(pos1.x,pos1.y,pos1.yaw, pos1.pitch); // Position 1
    delay(movementDelay);

    // Grip
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP + 20));
    delay(gripDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
    delay(movementDelay);

    goToPosition(pos2.x,pos2.y,pos2.yaw,pos2.pitch); // Position 2
    delay(movementDelay);

    // Release
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP - 20));
    delay(releaseDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
    delay(movementDelay);

    goToPosition(pos1.x,pos1.y,pos1.yaw, pos1.pitch); // Position 1
    delay(movementDelay);

    // Grip
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP + 20));
    delay(gripDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
    delay(movementDelay);

    goToPosition(pos3.x,pos3.y,pos3.yaw,pos3.pitch); // Position 3
    delay(movementDelay);

    // Release
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP - 20));
    delay(releaseDelay);
    pwm.setPWM(gripperServo, 0, angleToPulse(SERVO_STOP));
    delay(movementDelay);

    goToPosition(home.x,home.y,home.yaw,home.pitch,true); // Home
    delay(movementDelay);
  } else if (request.indexOf("GET /SAVE") >= 0) {
    if (saving) {
      saving = false;
    } else {
      saving = true;
    }
  }

  // This is the beginning of the ChatGPT code
  /* Prompt used:
      Create a game controller using HTML, CSS, and JS that contains these components:
      - Forward, backward, left, and right in a plus shape.
      - Up and down, pitch up and pitch down, and open and close as pairs.
      - Also include a section for functions ranging from 1-4 as well as a home and save button.
      Make the design dark, sleek and simple, and have it send a command I can read with an ESP32.
      Give me the HTML, CSS, and JS code then seperately, give me the same code but format it so 
      it can work with Wifi.h on an ESP32. (This should be pasted into client.print())
  */
  if (request.indexOf("GET / ") >= 0) {
    client.print(F(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"

    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>Robot Arm 2.0</title>"

    "<style>"
    "body{font-family:Arial;background:#111;color:#eee;margin:0;padding:15px;}"
    "h1{text-align:center;margin-bottom:20px;}"
    ".main{display:flex;flex-direction:column;align-items:center;gap:20px;}"
    ".panel{background:#1e1e1e;padding:15px 20px;border-radius:10px;}"
    ".panel h2{text-align:center;margin-bottom:10px;}"
    "button{background:#333;color:#fff;border:none;border-radius:8px;padding:12px 18px;font-size:16px;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none;}"
    "button:active{background:#00aaff;}"
    ".dpad{display:grid;grid-template-columns:60px 60px 60px;grid-template-rows:60px 60px 60px;gap:6px;justify-items:center;align-items:center;}"
    ".up{grid-column:2;grid-row:1;}.left{grid-column:1;grid-row:2;}.right{grid-column:3;grid-row:2;}.down{grid-column:2;grid-row:3;}"
    ".stack{display:flex;flex-direction:column;gap:10px;}"
    ".functions{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;}"
    "#save_button.active {background-color:aquamarine;color:#000;}"
    "@media(min-width:900px){.main{flex-direction:row;flex-wrap:wrap;justify-content:center;align-items:flex-start;}}"
    "</style>"

    "</head><body>"

    "<h1>Robot Arm 2.0</h1>"
    "<div class='main'>"

    "<div class='panel'><h2>Movement</h2><div class='dpad'>"
    "<button class='up' ontouchstart=\"onTouchStart('FORWARD')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('FORWARD')\" onmouseup=\"onMouseUp()\">↑</button>"
    "<button class='left' ontouchstart=\"onTouchStart('LEFT')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('LEFT')\" onmouseup=\"onMouseUp()\">←</button>"
    "<button class='right' ontouchstart=\"onTouchStart('RIGHT')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('RIGHT')\" onmouseup=\"onMouseUp()\">→</button>"
    "<button class='down' ontouchstart=\"onTouchStart('BACKWARD')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('BACKWARD')\" onmouseup=\"onMouseUp()\">↓</button>"
    "</div></div>"

    "<div class='panel'><h2>Vertical</h2><div class='stack'>"
    "<button ontouchstart=\"onTouchStart('UP')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('UP')\" onmouseup=\"onMouseUp()\">Up</button>"
    "<button ontouchstart=\"onTouchStart('DOWN')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('DOWN')\" onmouseup=\"onMouseUp()\">Down</button>"
    "</div></div>"

    "<div class='panel'><h2>Pitch</h2><div class='stack'>"
    "<button ontouchstart=\"onTouchStart('PITCHUP')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('PITCHUP')\" onmouseup=\"onMouseUp()\">Pitch +</button>"
    "<button ontouchstart=\"onTouchStart('PITCHDOWN')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('PITCHDOWN')\" onmouseup=\"onMouseUp()\">Pitch -</button>"
    "</div></div>"

    "<div class='panel'><h2>Gripper</h2><div class='stack'>"
    "<button ontouchstart=\"onTouchStart('OPEN')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('OPEN')\" onmouseup=\"onMouseUp()\">Open</button>"
    "<button ontouchstart=\"onTouchStart('CLOSE')\" ontouchend=\"onTouchEnd()\" onmousedown=\"onMouseDown('CLOSE')\" onmouseup=\"onMouseUp()\">Close</button>"
    "</div></div>"

    "<div class='panel'><h2>Functions</h2><div class='functions'>"
    "<button onclick=\"cmd('SAVE')\" id=\"save_button\">Save</button>"
    "<button onclick=\"cmd('HOME')\">Home</button>"
    "<button onclick=\"cmd('POSITION1')\">1</button>"
    "<button onclick=\"cmd('POSITION2')\">2</button>"
    "<button onclick=\"cmd('POSITION3')\">3</button>"
    "<button onclick=\"cmd('POSITION4')\">4</button>"
    "<button onclick=\"cmd('POSITION5')\">5</button>"
    "<button onclick=\"cmd('POSITION6')\">6</button>"
    "</div></div>"

    "</div>"

    "<script>"
    "let timer=null;let touchActive=false;"
    "function startCmd(c){if(timer)return;fetch('/'+c);timer=setInterval(()=>fetch('/'+c),120);}"
    "function stopCmd(){clearInterval(timer);timer=null;fetch('/STOP');}"
    "function onTouchStart(c){touchActive=true;startCmd(c);}"
    "function onTouchEnd(){stopCmd();setTimeout(()=>touchActive=false,50);}"
    "function onMouseDown(c){if(touchActive)return;startCmd(c);}"
    "function onMouseUp(){if(touchActive)return;stopCmd();}"
    "function cmd(c){fetch('/'+c);}"
    "function cmd(c){const saveBtn = document.getElementById('save_button');if (c === 'SAVE') {saveBtn.classList.toggle('active');}"
    "else if (c !== 'HOME') {saveBtn.classList.remove('active');}fetch('/' + c);}"
    "</script>"

    "</body></html>"
    ));
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


void execute_target() {
  hand_theta *=  M_PI / 180;
  target_copy[0] = target[0];
  target_copy[1] = target[1];
  target[0] -= L3 * cos(hand_theta);
  target[1] -= L3 * sin(hand_theta);
  float r_target = sqrt(pow(target[0], 2) + pow(target[1], 2));
  float theta_target = atan(target[1] / target[0]);  

  theta[1] = acos((pow(L1, 2) + pow(L2, 2) - pow(r_target, 2)) / (2 * L1 * L2));    // Cosine law
  theta[0] = asin(sin(theta[1]) / r_target * L2) + theta_target;                  // Sine law
  theta[1] = M_PI - theta[1];
  theta[2] = M_PI / 2 + hand_theta + theta[1] - theta[0];

  if (target[0] <= 0) {
    theta[0] += M_PI;
    theta[2] += M_PI;
  }

  if (theta[2] >= 2 * M_PI) {
    theta[2] -= 2 * M_PI;
  }

  target[0] = target_copy[0];
  target[1] = target_copy[1];

  if (is_valid(theta)) {
    // Move the arm if the target is in the range
    for (int i = 0; i < 3; i++) {
      theta[i] *= 180 / M_PI;
    }

    hand_theta *=  180 / M_PI;

    invalid = 0;

    smoothMotionForTheta(old_theta1, theta[0], old_theta2, theta[1], old_theta3, theta[2]);
    old_theta1 = theta[0];
    old_theta2 = theta[1];
    old_theta3 = theta[2];

    printStatus();
  } else {
    Serial.println("Out of Range");
    hand_theta *=  180 / M_PI;
    invalid = 1;
  }
}


bool is_valid(float theta[]) { // Validate if the angles will be in the range of the arm
  if (isnan(theta[0]) || isnan(theta[1]) || isnan(theta[2])) {
    return false;
  }

  // Check range of each motor
  if (theta[0] > M_PI || theta[0] < 0) {
    return false;
  }

  if (theta[1] >= M_PI || theta[1] < 0) {
    return false;
  }

  if (theta[2] > M_PI || theta[2] < 0) {
    return false;
  }

  // Calculating intersections of line segments
  float x1, x2, x3, y1, y2, y3, a, b, c, alpha, beta;

  x1 = L1 * cos(theta[0]);
  y1 = L1 * sin(theta[0]);

  x2 = L2 * cos(theta[0] - theta[1]);
  y2 = L2 * sin(theta[0] - theta[1]);

  x3 = L3 * cos(theta[0] - theta[1] + theta[2] - M_PI/2);
  y3 = L3 * sin(theta[0] - theta[1] + theta[2] - M_PI/2);

  float v1[2] = {0,0}, v2[2] = {x1,y1}, v3[2] = {x1 + x2, y1 + y2}, v4[2] = {x1 + x2 + x3, y1 + y2 + y3};

  a = (v4[0]-v3[0])*(v3[1]-v1[1])-(v4[1]-v3[1])*(v3[0]-v1[0]);
  b = (v4[0]-v3[0])*(v2[1]-v1[1])-(v4[1]-v3[1])*(v2[0]-v1[0]);
  c = (v2[0]-v2[0])*(v3[1]-v1[1])-(v2[1]-v1[1])*(v3[0]-v1[0]);

  if (b == 0 || (a == 0 && b == 0)) {
    return false;
  }

  alpha = a / b;
  beta = c / b;

  if (alpha >= -0.1 && alpha <= 1.1 && beta >= -0.1 && beta <= 1.1) {
    return false;
  }

  // Calculating intersection with base
  if (v4[1] <= TABLE || v3[1] <= TABLE || v2[1] <= TABLE) {
    return false;
  }

  if (v4[0] <= FOOT_LENGTH && v4[0] >= -FOOT_LENGTH && v4[1] <= TABLE + FOOT_HEIGHT) {
    return false;
  }

  if (v4[0] <= INNER_HUB_RADIUS && v4[0] >= -INNER_HUB_RADIUS && v4[1] <= INNER_HUB_HEIGHT) {
    return false;
  }

  if (v4[0] <= 0 && v4[0] >= -150 && v4[1] <= 10) {
    return false;
  }

  return true;
}


float angleToPulse(float angle) { // Convert angle into pulses the servo can read
  return map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
}


void printStatus() { // Printing all values of each motor
  if (allowPrintStatus) {
    Serial.print("Servo 1 angle = ");
    Serial.print(theta[0]);
    Serial.print(" | ");
    Serial.print("Servo 2 angle = ");
    Serial.print(theta[1]);
    Serial.print(" | ");
    Serial.print("Servo 3 angle = ");
    Serial.print(theta[2]);
    Serial.print(" | ");
    Serial.print("Target = ");
    Serial.print(target[0]);
    Serial.print(", ");
    Serial.print(target[1]);
    Serial.print(" | ");
    Serial.print("Hand Angle = ");
    Serial.print(hand_theta);
    Serial.print(" | ");
    Serial.print("Yaw Angle = ");
    Serial.println(yaw_angle);
  }
}


void smoothMotion(float start, float end, int servo) {
  int steps = 200, duration = abs(end - start) * 10;
  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    float eased = (1 - cos(t * PI)) / 2;
    int pos = start + (end - start) * eased;
    pwm.setPWM(servo, 0, angleToPulse(pos));
    delay(duration/steps);
  }
}


void smoothMotionForTheta(float start1, float end1, float start2, float end2, float start3, float end3) {
  int steps = 200, duration = max(max(abs(end1 - start1), abs(end2 - start2)), abs(end3 - start3)) * 9;
  for (int i = 0; i <= steps; i++) {
    if (invalid == 1) {
      return;
    }
    float t = (float)i / steps;
    float eased = (1 - cos(t * PI)) / 2;

    int pos1 = start1 + (end1 - start1) * eased;
    pwm.setPWM(servo1, 0, angleToPulse(pos1));

    int pos2 = start2 + (end2 - start2) * eased;
    pwm.setPWM(servo2, 0, angleToPulse(pos2));

    int pos3 = start3 + (end3 - start3) * eased;
    pwm.setPWM(servo3, 0, angleToPulse(pos3));

    delay(duration/steps);
  }
}


void clearSerial() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}


void goToPosition(float targetX, float targetY, float yawAngle, float handPitch, bool reverse) {
  target[0] = targetX;
  target[1] = targetY;
  hand_theta = handPitch;
  if (reverse) {
    execute_target();
    smoothMotion(yaw_angle, yawAngle, yawServo);
    yaw_angle = yawAngle;
  } else {
    smoothMotion(yaw_angle, yawAngle, yawServo);
    yaw_angle = yawAngle;
    execute_target();
  }
}

