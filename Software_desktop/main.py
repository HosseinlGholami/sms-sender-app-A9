from PyQt5.QtCore import pyqtSignal, pyqtSlot, Qt, QThread
from PyQt5.QtGui import QPixmap
from PyQt5.QtWidgets import QWidget

import numpy as np
import time

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
from Search_comport import *
from send_sms import send_message


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
        
        self.widget_update()
        self.widget_action()
        
        #just for test
        # self.testWindow.hide()
        # self.introWindow.show()
        self.testWindow.show()
        
        sys.exit(app.exec_())
    def widget_update(self):
        self.tstui.Login_Button_3.setEnabled(False)
        self.tstui.Login_Button_2.setEnabled(False)
        
    def widget_action(self):
        self.intui.searchButton.clicked.connect(self.th_search_comport_function)
        self.tstui.Login_Button.clicked.connect(self.check_validity)
        self.tstui.Login_Button_2.clicked.connect(self.send_test_sms)
        self.tstui.Login_Button_3.clicked.connect(self.go_for_all)
    
    def check_validity(self):
        self.sms_text=self.tstui.plainTextEdit.toPlainText()
        
        if len(self.sms_text)<250:
            self.tstui.label_2.setText("IS valid")
            self.tstui.Login_Button_2.setEnabled(True)
            send_log("your messege is valid")
        else:
            self.tstui.label_2.setText("NOT valid ")
            self.tstui.Login_Button_2.setEnabled(False)
            self.tstui.Login_Button_3.setEnabled(False)
            send_log("reduce the lenght of the messege")
    def send_test_sms(self):
        tst_numer=self.tstui.lineEdit_2.text()
        status=send_message(tst_numer,self.sms_text,self.COMPORT)
        if   status ==1:
            send_log("messege sends to device ok")
        elif status ==2:
            send_log("messege sends to device Fail try agin ...")
        elif status ==3:
            send_log("messege sends to client success")
        elif status ==4:
            send_log("messege sends to client Fail")
        elif status ==5:
            send_log("messege deliverd to client")
            
        # self.tstui.Login_Button_3.setEnabled(True)
    
    def go_for_all(self):
        print("go for all")
        self.testWindow.hide()
    
    def th_search_comport_function(self):
        self.search_Signal=SearchSignals()
        self.search_Signal.progress.connect(self.send_log)
        self.search_Signal.progress.connect(self.receive_valid_comport)
        self.search_Signal.finished.connect(self.Finish_search_thread)
        self.comport_search_thread=ComPortSearch(self.search_Signal)
        self.comport_search_thread.start()
    
    def receive_valid_comport(self,txt):
        if self.COMPORT=="":
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
    