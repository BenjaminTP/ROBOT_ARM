# Place a dot in all possible places the arm can reach. Reduce "multiplier" to increase resolution
import numpy as np
import matplotlib.pyplot as plt
import time
from SINGULAR_ARM_PLOTTING_AND_ANGLE_CALCULATION import calculate_arm_angles

def main():
    L1 = 137.4
    L2 = 85.8
    L3 = 103.3
    TABLE = -98.08
    max_arm_length = L1+L2+L3
    number_of_impossible_solutions = 0
    number_of_possible_solutions = 0
    multiplier = 10
    
    t0 = time.time()

    for i in range(int((-max_arm_length-10)/multiplier), int((max_arm_length+10)/multiplier)):
        for j in range(int((TABLE-10)/multiplier), int((max_arm_length+10)/multiplier)):
            for k in range(int(360/multiplier)):
                hand_theta = k*multiplier
                target = [i*multiplier,j*multiplier]
                try:
                    sigma = calculate_arm_angles(target,L1,L2,L3,hand_theta,TABLE)
                    point = target_from_angles(sigma,L1,L2,L3)
                    plt.plot(point[0], point[1], marker=".")
                    number_of_possible_solutions += 1
                except ValueError:      
                    number_of_impossible_solutions += 1

    t1 = time.time()

    print(f"Time to calculate positions = {t1-t0}s")

    plt.xlim(-(max_arm_length+10), max_arm_length+10)
    plt.ylim(TABLE, max_arm_length+10)
    plt.show()
    
    print(f"{number_of_possible_solutions/(number_of_possible_solutions+number_of_impossible_solutions)*100}% valid")


def target_from_angles(theta,L1,L2,L3):
    theta = np.radians(theta)

    x1 = L1*np.cos(theta[0])
    y1 = L1*np.sin(theta[0])

    x2 = L2*np.cos(theta[0]-theta[1])
    y2 = L2*np.sin(theta[0]-theta[1])

    x3 = L3*np.cos(theta[0]-theta[1]+theta[2]-np.pi/2)
    y3 = L3*np.sin(theta[0]-theta[1]+theta[2]-np.pi/2)

    return [x1+x2+x3,y1+y2+y3]


if __name__ == "__main__":
    main()