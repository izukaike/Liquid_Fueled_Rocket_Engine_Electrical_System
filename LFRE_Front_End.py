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
    openpyxl

    
'''
import sys
import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton,QHBoxLayout,QGridLayout,QSlider
from PyQt5 import QtCore
from PyQt5.QtCore import Qt, QThread, pyqtSignal
from PyQt5 import uic
import numpy as np
import serial
import time 
from timeit import default_timer as timer
import subprocess
import numpy
from pyqtgraph.Qt import QtGui
import openpyxl
from openpyxl.chart import ScatterChart, Reference
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


#opening excel file premature for less wait time in the future
dataframe = openpyxl.load_workbook("Recording_File.xlsx")
sheet = dataframe.active
sheet.delete_rows(2,sheet.max_row)
dataframe.save("C:\\Users\\izuka\\Documents\\A_New_Ard_Proj\\Recording_File.xlsx")
dataframe = openpyxl.load_workbook("Recording_File.xlsx")
sheet = dataframe.active

txt_file = open('data.txt', 'w+')
txt_file.write("")
txt_file.close()
txt_file = open("data.txt", 'w')
if(txt_file):
    print("1: Raw File Ready")
else:
    print("1: Raw File Not Ready")
if(dataframe):
    print("2: Record File Available")
else:
    print("2: Record File Not Available")




#clears port buffer anyhting
if arduino.isOpen():
    arduino.close()
arduino.open()
if arduino.isOpen():
    print("3: Connected to Arduino")
else:
    print("3: Not Connected to Arduino")

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
        self.arm1 = 0
        self.arm2 = 0
        self.armed = 0
        self.is_on1 = True
        self.is_on2 = True
        self.is_on3 = True
        self.is_on4 = True
        self.is_on5 = True
        self.is_on6 = True
        self.is_on7 = True
        self.start_recording = 0
        self.i = 0
        self.j = 0
        self.v1 = 0
        self.v2 = 0
        self.v3 = 0
        self.i1= 0
        self.data_max = 100
        self.timer = 0
        self.bytes_read = 12
        self.new_data1 = 0
        self.new_data2 = 0
        self.new_data3 = 0
        self.new_data4 = 0
        self.plot = 0
        self.thread = QThread()
        self.worker = ()
        


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
        self.plot_widgets = [pg.PlotWidget() for _ in range(4)]
        
        #adding labels to graph
        self.plot_widgets[0].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[0].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[0].getPlotItem().setLabel('top', text = '<font color="black">Pressure Transducer 1</font>')


        self.plot_widgets[1].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[1].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[1].getPlotItem().setLabel('top','<font color="black">Pressure Transducer 2</font>')


        self.plot_widgets[2].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[2].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[2].getPlotItem().setLabel('top','<font color="black">Pressure Transducer 3</font>')

        self.plot_widgets[3].getPlotItem().setLabel('left',text = '<font color="black">Psi</font>')
        #self.plot_widgets[0].getPlotItem().setLabel('bottom', 'Time')
        self.plot_widgets[3].getPlotItem().setLabel('top', text = '<font color="black">Pressure Transducer 4</font>')

        self.bar_item = [0,1,3,5]
        self.bar_item[0] = pg.BarGraphItem(x=[1], height=60, width=1, brush=(169,169,169))
        self.plot_widgets[0].addItem(self.bar_item[0])
        self.plot_widgets[0].setBackground('white')

        self.bar_item[1] = pg.BarGraphItem(x=[1], height=33, width=1, brush=(169,169,169))
        self.plot_widgets[1].addItem(self.bar_item[1])
        self.plot_widgets[1].setBackground('white')


        self.bar_item[2] = pg.BarGraphItem(x=[1], height=85, width=1, brush=(169,169,169))
        self.plot_widgets[2].addItem(self.bar_item[2])
        self.plot_widgets[2].setBackground('white')

        self.bar_item[3] = pg.BarGraphItem(x=[1], height=60, width=1, brush=(169,169,169))
        self.plot_widgets[3].addItem(self.bar_item[3])
        self.plot_widgets[3].setBackground('white')


        '''
        self.plot_widgets.setLabel('bottom', "Categories")
        self.plot_widgets.setLabel('left', "Values")
        '''

        #self.plot_widgets.setYRange(0, 500)
        
        #adding widgets to GUI
        number_of_plots = 4
        for i in range(number_of_plots):
            layout.addWidget(self.plot_widgets[i],0,i)
        

        # Create three  toggle buttons
        self.switch1 = QPushButton("VALVE 1: CLOSED")
        self.switch2 = QPushButton("VALVE 2: CLOSED")
        self.switch3 = QPushButton("VALVE 3: CLOSED")
        self.switch4 = QPushButton("IGNITER: OFF")
        self.switch5 = QPushButton("ARMED1: OFF")
        self.switch6 = QPushButton("ARMED2: OFF")
        self.switch7 = QPushButton("RECORDING: OFF")

        #switch 1 setup
        self.switch1.setCheckable(True)
        self.switch1.clicked.connect(self.toggle_switch1)
        self.switch1.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch1)

        #switch 2 setup
        self.switch2.setCheckable(True)
        self.switch2.clicked.connect(self.toggle_switch2)
        self.switch2.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch2)

        #switch 3 setup
        #self.switch3.setCheckable(True)
        #self.switch3.clicked.connect(self.toggle_switch3)
        #self.switch3.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch3)

        self.switch4.setCheckable(True)
        self.switch4.clicked.connect(self.toggle_switch4)
        self.switch4.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch4)

        #switch 5 setup
        self.switch5.setCheckable(True)
        self.switch5.clicked.connect(self.toggle_switch5)
        self.switch5.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch5)

        #switch 6 setup
        self.switch6.setCheckable(True)
        self.switch6.clicked.connect(self.toggle_switch6)
        self.switch6.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch6)

        self.switch7.setCheckable(True)
        self.switch7.clicked.connect(self.toggle_switch7)
        self.switch7.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 20px; }")
        #layout.addWidget(self.switch7)

        layout.addWidget(self.switch1)
        layout.addWidget(self.switch2)
        layout.addWidget(self.switch4)
        layout.addWidget(self.switch7)
        layout.addWidget(self.switch5)
        layout.addWidget(self.switch6)


        number_button = 5
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
                button.setText("TEST PROCEDURE: NOT ARMED")
                button.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
                button.clicked.connect(lambda: self.start_test())  # Connect button click event
            if(i == 4):
                button.setText("CALIBRATE")
                button.setStyleSheet("QPushButton { background-color: white; color: black; font-size: 18px; }")
                button.clicked.connect(lambda: self.start_calibration())  # Connect button click event
            
        layout.addWidget(self.buttons[1])
        for i in range(number_button):
            #if( i == 4):
            if i != 1:
                layout.addWidget(self.buttons[i])
            '''
            if( i == 3):
                layout.addWidget(self.buttons[i],4,1,1,1)
            else:
                layout.addWidget(self.buttons[i])
            '''

            
            
        
        # Initialize data for the plots
        number_of_points = 4
        self.data = [np.zeros(number_of_points) for _ in range(4)]

        # Initialize the plots
        self.curves = [pw.plot(self.data[i]) for i, pw in enumerate(self.plot_widgets)]

        #set x limit and y limit
        for pw in self.plot_widgets:
            pw.setRange(xRange=[0,2], yRange=[0, 100])
        
        # Start a timer to update the plots at regular intervals 
        refresh_rate = 0 # 1ms
        self.timer = pg.QtCore.QTimer(self)
        self.timer.setInterval(refresh_rate)
        self.timer.timeout.connect(self.update_plots)
        self.timer.start(refresh_rate)  # Update every 100 ms

#end of setup code
#----------------------------------------------------------------------------------
#Class Functions
    def toggle_switch1(self):
        sender = self.sender()  # Get the button that emitted the signal
        sender.isChecked = 0
        self.is_on1 = not self.is_on1
        if self.is_on1:
            self.v1 = 0
            self.control_valve1_open()
            self.switch1.setText("VALVE1: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.v1 = self.data_max
            self.control_valve1_close()
            sender.setText("VALVE 1:  OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        '''
        if sender.isChecked():
            #important line that calls valve opening/closing function the rest is aesthetic
            self.control_valve1_open()
            self.setText("VALVE1:CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.control_valve1_close()
            sender.setText("VALVE 1:  OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        '''
    
    def toggle_switch2(self):
        sender = self.sender()  # Get the button that emitted the signal
        self.is_on2 = not self.is_on2
        if self.is_on2:
            self.v2 = 0
            self.control_valve2_open()
            self.switch2.setText("VALVE2: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.v2 = self.data_max
            self.control_valve2_close()
            sender.setText("VALVE 2:  OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")

        '''
        if sender.isChecked():
            #important line that calls valve opening/closing function the rest is aesthetic
            self.control_valve2_open()
            sender.setText("VALVE 2: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.control_valve2_close()
            sender.setText("VALVE 2: OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        '''
    '''
    def toggle_switch3(self):
        sender = self.sender()  # Get the button that emitted the signal

        self.is_on3 = not self.is_on3
        if self.is_on3:
            self.v3 = 0
            self.control_valve3_open()
            self.switch3.setText("VALVE 3: CLOSED")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.v3 = self.data_max
            self.control_valve3_close()
            self.switch3.setText("VALVE 3:  OPEN")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
    '''
    def toggle_switch4(self):
        sender = self.sender()  # Get the button that emitted the signal
        self.is_on4 = not self.is_on4
        if self.is_on4:
            self.i1 = 0
            self.control_igniter_close()
            self.switch4.setText("IGNITER: OFF")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
        else:
            self.i1 = self.data_max
            self.control_igniter_open()
            self.switch4.setText("IGNITER: ON")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")

    def toggle_switch5(self):
        sender = self.sender()  # Get the button that emitted the signal
        self.is_on5 = not self.is_on5
        if self.is_on5:
            self.switch5.setText("ARMED1: OFF")
            self.arm1 = 0
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        else:
            self.switch5.setText("ARMED1: ON")
            self.arm1 = 1
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
    
    def toggle_switch6(self):
        sender = self.sender()  # Get the button that emitted the signal
        self.is_on6 = not self.is_on6
        if self.is_on6:
            self.switch6.setText("ARMED2: OFF")
            self.arm2 = 0
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        else:
            self.switch6.setText("ARMED2: ON")
            self.arm2= 1
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
            if self.arm1 and self.arm2:
                self.test_armed = 1
                self.buttons[1].setText("TEST PROCEDURE: ARMED")
                self.buttons[1].setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
            else:
                self.test_armed = 0
                self.buttons[1].setText("TEST PROCEDURE: NOT ARMED")
                self.buttons[1].setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")

    def toggle_switch7(self):
        sender = self.sender()  # Get the button that emitted the signal
        self.is_on7= not self.is_on7
        if self.is_on7:
            self.start_recording = 0
            self.switch7.setText("RECORDING: OFF")
            sender.setStyleSheet("QPushButton { background-color: red; color: black; font-size: 18px; }")
        else:
            #add recording function 
            self.start_recording = 1
            self.switch7.setText("RECORDING: ON")
            sender.setStyleSheet("QPushButton { background-color: green; color: black; font-size: 18px; }")
    #function that communicates with Arduino 
    def control_valve1_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'1')
        time.sleep(0.1)
    
    def control_valve1_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'4')
        arduino.flush()
        time.sleep(0.1)

        #arduino.close()
    
    def control_valve2_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'2')
        arduino.flush()
        time.sleep(0.1)
        #arduino.close()

    def control_valve2_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'5')
        arduino.flush()
        time.sleep(0.1)

    def control_valve3_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'3')
        arduino.flush()
        time.sleep(0.1)

    def control_valve3_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        arduino.write(b'6')
        arduino.flush()
        time.sleep(0.1)

    def control_igniter_open(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        self.thread[0] = GenericThread(parent=None,index = 1)
        for i in range(25):
            arduino.write(b'7')
            time.sleep(0.1)
        arduino.flush()
        
    
    def control_igniter_close(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        for i in range(25):
            arduino.write(b'8')
            time.sleep(0.1)
        arduino.flush()

    def start_test(self):
        #valve 1 sends 1 to arduino and arduino reads 1 and knows to activate valve1
        if(self.test_armed == 1):
            arduino.write(b'!')
            arduino.flush()

    def start_calibration(self):
        arduino.write(b'@')
        arduino.flush()

    def runLabView_Windows(self):
        #this is function is operating system specific and pretty much goes into the command line and oepns the LabView app
        subprocess.run('cd C:\\Program Files (x86)\\National Instruments\\LabVIEW 2023 & dir & start LabVIEW', shell = True)
    
    def start_plot(self):
        #im not sure the best place to put it
        arduino.reset_input_buffer()
        arduino.write(b'9')
        arduino.flush()
        arduino.timeout = None
        self.timer = time.time()
        #arduino.close()
        #arduino.reset_input_buffer()
    def abort(self):
        #close valve 1
        arduino.write(b'4')
        time.sleep(0.1)
        #close valve 2
        arduino.write(b'5')

    #main function that updates plots
    def update_plots(self):
        #start = time.time()
        #print(arduino.in_waiting)
        #tempt = time.time()
        bits1 = arduino.read(self.bytes_read)
        #tempt = time.time()
        off1 = 0
        off2 = 4
        off3 = 8
        if(bits1 != b''):
            self.new_data1 = int((bits1[off1]+bits1[off2]+bits1[off3])/3)
            #self.data[0] = new_data1
            self.bar_item[0].setOpts(x=[1, 2, 3], height=self.new_data1)
            #self.curves[0].setData(new_data)

            #new 
            self.new_data2 = int((bits1[off1+1]+bits1[off2+1]+bits1[off3+1])/3)
            #print(new_data)
            #self.data[1] = new_data2
            self.bar_item[1].setOpts(x=[1, 2, 3], height=self.new_data2)
            #new
            
            #reads but data from arduino
            self.new_data3 = int((bits1[off1+2]+bits1[off2+2]+bits1[off3+2])/3)
            #self.data[2] = new_data3
            self.bar_item[2].setOpts(x=[1, 2, 3], height=self.new_data3)

            self.new_data4 = int((bits1[off1+3]+bits1[off2+3]+bits1[off3+3])/3)
            #self.data[2] = new_data3
            self.bar_item[3].setOpts(x=[1, 2, 3], height=self.new_data4)
            #self.curves[2].setData(new_data)
        #temptt = time.time()
        #print("plot code = " + str((temptt-tempt)) )
            
        #tempt = time.time()
            if( self.start_recording == 1):
                txt_file.write(str(self.new_data1)+"\n"+str(self.new_data2)+"\n"+str(self.new_data3)+"\n"+str(self.new_data4)+"\n"+str(self.v1)+"\n")
                #txt_file.write(str(self.new_data2)+"\n")+str(self.new_data3)+"\n"
                #txt_file.write(str(self.new_data3)+"\n")
                #txt_file.write(str(self.v1)+"\n")
                txt_file.write(str(self.v2)+"\n"+str(self.i1)+"\n"+str(time.time()-self.timer)+"\n")
                #txt_file.write(str(self.v3)+"\n")
                #txt_file.write(str(self.i1)+"\n")
                #txt_file.write(str(time.time()-self.timer)+"\n")
                self.i += 1
            #temptt = time.time()
            #print("record code = " + str((temptt-tempt)))
            #end = time.time()
            #print((end-start)*1000)


        
#---------------------Main Function-----------------------------------------
if __name__ == '__main__':
    #Application set up
    app = QApplication(sys.argv)
    window = RealTimePlotApp()
    arduino.reset_input_buffer()
    window.show()
    app.exec_()
    txt_file.close()
    txt_file = open("data.txt",'r')
    count = 0
    for i in range(window.i):
        for j in range(8):
            rec_data1 = sheet.cell(row = i+2, column = j+1)
            rec_data1.value = str("=" + txt_file.readline()) 
        count += 1

    dataframe.save("C:\\Users\\izuka\\Documents\\A_New_Ard_Proj\\Recording_File.xlsx")
    print("Data Is Saved")
    if window.start_recording == 1:
        subprocess.run('C:\\Users\\izuka\Documents\\A_New_Ard_Proj\\Recording_File.xlsx', shell = True)
    sys.exit()

   
