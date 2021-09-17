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
from ui.test import Ui_MainWindow as test_MainWindow
import concurrent.futures
import threading
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from sysinfo import *


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


class SearchSignals(QObject):
    progress = pyqtSignal(str)
    Valid_comport = pyqtSignal(str)
    finished = pyqtSignal()
    
class ComPortSearch(QThread):
    def __init__(self,Signals):
        super(ComPortSearch, self).__init__()
        self.Queue= queue.Queue(maxsize=10)
        self.signals=Signals
    def run(self):
        self.search_comport_function()
        
    def search_comport_function(self):
        self.signals.progress.emit("start of the program")
        ports = serial.tools.list_ports.comports()
        list_of_ports=[x[0] for x in sorted(ports)]
        port_index=0
        while(True):
            self.signals.progress.emit(f"check COM port: {list_of_ports[port_index]}")
            self.init_uart(list_of_ports[port_index])
            try:
                print(f" iiiii -->{list_of_ports[port_index]}")
                response=self.Queue.get(block=True, timeout=7)
                if 'Hi' in response:
                    init_done=True
                    self.signals.Valid_comport.emit(list_of_ports[port_index])
                    break
                print(f"exeption andakhtam")
                raise Exception
            except:
                port_index+=1
                self.uart_reciver.stop()
                self.serial.close()
                if port_index==len(list_of_ports):
                    self.signals.progress.emit("=============FAIL===============")
                    self.signals.progress.emit(" make sure your device is connected and pwr on")
                    self.signals.progress.emit("================================")
                    init_done=False
                    break
        print(f"ghable init doen chekc")
        if init_done:
            self.signals.progress.emit("Port Finded")
            if (self.write_uart_wait_for_response("$HI!",-1,"who")):
                print("send hi is fine and he say who")
            if (self.write_uart_wait_for_response(f"$WHO!{getSystemInfo()}",-1,"$SES!")):
                print("He sends Who")
            self.signals.finished.emit()
        print("--end--")
            
    def init_uart(self,COMPORT):
        self.serial = serial.Serial(COMPORT, 115200, timeout=0)
        self.uart_reciver=UartReciever(self.serial,self.Queue)
        self.uart_reciver.setTerminationEnabled(True)
        self.uart_reciver.start()

    def write_uart_wait_for_response(self,data,number,resp):
        self.serial.write(data.encode())
        response=False
        tries=0
        while(not response):
            try:
                response=self.Queue.get(block=True, timeout=3)
                if resp in response:
                    return True
                break
            except:
                self.send_log(f"no response :{tries+1}")
                tries=tries+1
                if tries>number:
                    response=None
                    break
                else:
                    self.send_log('send again ...')
                    self.serial.write(data.encode())
        return False
        
    def stop(self):
        self.serial.close()
        self.uart_reciver.stop()
        self.terminate()
