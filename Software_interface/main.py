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
from ui.sendall import Ui_MainWindow as sendall_MainWindow

import concurrent.futures
import threading
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from Search_comport import ComPortSearch , SearchSignals
from send_validity  import send_validity , validitySignals
from send_sms       import send_message  , sendSmsSignals
from handel_excell_file import get_number_from_excel_file

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
        self.sendallWindow = QtWidgets.QMainWindow()
        
        #SetupUIs
        self.intui = introUI_MainWindow()
        self.intui.setupUi(self.introWindow )

        self.tstui = test_MainWindow()
        self.tstui.setupUi(self.testWindow )
        
        self.senallui= sendall_MainWindow()
        self.senallui.setupUi(self.sendallWindow )
        
        self.widget_update()
        self.widget_action()
        
        #todo uncomment
            # self.sendallWindow.hide()
            # self.testWindow.hide()
            # self.introWindow.show()
        
        # todo: remove
        self.sendallWindow.show()
        
        sys.exit(app.exec_())
        
    def widget_update(self):
        self.tstui.Login_Button_3.setEnabled(False)
        self.tstui.Login_Button_2.setEnabled(False)
        self.senallui.pushButton_2.setEnabled(False)
        
    def widget_action(self):
        self.intui.searchButton.clicked.connect(self.th_search_comport_function)
        self.tstui.Login_Button.clicked.connect(self.check_validity)
        self.tstui.Login_Button_2.clicked.connect(self.send_test_sms)
        self.tstui.Login_Button_3.clicked.connect(self.go_for_all)
        self.senallui.pushButton.clicked.connect(self.load_excel_file)
        self.senallui.pushButton_2.clicked.connect(self.start_send_to_all)
    

    
    def start_send_to_all(self):
        for i,number in enumerate(self.numbers):
            self.person_index=i
            self.sendsms_Signal=sendSmsSignals()
            self.sendsms_Signal.progress.connect(self.send_result)
            #TODO: COMPORT SHOULD CHANGE 
            send_message(number[0],"self.sms_text",self.sendsms_Signal,"COM10")
            self.send_log_sendall(f"{number}"+"excel loaded")
            for _ in range(10*10):
                # Process events between short sleep periods
                QtWidgets.QApplication.processEvents()
                time.sleep(0.1)

    def send_result(self,status):
        if   status =="1":
            self.numbers[self.person_index][1]='ok'
            self.send_log_test("messege sends to device ok")
        elif status =="2":
            self.numbers[self.person_index][2]='ok'
            self.send_log_test("messege sends to client success")
            self.tstui.Login_Button_3.setEnabled(True)
        elif status =="3":
            self.send_log_test("messege sends to client has problem you can try again!")
        elif status =="4":
            self.send_log_test("messege sends to device Fail or your content has problem.")
            
        

    def load_excel_file(self):
        FILE_NAME="./a.xlsx"
        self.numbers=get_number_from_excel_file(FILE_NAME)
        self.senallui.label_3.setText(str(len(self.numbers)))
        self.send_log_sendall("excel loaded")
        self.senallui.pushButton_2.setEnabled(True)
        
    
    def check_validity(self):
        self.sms_text=self.tstui.plainTextEdit.toPlainText()
        self.validity_Signal=validitySignals()
        self.validity_Signal.progress.connect(self.validity_result)
        todo: change 
        send_validity(self.validity_Signal,self.COMPORT)
        # send_validity(self.sms_text,self.validity_Signal,"COM10")
        
    def validity_result(self,result):
        if result:
            self.tstui.label_2.setText("IS valid")
            self.tstui.Login_Button_2.setEnabled(True)
            self.send_log_test("your messege is valid")
        else:
            self.tstui.label_2.setText("NOT valid ")
            self.tstui.Login_Button_2.setEnabled(False)
            self.tstui.Login_Button_3.setEnabled(False)
            self.send_log_test("reduce the lenght of the messege")
            
    def send_test_sms(self):
        tst_numer=self.tstui.lineEdit_2.text()
        self.testsms_Signal=sendSmsSignals()
        self.testsms_Signal.progress.connect(self.sendtest_result)
        #todo: change
            # send_message(tst_numer,self.sms_text,self.testsms_Signal,self.COMPORT)
        send_message(tst_numer,self.sms_text,self.testsms_Signal,"COM10")
        
    def sendtest_result(self,status):
        if   status =="1":
            self.send_log_test("messege sends to device ok")
        elif status =="2":
            self.send_log_test("messege sends to client success")
            self.tstui.Login_Button_3.setEnabled(True)
        elif status =="3":
            self.send_log_test("messege sends to client has problem you can try again!")
        elif status =="4":
            self.send_log_test("messege sends to device Fail or your content has problem.")
        
    def go_for_all(self):
        print("go for all")
        self.testWindow.hide()
        self.sendallWindow.show()
        
    
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
    def send_log_test(self,txt):
        pre_txt=self.tstui.LogtextBrowser.toPlainText()
        if (pre_txt==''):
            self.tstui.LogtextBrowser.setText(txt)
        else:
            self.tstui.LogtextBrowser.setText(pre_txt+'\n'+txt)
    def send_log_sendall(self,txt):
        pre_txt=self.senallui.LogtextBrowser.toPlainText()
        if (pre_txt==''):
            self.senallui.LogtextBrowser.setText(txt)
        else:
            self.senallui.LogtextBrowser.setText(pre_txt+'\n'+txt)
            
            

        
    
if __name__ == "__main__":
    RunDesignerGUI()
    