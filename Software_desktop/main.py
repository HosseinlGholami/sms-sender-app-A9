from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QThread
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QWidget
import numpy as np
import time
import serial
import serial.tools.list_ports
import queue
from PyQt5 import QtWidgets,QtGui
from PyQt5.QtCore import Qt
import sys
from ui.introUI import Ui_MainWindow as introUI_MainWindow

class UartReciever(QThread):
    def __init__(self,serial,queue_R):
        super(UartReciever, self).__init__()
        self.ser=serial
        self.run_read=True
        self.Queue_R=queue_R
    def run(self):
        while(self.run_read):
            content=self.ser.readlines()
            if (content):
                content=content[0].decode()
                print(content)
                self.Queue_R.put(content)

    def stop(self):
        self.terminate()


class RunDesignerGUI():
    def __init__(self):
        app = QtWidgets.QApplication(sys.argv)
        #Windows
        self.introWindow = QtWidgets.QMainWindow()
        #SetupUIs
        self.intui = introUI_MainWindow()
        self.intui.setupUi(self.introWindow )
        
        self.widget_action()
        self.Queue= queue.Queue(maxsize=10)
        
        self.introWindow.show()
        sys.exit(app.exec_())
        
    def widget_action(self):
        self.intui.searchButton.clicked.connect(self.search_comport_function)
    def search_comport_function(self):
        print("start of the program")
        ports = serial.tools.list_ports.comports()
        list_of_ports=[x[0] for x in sorted(ports)]
        port_index=0
        while(True):
            self.init_uart(list_of_ports[port_index])
            try:
                response=self.Queue.get(block=True, timeout=10)
                if 'Hi' in response:
                    init_done=True
                    break
                raise Exception
            except:
                port_index+=1
                self.uart_reciver.stop()
                self.serial.close()
                if port_index==len(list_of_ports):
                    print("make sure your device is connected")
                    init_done=False
                    break
        if init_done:
            print("continue of the program")

    def init_uart(self,COMPORT):
        self.serial = serial.Serial(COMPORT, 115200, timeout=0)
        self.uart_reciver=UartReciever(self.serial,self.Queue)
        self.uart_reciver.setTerminationEnabled(True)
        self.uart_reciver.start()
        
                    
            

        
    
if __name__ == "__main__":
    RunDesignerGUI()
    