from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QThread
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QWidget
import numpy as np
import time
import serial
import queue
from PyQt5 import QtWidgets,QtGui
from PyQt5.QtCore import Qt
import sys


class UartReciever(QThread):
    def __init__(self,serial,queue_R,queue_ok):
        super(UartReciever, self).__init__()
        self.ser=serial
        self.run_read=True
        self.Queue_R=queue_R
        self.Queue_OK=queue_ok
    def run(self):
        while(self.run_read):
            content=self.ser.readlines()
            if (content):
                content=content[0].decode()
                if content[:5]=='$SES!':
                    print("send commands recieved")
                    self.Queue_R.put(content)
                elif content[:5]=='$SOK!':
                    print("send is ok")
                    self.Queue_OK.put(content)
                elif content[:5]=='$POK!':
                    print("you used this device without licenese, and your device is lock. for unlocking the device please have contact with +989129459183")
                else:
                    print("not standard content:")
                    print(content)
    def stop(self):
        self.run_read=False`


class RunDesignerGUI():
    def __init__(self):
        app = QtWidgets.QApplication(sys.argv)
        
        self.init_uart('COM6')
        
        while(1): 
            x=input()
            if x[0]=="S":
                resp=self.write_uart_wait_for_response("$SE!+989129459183,سلام و عرض ادب")
                print(resp)
                print('-----')
            else:
                print("khiyar")
        sys.exit(app.exec_())
    
    def init_uart(self,COMPORT):
        self.Queue_R= queue.Queue(maxsize=10)
        self.Queue_OK= queue.Queue(maxsize=10)
        self.serial = serial.Serial(COMPORT, 115200, timeout=0)
        self.uart_reciver=UartReciever(self.serial,self.Queue_R,self.Queue_OK)
        self.uart_reciver.start()
        
    def write_uart_wait_for_response(self,data):
        self.serial.write(data.encode())
        response=False
        tries=0
        while(not response):
            
            try:
                response=self.Queue_R.get(block=True, timeout=3)
                break
            except:
                print(f"no response :{tries+1}")
                tries=tries+1
                if tries>2:
                    response=None
                    break
                else:
                    print('send again ...')
                    self.serial.write(data.encode())
        return response
                    
            

        
    
if __name__ == "__main__":
    RunDesignerGUI()
    