#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Instalacja pyinstaller (budowanie exe, nie wiedziec czemu w Python3 Windows 10 wykrywa jako wirusa):
#	1. Install python version python-2.7.18.amd64
#	1. Open Command Prompt as administrator
#	2. Downgrade pip to version 18.1 pip install pip==18.1
#	3. Download pyinstaller version 3.4 pip install pyinstaller==3.4
#	4. Success, This should work. Verify installation using pyinstaller -v
#
import socket
import sys
import signal
import serial
import time
import os
import csv
from time import sleep
from threading import Thread
import configparser
import argparse
import datetime
from binascii import hexlify
from codecs import encode  # alternative

config = configparser.ConfigParser()
config['DEFAULT'] = {'SerialPort': '/dev/ttyUSB0','OutputFile': 'adxl355z-$data.csv'}
headers = ["X","Y","Z"]

parser = argparse.ArgumentParser(description='Process some arguments.')
parser.add_argument('-s','--serial' ,help='serial port')
parser.add_argument('-o','--output' ,help='output file')

args = parser.parse_args()

config.read('config.ini')

if args.serial:
	config['DEFAULT']['SerialPort']=args.serial

if args.output:
	config['DEFAULT']['OutputFile']=args.output


now = datetime.datetime.now() # current date and time
serialPort = str(config['DEFAULT']['SerialPort'])
outputFile = str(config['DEFAULT']['OutputFile']).replace('$data',now.strftime("%Y-%m-%d_%H-%M-%S"))

print("Serial port = <"+serialPort+">, output file = <"+outputFile+">\n")

with open(outputFile, 'ab') as f_output:
	csv_output = csv.DictWriter(f_output, fieldnames=headers, delimiter=';', quotechar='"', quoting=csv.QUOTE_MINIMAL)
	f_output.seek(0, 2)
	if f_output.tell() == 0:
		csv_output.writeheader()
	ser = serial.Serial()
	ser.port = serialPort
	ser.baudrate = 1500000
	ser.bytesize = serial.EIGHTBITS    # number of bits per bytes
	ser.parity = serial.PARITY_NONE    # set parity check: no parity
	ser.stopbits = serial.STOPBITS_ONE # number of stop bits
	ser.timeout = 10                   # timeout block read
	ser.xonxoff = False                # disable software flow control
	ser.rtscts = False                 # disable hardware (RTS/CTS) flow control
	ser.dsrdtr = False                 # disable hardware (DSR/DTR) flow control
	ser.writeTimeout = 10              # timeout for write
	ser.open()
	time.sleep(0.1);
	if ser.isOpen():
		print('Send: Uart ready')
		try:
			ser.flushInput()
			ser.flushOutput()
			ser.readline();
			while True:
				s=str(ser.readline().strip())
				if s != '':
					print(s);
					newstr = s;
					listOfNumbers = newstr.split(";");
					if (len(listOfNumbers) > 2):
						csv_output.writerow({'X': listOfNumbers[0], 'Y': listOfNumbers[1], 'Z': listOfNumbers[2]})
						f_output.flush()
			ser.close()
			print("RES: OK ");
			sys.stdout.flush()
			os._exit(0)
		except:
			e1 = sys.exc_info()[0]
			print("error: " + str(e1))
			sys.stdout.flush()
			os._exit(0)


