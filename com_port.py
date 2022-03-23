import threading
import time
import keyboard as keyboard
import serial
from datetime import datetime

timer = datetime.now()
flag1: bool = True #flag for stoping thread which read from serial
flag2: bool = True #flag for stoping thread which read from keyboard
ser = serial.Serial('COM3', baudrate=115200, timeout=2) #Opening serial ports
my_file = open("output.txt", "w")                       #Opening or create file for data write

#Function for read from serial and write to file
def read_com_write_file():
    while flag1:
        com_data = ser.readline()

        if com_data.decode("utf-8") == "":
            print(".")
        else:
            my_file.write(com_data.decode("utf-8") + timer.strftime("%d-%m-%y %H:%M:%S\n"))
            print(com_data)

        time.sleep(0) #relinquishes the thread’s current time slice immediately, voluntarily handing over the CPU to other threads.

#Function for reading keyboard and send parametrs to serial port
def keyboard_listener():
    while flag2:
        if keyboard.is_pressed("s"):##send to PSOC - 's' for stop print data to UART
            ser.write(b's')
            time.sleep(0.5)
        elif keyboard.is_pressed("1"):#send to PSOC - '1' for change time reading data to 1 second
            ser.write(b'1')
            time.sleep(0.5)
        elif keyboard.is_pressed("2"):#send to PSOC - '2' for change time reading data to 2 second
            ser.write(b'2')
            time.sleep(0.5)
        elif keyboard.is_pressed("e"):#exit - stop program on python but don't stop PSOC
            ser.write(b'e')
            stop_work()
            break
        else:
            time.sleep(0)

#Function for stop threads
def stop_work():
    global flag1, flag2
    with lock:
        flag1 = False
        flag2 = False


lock = threading.Lock()                                                     #The class implementing primitive lock objects
thread_com_reader = threading.Thread(target=read_com_write_file, args=())   #Thread which read from serial
thread_keyboard = threading.Thread(target=keyboard_listener, args=())       #Thread which read from keyboard

thread_com_reader.start()
thread_keyboard.start()
