/* 
This code calculates the arm angles but using C++ for the first time. The main purpose of this
file is to test how much faster C++ is in comparison to python, ultimately pushing us towards doing
the calculations on the ESP32 instead of on the elsewhere.
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <stdexcept>

std::vector<float> calculate_angles(std::vector<float> target, float L1, float L2, float L3, float hand_theta, float TABLE);
bool is_valid(std::vector<float> theta, float L1, float L2, float L3, float table);

int main() {
    const float TABLE = -98.08, L1 = 137.4, L2 = 85.8, L3 = 103.3, HAND_THETA = 260;
    std::vector<float> TARGET = {-240,-85};
 
    try {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<float> sigma = calculate_angles(TARGET, L1, L2, L3, HAND_THETA, TABLE);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Time to calculate: " << elapsed.count() << "s\n";

        for (float val : sigma) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    } catch (const std::invalid_argument) {
        std::cout << "Invalid Target";
        return 1;
    }
    return 0;
}

std::vector<float> calculate_angles(std::vector<float> target, float L1, float L2, float L3, float hand_theta, float table) {
    std::vector<float> theta = {0.0, 0.0, 0.0};
    hand_theta *=  M_PI / 180;
    target[0] -= L3 * cos(hand_theta);
    target[1] -= L3 * sin(hand_theta);
    float r_target = sqrt(pow(target[0], 2) + pow(target[1], 2));
    float theta_target = atan(target[1] / target[0]);

    theta[1] = acos((pow(L1, 2) + pow(L2, 2) - pow(r_target, 2)) / (2 * L1 * L2));
    theta[0] = asin(sin(theta[1]) / r_target * L2) + theta_target;
    theta[1] = M_PI - theta[1];
    theta[2] = M_PI / 2 + hand_theta + theta[1] - theta[0];

    if (target[0] <= 0) {
        theta[0] += M_PI;
        theta[2] += M_PI;
    }

    if (theta[2] >= 2 * M_PI) {
        theta[2] -= 2 * M_PI;
    }

    if (is_valid(theta, L1, L2, L3, table)) {
        for (int i = 0; i < 3; i++ ) {
            theta[i] *= (180 / M_PI);
        }
        return theta;  
    }

    throw std::invalid_argument("Invalid Target");   
}

bool is_valid(std::vector<float> theta, float L1, float L2, float L3, const float TABLE) {
    const float FOOT_LENGTH = 116.46, FOOT_HEIGHT = 15.2;
    const int INNER_HUB_RADIUS = 45, INNER_HUB_HEIGHT = 27;
    float x1, x2, x3, y1, y2, y3, a, b, c, alpha, beta;
    std::vector<float> v1, v2, v3, v4;

    if (std::isnan(theta[0]) || std::isnan(theta[1]) || std::isnan(theta[2])) {
        return false;
    }

    if (theta[0] > M_PI || theta[0] < 0) {
        return false;
    }

    if (theta[1] >= M_PI || theta[1] < 0) {
        return false;
    }

    if (theta[2] > M_PI || theta[2] < 0) {
        return false;
    }

    x1 = L1 * cos(theta[0]);
    y1 = L1 * sin(theta[0]);

    x2 = L2 * cos(theta[0] - theta[1]);
    y2 = L2 * sin(theta[0] - theta[1]);

    x3 = L3 * cos(theta[0] - theta[1] + theta[2] - M_PI/2);
    y3 = L3 * sin(theta[0] - theta[1] + theta[2] - M_PI/2);

    v1 = {0,0};
    v2 = {x1,y1};
    v3 = {x1 + x2, y1 + y2};
    v4 = {x1 + x2 + x3, y1 + y2 + y3};

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