#!/usr/bin/python
# raspberry pi nrf24l01 hub
# more details at http://blog.riyas.org
# Credits to python port of nrf24l01, Joao Paulo Barrac & maniacbugs original c library
# ======================= changes on may 1,2015=======================
#added csv write..and parses multiple data elements (Temp, press, humidity)
#result goes to a file called temp.csv in the same place as this file

from nrf24 import NRF24
import time
from time import gmtime, strftime
from datetime import datetime

pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]

radio = NRF24()
radio.begin(0, 0,25,18) #set gpio 25 as CE pin
radio.setRetries(15,15)
radio.setPayloadSize(32)
radio.setChannel(0x4c)
radio.setDataRate(NRF24.BR_250KBPS)
radio.setPALevel(NRF24.PA_MAX)
radio.setAutoAck(1)
radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])
radio.startListening()
radio.stopListening()

radio.printDetails()
radio.startListening()
def hasNumbers(inputString):
    return any(char.isdigit() for char in inputString)
def extract(raw_string, start_marker, end_marker):
    start = raw_string.index(start_marker) + len(start_marker)
    end = raw_string.index(end_marker, start)
    return raw_string[start:end]
while True:
    pipe = [0]
    while not radio.available(pipe, True):
        time.sleep(1000/1000000.0)
    recv_buffer = []
    radio.read(recv_buffer)
    out = ''.join(chr(i) for i in recv_buffer)
    temper=extract(out,'T','T')
    humid=extract(out,'H','H')
    press=extract(out,'P','P')
    #import pdb; pdb.set_trace()
    #print out
    print temper
    print humid
    print press
    #write to csv if temper has a number
    if  hasNumbers(temper):
        print "writing csv"
        date = str(datetime.now())
        filep = open("temp.csv", "a")
        print >> filep, ";".join([date,temper, humid, press])
        filep.close()
    
