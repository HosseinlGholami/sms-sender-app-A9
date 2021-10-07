from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from sysinfo import *
import serial
import serial.tools.list_ports
import queue
import time

class sendSmsSignals(QObject):
    progress = pyqtSignal(str)
    
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

def queue_get(resp,queue,number_of_wait,function,args):
    function(args)
    iteration=1
    while(True):
        try:
            response=queue.get(block=True, timeout=3)
            if resp in response:
                return True
            raise Exception
        except:
            if iteration==number_of_wait:
                return None
            iteration=iteration+1
            function(args)
def Null(null):
    pass
def send_message(number,content,Signals,COMPORT):
    CSerial = serial.Serial(COMPORT, 115200, timeout=0)
    Queue = queue.Queue(maxsize=10)
    uart_reciver = UartReciever(CSerial,Queue)
    uart_reciver.setTerminationEnabled(True)
    uart_reciver.start()
    
    cs="$SEN!"+number+','+content
    
    if queue_get("$SES!",Queue,3,CSerial.write,cs.encode()):
        Signals.progress.emit("1")
        #the messege send to device find
        if queue_get("$SOK!",Queue,10,Null,None):
            Signals.progress.emit("2")
            #DEVICE SEND SMS TO BTS OK
        else:
            Signals.progress.emit("3")
    else:
        Signals.progress.emit("4")
        #the messege send to device but he didnt ack it 
    uart_reciver.stop()
    CSerial.close()
    del CSerial
    del Queue
    del uart_reciver
    
# a=sendSmsSignals()
# send_message("+989129459183","salamsalam",a,"COM10")