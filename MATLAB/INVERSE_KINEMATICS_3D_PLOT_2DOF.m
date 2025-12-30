% Calculates all possible shoulder and elbow angles, then plots the corresponding distance from point to target.
% Intuition should say there are two solutions for any point given two arms (curved down and curved up).
% A solution should mean there is 0 distance between point and target. That means our graph should have two zeros...
clear
close all
clc
tic

% SET UP CONSTANTS
TARGET = [-150,50];
L1 = 137.4;
L2 = 65.3;
RESOLUTION = 100;

% INITIALIZE ARRAYS
possible_angles = linspace(0,359,RESOLUTION);
distance = zeros(RESOLUTION,RESOLUTION);

% CALCULATE ALL POSSIBLE ORIENTATIONS OF THE ARM AND COMPARE TO DISTANCE
% FROM THE TARGET
for i = 1:RESOLUTION
    for j = 1:RESOLUTION
        x = L1 * cosd(possible_angles(i)) + L2 * cosd(possible_angles(j));
        y = L2 * sind(possible_angles(i)) + L2 * sind(possible_angles(j));
        arm = [x,y];
        distance(i,j) = norm(TARGET-arm);
    end
end

% PLOT
surf(possible_angles, possible_angles, distance);
colormap hot

toc