import threading
import time
import keyboard as keyboard
import serial
from datetime import datetime

timer = datetime.now()
flag1: bool = True
flag2: bool = True
ser = serial.Serial('COM3', baudrate=115200, timeout=2)
my_file = open("output.txt", "w")


def read_com_write_file():
    while flag1:
        print(flag1)
        com_data = ser.readline()

        if com_data.decode("utf-8") == "":
            print(".")
        else:
            my_file.write(com_data.decode("utf-8") + timer.strftime("%d-%m-%y %H:%M:%S\n"))
            print(com_data)

        time.sleep(0)


def keyboard_listener():
    while flag2:
        if keyboard.is_pressed("s"):
            ser.write(b's')
            time.sleep(0.5)
        elif keyboard.is_pressed("1"):
            ser.write(b'1')
            time.sleep(0.5)
        elif keyboard.is_pressed("2"):
            ser.write(b'2')
            time.sleep(0.5)
        elif keyboard.is_pressed("e"):
            ser.write(b'e')
            stop_work()
            break
        else:
            time.sleep(0)


def stop_work():
    global flag1, flag2
    with lock:
        flag1 = False
        flag2 = False


lock = threading.Lock()
thread_com_reader = threading.Thread(target=read_com_write_file, args=())
thread_keyboard = threading.Thread(target=keyboard_listener, args=())

thread_com_reader.start()
thread_keyboard.start()
