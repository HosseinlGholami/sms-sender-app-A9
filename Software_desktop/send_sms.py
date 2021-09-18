from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from sysinfo import *
import serial
import serial.tools.list_ports
import queue
import time

class sendSmsSignals(QObject):
    progress = pyqtSignal(int)
    
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
                print(content)
                self.Queue.put(content)
    def stop(self):
        self.terminate()

def queue_get(resp,queue,number_of_wait,function,args):
    iteration=0
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

    cs=content.encode()
    status=0
    if queue_get("SE",Queue,3,CSerial.write,cs):
        sendSmsSignals.
        #the messege send to device find
        if queue_get("OK1",Queue,10,Null,None):
            #DEVICE SEND SMS TO BTS OK
            if queue_get("OK2",Queue,3,Null,None):
                pass
                #sms has deliverd to client
            else:
                pass
                #sms deliverd ignored
        else:
            
            pass
            #the messege send to device and he ack it but doesent sendit!
    else:   
        pass
        #the messege send to device but he didnt ack it 
    uart_reciver.stop()
    CSerial.close()
    print("END")
    print(number,content)

sig=sendSmsSignals()
send_message("09","salamsalam -",sig,"COM7")