% Rotates the arm around a a given point
close all
clear
clc

% CALCULATE ANGLES FOR A GIVEN TARGET
function [theta] = calc(target, theta3)
    global L1;
    global L2;
    global L3;
    target = [target(1)-L3*cosd(theta3), target(2)-L3*sind(theta3)];
    theta = zeros(1,3);
    r_target  = norm(target);
    theta_target = atand(target(2)/target(1));
    
    theta(2) = acosd((L1^2 + L2^2 - r_target^2) / (2*L1*L2));
    theta(1) = asind(sind(theta(2)) / r_target * L2) + theta_target;
    theta(1) = real(theta(1));
    theta(2) = real(theta(2) - 180)*-1;
    theta(3) = real(theta(1)-theta(2)-90-theta3)*-1;
end

% PLOT ONE ARM FOR A GIVEN TARGET
function plot_arm(target, theta3)
    global L1;
    global L2;
    global L3;
    sigma = calc(target, theta3);
    plot(target(1),target(2),"o", "color", "w");
    plot([0, L1*cosd(sigma(1))], [0, L1*sind(sigma(1))],"LineWidth", 2, "color", "r");
    plot([L1*cosd(sigma(1)), L1*cosd(sigma(1)) + L2*cosd(sigma(1)-sigma(2))], [L1*sind(sigma(1)), L1*sind(sigma(1)) + L2*sind(sigma(1)-sigma(2))], "color", "b")
    plot([L1*cosd(sigma(1)) + L2*cosd(sigma(1)-sigma(2)),L1*cosd(sigma(1)) + L2*cosd(sigma(1)-sigma(2))+L3*cosd(theta3)], [L1*sind(sigma(1)) + L2*sind(sigma(1)-sigma(2)), L1*sind(sigma(1)) + L2*sind(sigma(1)-sigma(2)) + L3*sind(theta3)], "color", "c")
end

% SETUP
global L1; %#ok<*GVMIS>
global L2;
global L3;
L1 = 137.4;
L2 = 85.8;
L3 = 50;
xlim([-10,L1+L2+L3+10]);
ylim([-10,L1+L2+L3+10]);
pbaspect([1 1 1]);
hold on

% INITIALIZE CONSTANTS
START = 0;
END = 90;
NUMBER_OF_ARMS = 10;
TARGET = [130,130];

sigma = zeros(3,NUMBER_OF_ARMS);
theta3 = linspace(START, -END, NUMBER_OF_ARMS);

% PLOT ARMS AND CALCULATE ANGLES NEEDED TO ROTATED AROUND A POINT
tic
for i = 1:NUMBER_OF_ARMS
    sigma(:,i) = calc(TARGET, theta3(i));
    plot_arm(TARGET, theta3(i));
end
toc
hold off

disp(sigma')