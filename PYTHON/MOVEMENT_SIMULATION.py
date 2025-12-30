# IMPORTANT FILE! By using different keys, control a simulation of the robot arm created with matplotlib.
import matplotlib.pyplot as plt
import SINGULAR_ARM_PLOTTING_AND_ANGLE_CALCULATION as arm

def main():
    target = [115,100]
    L1 = 137.4
    L2 = 85.8
    L3 = 103.3
    global TABLE
    TABLE = -98.08
    hand_theta = 0

    plt.ion()
    fig, ax = plt.subplots()

    ax.set_xlim(-(L1+L2+L3+10), L1+L2+L3+10)
    ax.set_ylim(TABLE, L1+L2+L3+10)
    ax.set_aspect(1)

    sigma = arm.calculate_arm_angles(target,L1,L2,L3,hand_theta, TABLE)
    arm.plot_arm(L1,L2,L3,sigma)
    plt.draw()

    def on_key(event):
        nonlocal target, hand_theta

        xyz_change = 5
        hand_theta_change = 5

        key = event.key
       
        match key:
            case "up":
                target[1] += xyz_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    target[1] -= xyz_change
                    replot(target, L1, L2, L3, hand_theta, TABLE)

            case "down":
                target[1] -= xyz_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    target[1] += xyz_change
                    replot(target, L1, L2, L3, hand_theta, TABLE)

            case "left":
                target[0] -= xyz_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    target[0] += xyz_change
                    replot(target, L1, L2, L3, hand_theta, TABLE)

            case "right":
                target[0] += xyz_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    target[0] -= xyz_change
                    replot(target, L1, L2, L3, hand_theta, TABLE)

            case ",":
                hand_theta += hand_theta_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    hand_theta -= hand_theta_change

            case ".":
                hand_theta -= hand_theta_change
                try:
                    replot(target, L1, L2, L3, hand_theta, TABLE)
                except ValueError:
                    hand_theta += hand_theta_change

            case "r":
                target = [115,100]
                hand_theta = 0
                replot(target, L1, L2, L3, hand_theta, TABLE)

            case _:
                return
            
        plt.draw()

    fig.canvas.mpl_connect("key_press_event", on_key)
    plt.show(block=True)

def replot(target,L1,L2,L3,hand_theta,table):
        sigma = arm.calculate_arm_angles(target,L1,L2,L3,hand_theta,table)
        print(f"sigma={sigma}, hand_theta={hand_theta}, target={target}")
        plt.cla()
        plt.xlim(-(L1+L2+L3+10), L1+L2+L3+10)
        plt.ylim(TABLE, L1+L2+L3+10)
        arm.plot_arm(L1,L2,L3,sigma)
                

if __name__ == "__main__":
    main()
