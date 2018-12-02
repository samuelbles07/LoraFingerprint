import tkinter as tk
from tkinter import ttk
from tkinter import *

class MyGui(tk.Frame):

    def __init__(self, master):
        self.master = master
        tk.Frame.__init__(self, self.master) # , bg="red")

        self.pack(fill='both', expand=True)

        self.create_main_window()
        self.avail_enroll_data = list()
        # self.avail_enroll_data = [[1, 'abc', 'abc', 'abc'], [2, 'ghj', 'ghj', 'ghj'], [4, '29i', '29i', '29i']]
        self.data_to_save = []
        self.ready_to_save = False

    def create_main_window(self):

        # create tk variable
        self.timervar = tk.StringVar(self)

        # dropdown dictionary
        self.timerDict = {"-", "5 minutes", "10 minutes", "15 minutes"}
        self.timervar.set("-")  # <-- set the default value

        # timer dropdown menu
        self.timer_option = tk.OptionMenu(self, self.timervar, *self.timerDict) #, command=self.req_timer)
        self.timer_option.grid(row=1, column=3, columnspan=2, padx=3, pady=3, sticky=tk.NSEW)

        self.add_menu_bar()
        self.add_tree_view()

    def save_data(self):
        self.avail_enroll_data.remove(self.data_to_save)
        self.data_to_save[1] = self.txtname.get()
        #disini save data ke db
        print(self.data_to_save)
        print(self.avail_enroll_data)
        self.txtid.delete(0, END)
        self.txtname.delete(0, END)
        self.ready_to_save = True

    def get_data(self):
        self.txtid.delete(0, END)
        self.txtname.delete(0, END)
        if len(self.avail_enroll_data) > 0:
            self.data_to_save = self.avail_enroll_data[0]
            self.txtid.insert(0, self.data_to_save[0])


    def enroll_window(self):
        filewin = Toplevel(self.master)
        filewin.geometry("290x150")

        # lbl = Label(filewin, text="ID").pack()
        Label(filewin, text="ID").grid(row=0, padx=(20, 10), pady=(10,5))
        Label(filewin, text="Nama").grid(row=1, padx=(20, 10), pady=(5,10))

        self.txtid = Entry(filewin)
        self.txtname = Entry(filewin)

        self.txtid.grid(row=0, column=1)
        self.txtname.grid(row=1, column=1)

        btnrefresh = Button(filewin, text="Refresh", command=self.get_data)
        btnrefresh.grid(row=2, column=1, sticky=NSEW)#, padx=(20, 10), pady=(25,5))
        btnsave = Button(filewin, text="Save", command=self.save_data)
        btnsave.grid(row=3, column=1, sticky=NSEW)#, padx=(20, 10), pady=(30,5))
        # button.pack()

    def do_nothing(self):
        data = ["dfa", "rtwqt", "vxnmv", "hfsjk"]
        self.insert_new_data(data)

    def add_menu_bar(self):
        menubar = Menu(self.master)
        filemenu = Menu(menubar, tearoff=0)
        filemenu.add_command(label="Enroll", command=self.enroll_window)
        filemenu.add_command(label="Export", command=self.do_nothing)
        filemenu.add_command(label="Send", command=self.do_nothing)
        menubar.add_cascade(label="Run", menu=filemenu)
        self.master.config(menu=menubar)

    def add_tree_view(self):
        # scroll bar for the terminal outputs
        self.terminal_scrollbar = tk.Scrollbar(self)
        self.terminal_scrollbar.grid(row=4, column=25, sticky=tk.NS)

        # terminal output
        self.terminal_tree = ttk.Treeview(self, height=20)
        self.terminal_tree.configure(style="Treeview")
        
        self.terminal_tree.grid(row=4, column=0, columnspan=5, sticky=tk.NSEW)
        self.terminal_tree.configure(yscrollcommand=self.terminal_scrollbar.set)
        self.terminal_tree["columns"] = ("id", "name", "datetime", "room")
        self.terminal_tree['show'] = 'headings'
        self.terminal_tree.column("id", width=70, anchor='c')
        self.terminal_tree.column("name", width=250, anchor='c')
        self.terminal_tree.column("datetime", width=200, anchor='c')
        self.terminal_tree.column("room", width=100, anchor='c')
        self.terminal_tree.heading("id", text="ID")
        self.terminal_tree.heading("name", text="Nama")
        self.terminal_tree.heading("datetime", text="Tanggal/Jam")
        self.terminal_tree.heading("room", text="Ruangan")

    def insert_new_data(self, data):
        self.terminal_tree.insert("",'end',text="L1",values=(data[0],data[1], data[2], data[3]))

    def show_all_match_data(self, data):
        for row in data[::-1]:
            self.insert_new_data(row)


if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("645x420")
    MyGui(root)
    root.mainloop()
