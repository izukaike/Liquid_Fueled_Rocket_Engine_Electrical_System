'''
Contributors: Izuka Ikedionwu
    *As you edit,add, delete* please add your name :)*

Date Created: 10/12/23

Description: 
    GUI for the liquid fueled rocket engine. Interacts with Arduino through serial port.
    Using PyQtGraph for Interface with LabView Capabilities. Object Oriented with the main 
    class being RealTimePlotApp the initialization sets up all components or "widgets" followed
    by functions that control valves, igniter, and updating plots

Work:
    - Plot time
    - data sync for plots
    - read 6 bytes and plot 6 bytes of data each loop 
    - Labview button for MAC
    - optimize update plot loopc
    - change update plots to read 6 bytes at a time and plot 5 bytes

Dependencies:
    pyqtgraph
    pyqt5
    numpy
    serial 
    subprocess

    
'''
import sys
import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton,QHBoxLayout,QGridLayout
import numpy as np
import serial
import time 
from timeit import default_timer as timer
import subprocess
import numpy
from pyqtgraph.Qt import QtGui
#---------------------------------------------------------------------------------------

#get com port // differs by cable
comport = input("Enter COMPORT:")

#Initialize Serial Communication with Arduino
arduino=serial.Serial() 	
arduino.baudrate=115200#comm speed // has to be synced with arduino
arduino.port= comport
#arduino.parity=serial.PARITY_ODD
#arduino.stopbits=serial.STOPBITS_ONE
arduino.bytesize=serial.EIGHTBITS
arduino.timeout = 0.01# this may be adjusted

#clears port buffer anyhting
if arduino.isOpen():
    arduino.close()
arduino.open()
if arduino.isOpen():
    print("Connected to Arduino")

#this delay is needed for arduino to set up
time.sleep(2)
#calibaration code

