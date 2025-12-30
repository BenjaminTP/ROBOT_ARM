// This file contains all functions used to calculate arm angles
#pragma once

// Constants the were measured with the robot (e.g., lengths of the arms, servo limits)
struct numericalConstants {
  static constexpr float upperArmLength = 137.4f;
  static constexpr float foreArmLength = 85.8f;
  static constexpr float handLength = 103.3f;
  static constexpr float tableHeight = -98.08f;
  static constexpr float footLength = 116.46f;
  static constexpr float footHeight = 15.2f;
  static constexpr uint16_t innerHubRadius = 45;
  static constexpr uint16_t innerHubHeight = 26;
  static constexpr uint16_t servoMinAngle = 0;
  static constexpr uint16_t servoMaxAngle = 180;
  static constexpr uint16_t servoMinPulse = 150;
  static constexpr uint16_t servoMaxPulse = 600;
};

float angleToPulse(float angle);
float radiansToDegrees(float angle);
float degreesToRadians(float angle);
float degreesToRadians(float angle);
bool isValidTarget(float* armAngles);
bool checkRangeOfServos(float* armAngles);
bool checkIntersections(float* armAngles);
bool checkIntersectionsWithBase(float* armAngles, float v2[2], float v3[2], float v4[2]);

numericalConstants constants;

// Converts a given angle to a pulse that can be given to the PCA9685 board
float angleToPulse(float angle) {
  return map(angle, 0, 180, constants.servoMinPulse, constants.servoMaxPulse);
}


float radiansToDegrees(float angle) {
  return angle *= 180 / M_PI;
}


float degreesToRadians(float angle) {
  return angle *= M_PI / 180;
}

// Given three values and an array of angles, populate the given array with the angles needed to get the arm to the given target.
void calculateAngles(float targetX, float targetY, float wristPitch, float* armAngles) {
  // Adjust given values
  wristPitch = degreesToRadians(wristPitch);
  targetX -= constants.handLength * cos(wristPitch);
  targetY -= constants.handLength * sin(wristPitch);

  // Convert target to polar form
  float distanceToTarget = sqrt(pow(targetX, 2) + pow(targetY, 2));
  float angleToTarget = atan(targetY / targetX);

  // Solve for arm angles with trigonometry
  armAngles[1] = acos((pow(constants.upperArmLength, 2) + pow(constants.foreArmLength, 2) - pow(distanceToTarget, 2)) / (2 * constants.upperArmLength * constants.foreArmLength));
  armAngles[0] = asin(sin(armAngles[1]) / distanceToTarget * constants.foreArmLength) + angleToTarget;
  
  // Adjust for orientation of servos in real life
  armAngles[1] = M_PI - armAngles[1];
  armAngles[2] = M_PI / 2 + wristPitch + armAngles[1] - armAngles[0];
  if (targetX <= 0) {
    armAngles[0] += M_PI;
    armAngles[2] += M_PI;
  }
  if (armAngles[2] >= 2 * M_PI) {
    armAngles[2] -= 2 * M_PI;
  }

  // Return angles in degrees
  for (int i = 0; i < 3; i++) {
    armAngles[i] = radiansToDegrees(armAngles[i]);
  }
  wristPitch = radiansToDegrees(wristPitch);
}


bool isValidTarget(float* armAngles) {
  return checkRangeOfServos(armAngles) && checkIntersections(armAngles);
}


bool checkRangeOfServos(float* armAngles) {
  if (isnan(armAngles[0]) || isnan(armAngles[1]) || isnan(armAngles[2])) {
    return false;
  }

  if (armAngles[0] > constants.servoMaxAngle || armAngles[0] < constants.servoMinAngle) {
    return false;
  }

  if (armAngles[1] >= constants.servoMaxAngle || armAngles[1] < constants.servoMinAngle) {
    return false;
  }

  if (armAngles[2] > constants.servoMaxAngle || armAngles[2] < constants.servoMinAngle) {
    return false;
  }

  return true;
}


bool checkIntersections(float* armAngles) {
  // Calculate using intersections of line segments
  float x1, x2, x3, y1, y2, y3, a, b, c, alpha, beta;

  x1 = constants.upperArmLength * cos(degreesToRadians(armAngles[0]));
  y1 = constants.upperArmLength * sin(degreesToRadians(armAngles[0]));

  x2 = constants.foreArmLength * cos(degreesToRadians(armAngles[0] - armAngles[1]));
  y2 = constants.foreArmLength * sin(degreesToRadians(armAngles[0] - armAngles[1]));

  x3 = constants.handLength * cos(degreesToRadians(armAngles[0] - armAngles[1] + armAngles[2] - constants.servoMaxAngle / 2));
  y3 = constants.handLength * sin(degreesToRadians(armAngles[0] - armAngles[1] + armAngles[2] - constants.servoMaxAngle / 2));

  float v1[2] = {0,0}, v2[2] = {x1,y1}, v3[2] = {x1 + x2, y1 + y2}, v4[2] = {x1 + x2 + x3, y1 + y2 + y3};

  a = (v4[0]-v3[0])*(v3[1]-v1[1])-(v4[1]-v3[1])*(v3[0]-v1[0]);
  b = (v4[0]-v3[0])*(v2[1]-v1[1])-(v4[1]-v3[1])*(v2[0]-v1[0]);
  c = (v2[0]-v2[0])*(v3[1]-v1[1])-(v2[1]-v1[1])*(v3[0]-v1[0]);

  if (b == 0 || (a == 0 && b == 0)) {
    return false;
  }

  alpha = a / b;
  beta = c / b;

  // If the lines are close to intersection we have [-0.1, 1.1] instead of [0, 1]
  if (alpha >= -0.1 && alpha <= 1.1 && beta >= -0.1 && beta <= 1.1) {
    return false;
  }

  if (!checkIntersectionsWithBase(armAngles, v2, v3, v4)) {
    return false;
  }

  return true;
}


bool checkIntersectionsWithBase(float* armAngles, float v2[2], float v3[2], float v4[2]) {
  // Ensure the arm does not attempt to crash into its own base
  if (v4[1] <= constants.tableHeight || v3[1] <= constants.tableHeight || v2[1] <= constants.tableHeight) {
    return false;
  }

  if (v4[0] <= constants.footLength && v4[0] >= -constants.footLength && v4[1] <= constants.tableHeight + constants.footHeight) {
    return false;
  }

  if (v4[0] <= constants.innerHubRadius && v4[0] >= -constants.innerHubRadius && v4[1] <= constants.innerHubHeight) {
    return false;
  }

  /* 
    When plotting all possible positions of the arm, an anomaly was found such that the arm did not intersect with itself,
    reach outside the servo motor limits, nor did it place the target inside the base. This gave a very small blip near the origin
    that was technically accessible if the arm passed through the base. The solution to write another intersection check was considered,
    but it was a lot easier to draw a box around the blip from x = [-150,0] and y <= 10 that did not cut out anything other than the blip.
    This saved a lot of time and was a lot easier to implement, however, this solution is significantly less elegant. See PLOTS/BLIP.png.
  */
  if (v4[0] <= 0 && v4[0] >= -150 && v4[1] <= 10) {
    return false;
  }

  return true;
}

