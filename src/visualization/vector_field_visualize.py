import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider

default_line_angle = np.pi / 6
default_smoothness = 1

# Define the vector field function
def angle(x, y, line_angle, smoothness):
    rotated_y = np.cos(-line_angle) * y + np.sin(-line_angle) * x
    return line_angle + np.arctan(-rotated_y / smoothness)

min_xy = -10
max_xy = 10

# Define the range of the vector field
x = np.linspace(min_xy, max_xy, 51)
y = np.linspace(min_xy, max_xy, 51)

# Create a grid of coordinates for the vector field
X, Y = np.meshgrid(x, y)

# Compute the angles of the vectors
angles = angle(X, Y, default_line_angle, default_smoothness)

# Compute the x and y components of the vectors
U = np.cos(angles)
V = np.sin(angles)

# Create the plot with arrows
fig, (ax, line_angle_slider_ax, smoothness_slider_ax) = plt.subplots(
    nrows=3, figsize=(8, 9), gridspec_kw={'height_ratios': [10, 1, 1], 'hspace': 0.05}
)
line, = ax.plot([
    10 * min_xy * np.cos(default_line_angle),
    10 * max_xy * np.cos(default_line_angle)
  ], [
    10 * min_xy * np.sin(default_line_angle),
    10 * max_xy * np.sin(default_line_angle)
  ], 'r')
quiver = ax.quiver(X, Y, U, V, scale=45)

# Set the limits and labels of the plot
ax.set_xlim([min_xy, max_xy])
ax.set_ylim([min_xy, max_xy])
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_title('Vector Field')

# Define the sliders
line_angle_slider = Slider(line_angle_slider_ax, 'Line Angle', -np.pi, np.pi, valinit=default_line_angle)
smoothness_slider = Slider(smoothness_slider_ax, 'Smoothness', 0.2, 5, valinit=default_smoothness)

# Function to update the plot when the slider is changed
def update(val):
    line.set_data([[
        10 * min_xy * np.cos(line_angle_slider.val),
        10 * max_xy * np.cos(line_angle_slider.val)
      ], [
        10 * min_xy * np.sin(line_angle_slider.val),
        10 * max_xy * np.sin(line_angle_slider.val)
      ]])
    angles = angle(X, Y, line_angle_slider.val, smoothness_slider.val)  
    quiver.set_UVC(np.cos(angles), np.sin(angles))
    fig.canvas.draw_idle()

# Connect the sliders to the update function
line_angle_slider.on_changed(update)
smoothness_slider.on_changed(update)

# Adjust the margins of the overall plot
fig.subplots_adjust(top=0.95, bottom=0.05, left=0.2, right=0.8)

# Set the aspect ratio of the vector field plot to 1:1
ax.set_aspect('equal')

# Show the plot
plt.show()