'''
sample_number = 150
sum = 0
arduino.reset_input_buffer()
arduino.write(b'8')
for i in range(sample_number):
    start = timer()
    #reads but data from arduino
    bits1 = arduino.read(bytes_read)
    #converts bytes to int
    data1 = int.from_bytes(bits1, "big")
    end = timer()
    sum += (end-start)
    #out of loop
    avg = sum/sample_number
arduino.write(b'7')
# We need a better way to calibrate
print("insert " + str(avg*5.1*1_000_000) + " for delayMicroSeconds in Arduino IDE")
arduino.close()
'''
start_sequence = input("Enter Start Code:")
#arduino.open()
#arduino.reset_input_buffer()
#Main class
class RealTimePlotApp(QMainWindow):
    #class variables
    pause = 1
    #Default Constructor
    def __init__(self):
        super().__init__()
        self.pause = 1

        # Initialize the main window
        self.setWindowTitle("Liquid Fueled Rocket Engine GUI:The Linda")
        self.setGeometry(150, 150, 1200, 800)
        self.setStyleSheet("QMainWindow { background-color: light gray}")

        # Create a central widget to contain the plots and buttons
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Create layout for the central widget
        layout = QGridLayout(central_widget)
        # Create three PlotWidgets
        self.plot_widgets = [pg.PlotWidget() for _ in range(3)]
        
        #adding labels to graph
        self.plot_widgets[0].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[0].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[0].getPlotItem().setLabel('top', text = '<font color="black">Pressure Transducer 1</font>')


        self.plot_widgets[1].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[1].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[1].getPlotItem().setLabel('top','<font color="black">Pressure Transducer 1</font>')


        self.plot_widgets[2].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[2].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[2].getPlotItem().setLabel('top','<font color="black">Pressure Transducer 1</font>')

        self.bar_item = [0,1,3]
        self.bar_item[0] = pg.BarGraphItem(x=[1], height=60, width=1, brush=(169,169,169))
        self.plot_widgets[0].addItem(self.bar_item[0])
        self.plot_widgets[0].setBackground('white')

        self.bar_item[1] = pg.BarGraphItem(x=[1], height=33, width=1, brush=(169,169,169))
        self.plot_widgets[1].addItem(self.bar_item[1])
        self.plot_widgets[1].setBackground('white')


        self.bar_item[2] = pg.BarGraphItem(x=[1], height=85, width=1, brush=(169,169,169))
        self.plot_widgets[2].addItem(self.bar_item[2])
        self.plot_widgets[2].setBackground('white')

        '''
        self.plot_widgets.setLabel('bottom', "Categories")
        self.plot_widgets.setLabel('left', "Values")
        '''

        #self.plot_widgets.getBarItem().setXRange(0, 4)
        #self.plot_widgets.setYRange(0, 100)
        
        #adding widgets to GUI
        number_of_plots = 3
        for i in range(number_of_plots):
            layout.addWidget(self.plot_widgets[i],0,i)
        

        # Create three  toggle buttons
        self.switch1 = QPushButton("VALVE 1: CLOSED")
        self.switch2 = QPushButton("VALVE 2: CLOSED")
        self.switch3 = QPushButton("IGNITER: OFF")

        #switch 1 setup
        self.switch1.setCheckable(True)
        self.switch1.clicked.connect(self.toggle_switch1)
        self.switch1.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        layout.addWidget(self.switch1,1,0,1,3)

        #switch 2 setup
        self.switch2.setCheckable(True)
        self.switch2.clicked.connect(self.toggle_switch2)
        self.switch2.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        layout.addWidget(self.switch2,2,0,1,3)

        #switch 3 setup
        self.switch3.setCheckable(True)
        self.switch3.clicked.connect(self.toggle_switch3)
        self.switch3.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        layout.addWidget(self.switch3,3,0,1,3)

        number_button = 4
        #initializes 2 buttons
        self.buttons = [QPushButton(f"Valve {i+1}") for i in range(number_button)]
        for i, button in enumerate(self.buttons):
            
            button.setStyleSheet("QPushButton { background-color:white; color: black; font-size: 20px; }")
            #button setup 
            j = 0
            if(i == 0):
                button.setText("START PLOT")
                button.clicked.connect(lambda: self.start_plot())  # Connect button click event
            if(i == 3):
                button.setText("ABORT")
                button.clicked.connect(lambda: self.abort())  # Connect button click event
                button.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
            if(i == 2):
                button.setText("LabView(Windows)")
                button.clicked.connect(lambda: self.runLabView_Windows())  # Connect button click event
            if(i == 1):
                button.setText("TEST")
                button.setStyleSheet("QPushButton { background-color: white; color: black; font-size: 18px; }")
                button.clicked.connect(lambda: self.runLabView_Windows())  # Connect button click event

        for i in range(number_button):
            if( i == 3):
                layout.addWidget(self.buttons[i],5,1,2,1)
            else:
                layout.addWidget(self.buttons[i])

            
            
        
        # Initialize data for the plots
        number_of_points = 4
        self.data = [np.zeros(number_of_points) for _ in range(3)]

        # Initialize the plots
        self.curves = [pw.plot(self.data[i]) for i, pw in enumerate(self.plot_widgets)]

        #set x limit and y limit
        for pw in self.plot_widgets:
            pw.setRange(xRange=[0,2], yRange=[0, 100])
        
        # Start a timer to update the plots at regular intervals 
        refresh_rate = 2 # 1ms
        self.timer = pg.QtCore.QTimer(self)
        self.timer.setInterval(refresh_rate)
        self.timer.timeout.connect(self.update_plots)
        self.timer.start(refresh_rate)  # Update every 100 ms

