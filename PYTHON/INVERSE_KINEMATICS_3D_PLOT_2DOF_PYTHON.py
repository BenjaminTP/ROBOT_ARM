# Calculates all possible shoulder and elbow angles, then plots the corresponding distance from point to target.
# Intuition should say there are two solutions for any point given two arms (curved down and curved up).
# A solution should mean there is 0 distance between point and target. That means our graph should have two zeros...
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

TARGET = [-150,50]
L1 = 137.4
L2 = 65.3
RESOLUTION = 100

possible_angles = np.linspace(0,2*np.pi,RESOLUTION)
distance = np.empty([RESOLUTION, RESOLUTION])

for i in range(0,RESOLUTION):
    for j in range(0,RESOLUTION):
        x = L1 * np.cos(possible_angles[i]) + L2 * np.cos(possible_angles[j])
        y = L1 * np.sin(possible_angles[i]) + L2 * np.sin(possible_angles[j])
        distance[i][j] = np.linalg.norm([x-TARGET[0], y-TARGET[1]])

fig = plt.figure()
ax = fig.add_subplot(projection="3d")
ax.set_box_aspect([1,1,0.5])

plotted_angles = np.linspace(0,359,RESOLUTION)
plotted_angles1, plotted_angles2 = np.meshgrid(plotted_angles, plotted_angles)
ax.plot_surface(plotted_angles1, plotted_angles2, distance, cmap=cm.Spectral, rstride=2, cstride=2)

plt.show()
