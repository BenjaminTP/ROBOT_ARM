% Given a start and ending point, return a "number_of_arms" long list of shoulder and elbow angles to reach that end point in a straight line
close all
clear
clc
tic;

function [theta] = calc(target)
    global L1;
    global L2;
    theta = zeros(1,2);
    r_target  = norm(target);
    theta_target = atand(target(2)/target(1));
    
    theta(2) = acosd((L1^2 + L2^2 - r_target^2) / (2*L1*L2));
    theta(1) = asind(sind(theta(2)) / r_target * L2) + theta_target;
    theta(1) = real(theta(1));
    theta(2) = real(theta(2) - 180);
end

function plot_arm(target)
    global L1;
    global L2;
    sigma = calc(target);
    plot(target(1),target(2),"o", "color", "w");
    plot([0, L1*cosd(sigma(1))], [0, L1*sind(sigma(1))],"LineWidth", 2, "color", "r");
    plot([L1*cosd(sigma(1)), L1*cosd(sigma(1)) + L2*cosd(sigma(2)+sigma(1))], [L1*sind(sigma(1)), L1*sind(sigma(1)) + L2*sind(sigma(2)+sigma(1))], "color", "b")
end

function plot_line(start, ending, line)
    global number_of_arms;
    plot([start(1),ending(1)], [start(2), ending(2)], "color", "w");
    
    for i = 1:number_of_arms
        plot_arm(line(:,i));
    end
end

global L1; %#ok<*GVMIS>
global L2;
global number_of_arms;
L1 = 137.4;
L2 = 65.3;
xlim([-10,L1+L2+10]);
ylim([-10,L1+L2+10]);
start = [100,100];
ending = [150,90];
number_of_arms = 50;
hold on

line = [linspace(start(1), ending(1), number_of_arms);linspace(start(2), ending(2), number_of_arms)];

plot_line(start, ending, line);

hold off
    

sigma = zeros(2, number_of_arms);
for i = 1:number_of_arms
    sigma(:,i) = calc(line(:,i));
end
disp(sigma');

toc;