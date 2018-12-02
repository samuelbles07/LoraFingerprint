import serial
import datetime
import time
import threading

from gui import MyGui
import tkinter as tk

from db import MyDb

MENDAFTAR = 1
KEHADIRAN = 2

class MainApp(MyGui, MyDb):

    def __init__(self, GUIs, DBs):
        self.lastSenderId   = None
        self.lastMode       = None
        self.lastData       = None
        
        self.ser = serial.Serial(      
            port='/dev/ttyAMA0',
            baudrate = 9600,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )

        self.mygui = GUIs
        self.mydb  = DBs

        data = self.mydb.get_matching_data()
        self.mygui.show_all_match_data(data)

    def parseData(self):
        data = self.ser.read(5)

        if (len(data) > 3):
            self.lastSenderId = data[3]
            self.lastMode     = data[4]
            self.lastId       = data[5]
            print(data)
            return True
        else:
            return False

    def run(self):
        while(True):
            if self.parseData() is True:
                print("New data")
                datenow = datetime.datetime.now().strftime("%H:%M %d-%m-%y")

                if self.lastMode == KEHADIRAN
                    name = self.mydb.save_match_data([self.lastId, self.lastSenderId, datenow])
                    self.mygui.insert_new_data([self.lastId, "", datenow, self.lastSenderId])

                elif self.lastMode == MENDAFTAR:
                    # self.mydb.save_enroll_data([self.lastId, None, datenow])
                    self.mygui.avail_enroll_data.append([self.lastId, "", datenow])

                else
                    self.mydb.delete_data(self.lastId)

            if self.mygui.ready_to_save is True:
                self.mydb.save_enroll_data(self.mygui.data_to_save)
                self.mygui.ready_to_save = False
                self.mygui.data_to_save = ""

            time.sleep(5)

    def startApp(self):
        threads = []
        myThread = threading.Thread(target=self.run)
        threads.append(myThread)
        myThread.start()
        

if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("645x420")
    mygui = MyGui(root)

    mydb = MyDb()

    app = MainApp(mygui, mydb)
    app.startApp()

    root.mainloop()
