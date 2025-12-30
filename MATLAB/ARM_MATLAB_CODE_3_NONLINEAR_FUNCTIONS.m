% Given a start point, end point, and a curve, return a list of shoulder and elbow angles to follow that curve.
% Multiple curves can also be chained together to create shapes made with mathematical functions.
close all
clear
clc
tic

% CALCULATE ANGLES FOR A GIVEN TARGET
function [theta] = calc(target)
    global L1;
    global L2;
    theta = zeros(1,2);
    r_target  = norm(target);
    theta_target = atand(target(2)/target(1));
    
    theta(2) = acosd((L1^2 + L2^2 - r_target^2) / (2*L1*L2));
    theta(1) = asind(sind(theta(2)) / r_target * L2) + theta_target;
    theta(1) = real(theta(1));
    theta(2) = real(theta(2) - 180)*-1;
end

% PLOT ONE ARM FOR A GIVEN TARGET
function plot_arm(target)
    global L1;
    global L2;
    sigma = calc(target);
    plot(target(1),target(2),"o", "color", "w");
    plot([0, L1*cosd(sigma(1))], [0, L1*sind(sigma(1))],"LineWidth", 2, "color", "r");
    plot([L1*cosd(sigma(1)), L1*cosd(sigma(1)) + L2*cosd(sigma(1)-sigma(2))], [L1*sind(sigma(1)), L1*sind(sigma(1)) + L2*sind(sigma(1)-sigma(2))], "color", "b")
end

% PLOT ARMS FOR EVERY POINT ON A GIVEN CURVE
function plot_curve(line, number_of_arms)
    plot(line(1,:), line(2,:), "w")
    for i = 1:number_of_arms
        plot_arm(line(:,i));
    end
end

% CALCULATE A LIST OF ANGLES FOR A GIVEN CURVE
function [sigma] = calc_sigma(line, number_of_arms)
    sigma = zeros(2, number_of_arms);
    for i = 1:number_of_arms
        sigma(:,i) = calc(line(:,i));
    end
end

% SETUP
global L1; %#ok<*GVMIS>
global L2;
L1 = 137.4;
L2 = 65.3;
xlim([-10,L1+L2+10]);
ylim([-10,L1+L2+10]);
pbaspect([1 1 1]);
hold on

% DECLARE STARTING, ENDING, NUMBER_OF_ARMS(ACCURACY)
START1 = 50;
END1 = 100;
NUMBER_OF_ARMS1 = 15;

START2 = 100;
END2 = 150;
NUMBER_OF_ARMS2 = 15;

START3 = 150;
END3 = 100;
NUMBER_OF_ARMS3 = 10;

START4 = 100;
END4 = 50;
NUMBER_OF_ARMS4 = 10;

% DECLARE X VARIABLE AND CREATE Y AS A FUNCTION OF X
x1 = linspace(START1, END1, NUMBER_OF_ARMS1);
y1 = real(sqrt(25^2-(x1-75).^2)+100);

x2 = linspace(START2, END2, NUMBER_OF_ARMS2);
y2 = real(sqrt(25^2-(x2-125).^2)+100);

x3 = linspace(START3, END3, NUMBER_OF_ARMS3);
y3 = x3-50;

x4 = linspace(START4, END4, NUMBER_OF_ARMS4);
y4 = -x4+150;

% CREATE LINE (LIST OF X,Y COORDS) AND PLOT THE CURVE AND ARMS
line1 = [x1;y1];
plot_curve(line1, NUMBER_OF_ARMS1);

line2 = [x2;y2];
plot_curve(line2, NUMBER_OF_ARMS2);

line3 = [x3;y3];
plot_curve(line3, NUMBER_OF_ARMS3);

line4 = [x4;y4];
plot_curve(line4, NUMBER_OF_ARMS4);

% FIND ANGLES
sigma1 = calc_sigma(line1, NUMBER_OF_ARMS1);
sigma2 = calc_sigma(line2, NUMBER_OF_ARMS2);
sigma3 = calc_sigma(line3, NUMBER_OF_ARMS3);
sigma4 = calc_sigma(line4, NUMBER_OF_ARMS4);

hold off

% DISPLAY ANGLES
disp(sigma1');
disp(sigma2');
disp(sigma3');
disp(sigma4');

toc