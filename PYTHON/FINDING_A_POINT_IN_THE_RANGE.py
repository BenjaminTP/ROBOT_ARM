# Given a target, return all possible combinations of angles to reach that point.
import SINGULAR_ARM_PLOTTING_AND_ANGLE_CALCULATION as arm
import matplotlib.pyplot as plt

def main():
    TARGET = [-240,-85]
    L1 = 137.4
    L2 = 85.8
    L3 = 103.3
    TABLE = -98.08
    
    for i in range(360):
        try:
            hand_theta = i

            sigma = arm.calculate_arm_angles(TARGET, L1, L2, L3, hand_theta, TABLE)
            
            print(*sigma, sep=", ", end=", ")
            print( f"hand_theta={sigma[0] - sigma[1] - 90 + sigma[2]}")   
        except ValueError:
            pass
    
    try:
        arm.plot_arm(L1, L2, L3, sigma)
        plt.xlim(-(L1+L2+L3+10), L1+L2+L3+10)
        plt.ylim(TABLE, L1+L2+L3+10)
        plt.show()
    except:
        print("Not a valid point")

if __name__ == "__main__":
    main()