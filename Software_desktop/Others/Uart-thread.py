import concurrent.futures
import threading
import serial
import time


def sender(ser):
    i=0
    while (1):
        print('yee')
        time.sleep(1)
        ser.write(b'salam'+str(i).encode())
        i=i+1
        
def receiver(ser):
    print("khiyar")
    while(1):
        content=ser.readlines()
        if (content):
            print(content)

if __name__ == "__main__":
    serial = serial.Serial('COM5', 115200, timeout=0)
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
        executor.submit(receiver , serial)
        executor.submit(sender, serial)
    print('serial closed')
    serial.close()
