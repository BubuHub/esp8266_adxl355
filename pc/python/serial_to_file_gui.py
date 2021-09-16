#!/usr/bin/env python3
import tkinter as tk
import tkinter.ttk as ttk
import serial.tools.list_ports
import socket
import sys
import signal
import serial
import time
import os
from time import sleep
from random import randint
import configparser

config = configparser.ConfigParser()
config['DEFAULT'] = {'SerialPort': '/dev/ttyUSB0','OutputFile': 'adxl355z-$data.csv'}

box_value=""
port_list=[port.device+ ': ' + port.description for port in serial.tools.list_ports.comports()]


root = tk.Tk()
root.title("Ustawienia skryptu")

# Create a style
style = ttk.Style(root)
# Set the theme with the theme_use method
style.theme_use('clam')  # put the theme name here, that you want to use

tk.Label(root, text="Port RS:").grid(row=0)
cb = ttk.Combobox(root, values=port_list, width=50)
if (len(port_list)>0):
	cb.current(0)
cb.grid(row=0, column=1,sticky=tk.W,pady=4)
tk.Label(root, text="Plik wyjsciowy:").grid(row=1)
e1 = tk.Entry(root)
e1.grid(row=1, column=1,sticky=tk.W,pady=4)
e1.insert(10, config['DEFAULT']['OutputFile'])

def test_send(event=None):
	ser_port = cb.get().split(':', 1)[0]
	if ser_port.upper().startswith('COM') and int(ser_port[3:]) > 8:
		ser_port = '\\\\.\\' + ser_port
	config['DEFAULT']['SerialPort']=ser_port
	config['DEFAULT']['OutputFile']=e1.get();
	with open('config.ini', 'w') as configfile:
		config.write(configfile)


def on_select(event=None):
	# get selection from event....
	print("event.widget:", event.widget.get())
	# or get selection directly from combobox
	print("comboboxes: ", cb.get())

# --- main ---

# assign function to combobox
cb.bind('<<ComboboxSelected>>', on_select)
button = tk.Button(root, text='ZAPISZ', width=50)
button.bind('<Button-1>', test_send)
button.grid(row=3,column=1,sticky=tk.W,pady=4)

root.mainloop()