#end of setup code
#----------------------------------------------------------------------------------
#Class Functions
    def toggle_switch1(self):
        sender = self.sender()  # Get the button that emitted the signal
        if sender.isChecked():
            #important line that calls valve opening/closing function the rest is aesthetic
            self.control_valve1_open()
            sender.setText("VALVE 1: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.control_valve1_close()
            sender.setText("VALVE 1:  OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
    
    def toggle_switch2(self):
        sender = self.sender()  # Get the button that emitted the signal
        if sender.isChecked():
            #important line that calls valve opening/closing function the rest is aesthetic
            self.control_valve2_open()
            sender.setText("VALVE 2: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.control_valve2_close()
            sender.setText("VALVE 2: OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
    
    def toggle_switch3(self):
        sender = self.sender()  # Get the button that emitted the signal
        if sender.isChecked():
            #important line that calls valve opening/closing function the rest is aesthetic
            self.control_valve3_open()
            sender.setText("IGNITER: ON")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.control_valve3_close()
            sender.setText("IGNITER: OFF")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
    #function that communicates with Arduino 
    def control_valve1_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'1')
    
    def control_valve1_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'4')
        #arduino.close()
    
    def control_valve2_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'2')
        #arduino.close()

    def control_valve2_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'5')

    def control_valve3_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'3')

    def control_valve3_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'6')

    def runLabView_Windows(self):
        #this is function is operating system specific and pretty much goes into the command line and oepns the LabView app
        subprocess.run('cd C:\\Program Files (x86)\\National Instruments\\LabVIEW 2023 & dir & start LabVIEW', shell = True)
    
    def start_plot(self):
        #im not sure the best place to put it
        arduino.reset_input_buffer()
        arduino.write(b'9')
        arduino.timeout = None
        #arduino.close()
        #arduino.reset_input_buffer()
    
    def abort(self):
        #close valve 1
        arduino.write(b'4')
        time.sleep(0.0001)
        #close valve 2
        arduino.write(b'5')

    #main function that updates plots
    def update_plots(self):
        #print(arduino.in_waiting)
        bytes_read = 12
        bits1 = arduino.read(bytes_read)
        #print(bits1)
        '''
        if(bits1 != b''):
            print(bits1[0])
            print(bits1[1])
            print(bits1[2])
        '''
        #print(" byte1 = " + str(bits1[0]) + " byte2 = " + str(bits1[1]) + " byte3 = " + str(bits1[2]))
        #converts bytes to int
        off1 = 0
        off2 = 3
        off3 = 6
        off4 = 9
        if(bits1 != b''):
            #new_data = np.roll(self.data[0], 4)
            #new_data = np.append(self.data[0])
            #reads but data from arduino
            '''
            new_data[0] = bits1[off1]
            new_data[1] = bits1[off2]
            new_data[2] = bits1[off3]
            new_data[3] = bits1[off4]
            self.data[0] = new_data
            '''
            new_data = int((bits1[off1]+bits1[off2]+bits1[off3]+bits1[off4])/4)
            self.data[0] = new_data
            self.bar_item[0].setOpts(x=[1, 2, 3], height=new_data)
            #self.curves[0].setData(new_data)

            #new_data = np.roll(self.data[1], 4)
            #new_data = np.append(self.data[1])
            #reads but data from arduino
            '''
            new_data[0] = bits1[off1+1]
            new_data[1] = bits1[off2+1]
            new_data[2] = bits1[off3+1]
            new_data[3] = bits1[off4+1]
            self.data[1] = new_data
            '''
            #new 
            new_data = int((bits1[off1+1]+bits1[off2+1]+bits1[off3+1]+bits1[off4+1])/4)
            #print(new_data)
            self.data[1] = new_data
            self.bar_item[1].setOpts(x=[1, 2, 3], height=new_data)
            #new
            #self.curves[1].setData(new_data)

            #new_data = np.roll(self.data[2], 4)
            #ew_data = np.append(self.data[2])
            '''
            new_data[0] = bits1[off1+2]
            new_data[1] = bits1[off2+2]
            new_data[2] = bits1[off3+2]
            new_data[3] = bits1[off4+2]
            '''
            #reads but data from arduino
            new_data = int((bits1[off1+2]+bits1[off2+2]+bits1[off1+3]+bits1[off1+4])/4)
            self.data[2] = new_data
            self.bar_item[2].setOpts(x=[1, 2, 3], height=new_data)
            #self.curves[2].setData(new_data)
        
#---------------------Main Function-----------------------------------------
if __name__ == '__main__':
    #Application set up
    app = QApplication(sys.argv)
    window = RealTimePlotApp()
    arduino.reset_input_buffer()
    window.show()
    sys.exit(app.exec_())
   
