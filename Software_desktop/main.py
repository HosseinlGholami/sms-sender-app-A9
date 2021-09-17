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
from Util import *


################
################UI Program
################
class RunDesignerGUI():
    def __init__(self):
        app = QtWidgets.QApplication(sys.argv)
        self.sms_text=""
        self.COMPORT=""
        #Windows
        self.introWindow = QtWidgets.QMainWindow()
        self.testWindow = QtWidgets.QMainWindow()
        
        #SetupUIs
        self.intui = introUI_MainWindow()
        self.intui.setupUi(self.introWindow )

        self.tstui = test_MainWindow()
        self.tstui.setupUi(self.testWindow )
        
        self.widget_action()
        
        self.testWindow.hide()
        self.introWindow.show()
        sys.exit(app.exec_())
        
    def widget_action(self):
        self.intui.searchButton.clicked.connect(self.th_search_comport_function)
        self.tstui.Login_Button.clicked.connect(self.check_validity)
        
    
    def check_validity(self):
        self.sms_text=self.tstui.plainTextEdit.toPlainText()
        print(self.sms_text)
        print(type(self.sms_text))
        if len(self.sms_text)<250:
            self.tstui.label_2.setText("IS valid")
        else:
            self.tstui.label_2.setText("NOT valid ")

    
    def th_search_comport_function(self):
        self.search_Signal=SearchSignals()
        self.search_Signal.progress.connect(self.send_log)
        self.search_Signal.progress.connect(self.receive_valid_comport)
        self.search_Signal.finished.connect(self.Finish_search_thread)
        self.comport_search_thread=ComPortSearch(self.search_Signal)
        self.comport_search_thread.start()
    
    def receive_valid_comport(self,txt):
        self.COMPORT=txt
          
    def Finish_search_thread(self):
        self.comport_search_thread.stop()
        self.introWindow.hide()
        self.testWindow.show()
        print("We have the"+self.COMPORT)            
    
    
    def send_log(self,txt):
        pre_txt=self.intui.LogtextBrowser.toPlainText()
        if (pre_txt==''):
            self.intui.LogtextBrowser.setText(txt)
        else:
            self.intui.LogtextBrowser.setText(pre_txt+'\n'+txt)
            

        
    
if __name__ == "__main__":
    RunDesignerGUI()
    