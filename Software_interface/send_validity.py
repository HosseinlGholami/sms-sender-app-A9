from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from sysinfo import *
import serial
import serial.tools.list_ports
import queue
import time


class validitySignals(QObject):
    progress = pyqtSignal(bool)
    
class UartReciever(QThread):
    def __init__(self,serial,queue_R):
        super(UartReciever, self).__init__()
        self.ser=serial
        self.Queue=queue_R
    def run(self):
        while(True):
            content=self.ser.readlines()
            if (content):
                content=content[0].decode()
                self.Queue.put(content)
    def stop(self):
        self.terminate()
    
def queue_get(queue,number_of_wait,function,args):
    function(args)
    iteration=1
    while(True):
        try:
            response=queue.get(block=True, timeout=3)
            return int(response)
        except:
            if iteration==number_of_wait:
                return None
            iteration=iteration+1
            function(args)
    
def send_validity(content,Signals,COMPORT):
    CSerial = serial.Serial(COMPORT, 115200, timeout=0)
    Queue = queue.Queue(maxsize=10)
    uart_reciver = UartReciever(CSerial,Queue)
    uart_reciver.setTerminationEnabled(True)
    uart_reciver.start()
    cs="$VSE!"+content
    response=queue_get(Queue,3,CSerial.write,cs.encode())
    if response:
        if response>254:
            Signals.progress.emit(False)
        else:
            Signals.progress.emit(True)
    else:
        Signals.progress.emit(False)
    CSerial.close()
    uart_reciver.stop()
    del CSerial
    del Queue
    del uart_reciver
    del response