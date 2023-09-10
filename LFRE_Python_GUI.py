'''
    Engineer: Izuka Ikedionwu
    Date Created: 9/10/23
    
'''

import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
from itertools import count

# Create a Tkinter window
root = tk.Tk()
root.title("LFRE Test GUI")

# Create a frame for the plots
frame = ttk.Frame(root)
frame.pack(fill=tk.BOTH, expand=True)

# Create three Figures and Axes for the plots

fig1, (ax1, ax2, ax3) = plt.subplots(1,3)
#
ax1.set_xlabel("Pressure Transducer 1")  
ax2.set_xlabel("Pressure Transducer 2")
ax3.set_xlabel("Pressure Transducer 3")
#
canvas1 = FigureCanvasTkAgg(fig1, master=frame)

canvas_widget1 = canvas1.get_tk_widget()

canvas_widget1.pack(fill=tk.BOTH, expand=True)


# Initialize data for the plots
x_max = 10  # Maximum number of data points to display
num_points = 10 # Number of initial data points
max_data = 5
x_values1 = list(range(num_points))
x_values2 = list(range(num_points))
x_values3 = list(range(num_points))
y_values1 = [np.random.randint(0, max_data) for _ in range(num_points)]
y_values2 = [np.random.randint(0, max_data) for _ in range(num_points)]
y_values3 = [np.random.randint(0, max_data) for _ in range(num_points)]

# Functions to update the plots
def update_plots():
    # we have to get rid of these global variables
    global x_values1, x_values2, x_values3, y_values1, y_values2, y_values3
    
    max_data = 6
    x_values1.append(x_values1[-1] + 1)
    x_values2.append(x_values2[-1] + 1)
    x_values3.append(x_values3[-1] + 1)

    y_values1.append(np.random.randint(0, max_data))
    y_values2.append(np.random.randint(0, max_data))
    y_values3.append(np.random.randint(0, max_data))

    if len(x_values1) > x_max:
        x_values1.pop(0)
        y_values1.pop(0)
        x_values2.pop(0)
        y_values2.pop(0)
        x_values3.pop(0)
        y_values3.pop(0)
    
    # uncomment to get rod of rainbow and slower
    #ax1.clear()
    #ax2.clear()
    #ax3.clear()

    ax1.plot(x_values1, y_values1)
    ax2.plot(x_values2, y_values2)
    ax3.plot(x_values3, y_values3)

    
    

    ax1.set_xlim(max(0, max(x_values1) - x_max), max(x_values1)+5)
    ax2.set_xlim(max(0, max(x_values2) - x_max), max(x_values2)+5)
    ax3.set_xlim(max(0, max(x_values3) - x_max), max(x_values3)+5)

    ax1.set_ylim(0, 5)
    ax2.set_ylim(0, 5)
    ax3.set_ylim(0, 5)

    canvas1.draw()

    root.after(1, update_plots)  # Update every 1 second

# Create a button to start updating the plots
start_button = ttk.Button(root, text="Start Plotting", command=update_plots)
start_button.pack()

# Create a button to quit the application
quit_button = ttk.Button(root, text="Quit", command=root.quit)
quit_button.pack()

#add valve and bs button then I will excel reading and writing code probably ina another file but if it can in the file
# then lets make it happen

root.mainloop()