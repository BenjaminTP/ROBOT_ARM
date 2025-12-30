# This file contains all the math used for all the files.
# This will also return a single plot containing one arm given parameters.
import numpy as np
import matplotlib.pyplot as plt
import time
    
def main():
    TARGET = [150,100]
    TABLE = -98.08
    L1 = 137.4
    L2 = 85.8
    L3 = 103.3
    HAND_THETA = 0

    t0 = time.time()
    sigma = calculate_arm_angles(TARGET, L1, L2, L3, HAND_THETA, TABLE)
    t1 = time.time()
    print(t1-t0)

    plot_arm(L1, L2, L3, sigma)
    print(*sigma, sep=", ") 

    plt.xlim(-(L1+L2+L3+10), L1+L2+L3+10)
    plt.ylim(TABLE, L1+L2+L3+10)
    plt.show()


def calculate_arm_angles(target, L1, L2, L3, theta3, table):
    theta3 = np.radians(theta3)
    target = target.copy()
    target[0] -= L3*np.cos(theta3)
    target[1] -= L3*np.sin(theta3)
    theta = np.zeros(3)
    r_target = np.linalg.norm(target)
    theta_target = np.atan(target[1] / target[0])

    theta[1] = np.arccos((L1**2 + L2**2 - r_target**2) / (2*L1*L2))          # LAW OF COS
    theta[0] = np.arcsin(np.sin(theta[1]) / r_target * L2) + theta_target    # LAW OF SIN
    theta[0] = np.real(theta[0])
    theta[1] = np.real(theta[1] - np.pi) * -1
    theta[2] = np.real(theta[0] - theta[1] - theta3 - np.pi/2) * -1

    if target[0] <= 0:
        theta[0] += np.pi
        theta[2] += np.pi
    if theta[2] >= 2*np.pi:
        theta[2] -= 2*np.pi

    if not is_valid(theta,L1,L2,L3, table):
        raise ValueError

    return np.degrees(theta)


def is_valid(theta,L1,L2,L3,table):
    foot_length = 116.46
    foot_height = 15.2
    inner_hub_radius = 45
    inner_hub_height = 27

    if np.isnan(theta[0]) or np.isnan(theta[1]) or np.isnan(theta[2]):
        return False

    if theta[0] > np.pi or theta[0] < 0:
        return False
    
    if theta[1] >= np.pi or theta[1] < 0:
        return False
    
    if theta[2] > np.pi or theta[2] < 0:
        return False
    
    x1 = L1*np.cos(theta[0])
    y1 = L1*np.sin(theta[0])

    x2 = L2*np.cos(theta[0]-theta[1])
    y2 = L2*np.sin(theta[0]-theta[1])

    x3 = L3*np.cos(theta[0]-theta[1]+theta[2]-np.pi/2)
    y3 = L3*np.sin(theta[0]-theta[1]+theta[2]-np.pi/2)
    
    v1 = [0,0]
    v2 = [x1,y1]
    v3 = [x1+x2,y1+y2]
    v4 = [x1+x2+x3,y1+y2+y3]

    a = (v4[0]-v3[0])*(v3[1]-v1[1])-(v4[1]-v3[1])*(v3[0]-v1[0])
    b = (v4[0]-v3[0])*(v2[1]-v1[1])-(v4[1]-v3[1])*(v2[0]-v1[0])
    c = (v2[0]-v2[0])*(v3[1]-v1[1])-(v2[1]-v1[1])*(v3[0]-v1[0])

    if b == 0 or (a == 0 and b == 0):
        return False
    
    alpha = a/b
    beta = c/b

    if alpha >= -0.1 and alpha <= 1.1 and beta >= -0.1 and beta <= 1.1:
        return False
    
    if v4[1] <= table or v3[1] <= table or v2[1] <= table:
        return False
    
    if v4[0] <= foot_length and v4[0] >= -foot_length and v4[1] <= table + foot_height:
        return False
    
    if v4[0] <= inner_hub_radius and v4[0] >= -inner_hub_radius and v4[1] <= inner_hub_height:
        return False
    
    if v4[0] <= 0 and v4[0] >= -150 and v4[1] <= 10:
        return False

    return True


def plot_arm(L1, L2, L3, theta):
    theta = np.radians(theta)

    x1 = L1*np.cos(theta[0])
    y1 = L1*np.sin(theta[0])

    x2 = L2*np.cos(theta[0]-theta[1])
    y2 = L2*np.sin(theta[0]-theta[1])

    x3 = L3*np.cos(theta[0]-theta[1]+theta[2]-np.pi/2)
    y3 = L3*np.sin(theta[0]-theta[1]+theta[2]-np.pi/2)

    plt.plot([0,x1], [0,y1], marker=".", color="red")
    plt.plot([x1,x1+x2], [y1,y1+y2], marker=".", color="blue")
    plt.plot([x1+x2, x1+x2+x3], [y1+y2, y1+y2+y3], marker=".", color="magenta")


if __name__ == "__main__":
    main()

    