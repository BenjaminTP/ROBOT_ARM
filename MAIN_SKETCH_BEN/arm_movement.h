// This file contains all functions that involve movement of the arm
#pragma once
#include "Adafruit_PWMServoDriver.h"
#include "arm_calculations.h"

// Position stores the data we need to know where the arm is. 4DOF = 4 Parameters to tweak.
struct Position {
  int16_t targetX;
  int16_t targetY;
  int16_t wristPitch;
  int16_t yawAngle;

  // Overloading == and != operators
  bool operator==(const Position &other) const {
    return (targetX == other.targetX && 
            targetY == other.targetY &&
            wristPitch == other.wristPitch && 
            yawAngle == other.yawAngle);
  }

  bool operator!=(const Position &other) const {
    return !(*this == other);
  }
};

// Constants involving the PCA9685 servo driver board
struct ServoDriverBoardConstants {
  static constexpr uint8_t yawServo = 0;
  static constexpr uint8_t shoulderServo = 1;
  static constexpr uint8_t elbowServo = 2;
  static constexpr uint8_t wristServo = 3;
  static constexpr uint8_t gripperServo = 4;
  static constexpr uint8_t gripperStop = 85;
};

void closeGripper();
void openGripper();
void stopGripper();
void moveArm(Position currentPosition, Position lastPosition, int speed);
void smoothlyMoveBetweenAngles( float yawStart, float yawEnd, float shoulderStart, float shoulderEnd, 
                                float elbowStart, float elbowEnd, float wristStart, float wristEnd, int speed);

Position currentPosition, lastPosition;
ServoDriverBoardConstants servos;
Adafruit_PWMServoDriver pwm(0x40);

void closeGripper() {
  pwm.setPWM(servos.gripperServo, 0, angleToPulse(servos.gripperStop + 20));
}


void openGripper() {
  pwm.setPWM(servos.gripperServo, 0, angleToPulse(servos.gripperStop - 20));
}


void stopGripper() {
  pwm.setPWM(servos.gripperServo, 0, angleToPulse(servos.gripperStop));
}


void moveArm(Position currentPosition, Position lastPosition, int speed) {
  float currentArmAngles[3];
  float lastArmAngles[3];

  calculateAngles(currentPosition.targetX, currentPosition.targetY, currentPosition.wristPitch, currentArmAngles);
  calculateAngles(lastPosition.targetX, lastPosition.targetY, lastPosition.wristPitch, lastArmAngles);

  if (isValidTarget(currentArmAngles)) {
    smoothlyMoveBetweenAngles(lastPosition.yawAngle, currentPosition.yawAngle, lastArmAngles[0], currentArmAngles[0], 
                              lastArmAngles[1], currentArmAngles[1], lastArmAngles[2], currentArmAngles[2], speed);
  }
}


void smoothlyMoveBetweenAngles( float yawStart, float yawEnd, float shoulderStart, float shoulderEnd, 
                                float elbowStart, float elbowEnd, float wristStart, float wristEnd, int speed) {

  float changeInYaw = abs(yawEnd - yawStart), changeInShoulder = abs(shoulderEnd - shoulderStart),
        changeInElbow = abs(elbowEnd - elbowStart), changeInWrist = abs(wristEnd - wristStart);
  
  float maxChange = max(max(changeInYaw, changeInShoulder), max(changeInElbow, changeInWrist));
  
  /* 
    Steps will change based on two parameters: distance travelled, and the speed we want it to move at. 
    The numbers 3, 200, and 10 have been chosen to express how much weight each parameter has on the movement.
    More steps = more time to travel.
  */
  float steps = maxChange * 3 + map(speed, 0, 100, 200, 10);

  /*
    For each joint in the arm, we will update the position such that the velocity will follow a sinusuidal wave.
    This would also make the acceleration and jerk follow similar waves, causing the arm to move smoothly.
  */
  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    float eased = (1 - cos(t * PI)) / 2;

    int yawVal = yawStart + (yawEnd - yawStart) * eased;
    pwm.setPWM(servos.yawServo, 0, angleToPulse(yawVal));

    int shoulderVal = shoulderStart + (shoulderEnd - shoulderStart) * eased;
    pwm.setPWM(servos.shoulderServo, 0, angleToPulse(shoulderVal));

    int elbowVal = elbowStart + (elbowEnd - elbowStart) * eased;
    pwm.setPWM(servos.elbowServo, 0, angleToPulse(elbowVal));

    int wristVal = wristStart + (wristEnd - wristStart) * eased;
    pwm.setPWM(servos.wristServo, 0, angleToPulse(wristVal));
  }
}
