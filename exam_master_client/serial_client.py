from time import sleep
from serial import Serial
from serial.tools.list_ports import comports
import sys
import os
import datetime

def send_exam(path, s):
    s.write('EXSTART\r\n'.encode())
    sleep(2)
    with open(path, 'rb') as f:
        zip_data = f.read()

    chunk_size = 128
    print(f"Sending {len(zip_data)} bytes of image data...")

    for i in range(0, len(zip_data), chunk_size):
        chunk = zip_data[i:i + chunk_size]
        s.write(chunk)
        sleep(0.1)

    sleep(2)
    s.write('EXEND\r\n'.encode())
    print("Finished transfering exam")


def parse_serial():
    global baud_rate
    state = None
    curr_file = None
    sent_exam = False

    sleep(2)

    ports = comports()
    for port, _, hwid in ports:
        print(hwid)
        if '58CD185518' in hwid:
            with Serial(port) as s:
                s.baudrate = 115200
                s.setDTR(False)
                s.setRTS(False)
                sleep(2)

                while True:
                    if s.in_waiting > 0:
                        data_unsplit = s.read(s.in_waiting)
                        try:
                            data_split = data_unsplit.decode().split('\r\n')
                            for data in data_split:
                                print(data)
                                if 'STUDENT_ID' in data:
                                    if not os.path.exists(data.split(';')[1]):
                                        os.mkdir(data.split(';')[1])
                                    wifi_handler = open(data.split(';')[1] + "/info.txt", 'w')
                                    sf = open(data.split(';')[1] + "/info.txt", 'w')
                                    sf.write(data.split(';')[1] + "\r\n")
                                    sf.close()
                                elif 'STUDENT_NAME' in data:
                                    sf = open(data.split(';')[1] + "/info.txt", 'a')
                                    sf.write(data.split(';')[2] + "\r\n")
                                    sf.close()
                                elif 'MAC' in data:
                                    sf = open(data.split(';')[1] + "/info.txt", 'a')
                                    sf.write(data.split(';')[2] + "\r\n")
                                    sf.close()
                                elif 'PSTART' in data:
                                    state = 'PROCESS'
                                    curr_file = open(data.split(';')[1] + "/processes.txt", 'w')
                                elif 'PEND' in data:
                                    state = None
                                    curr_file.close()
                                elif state == 'PROCESS':
                                    curr_file.write(data)
                                elif 'SCSTART' in data:
                                    state = 'SCRT'
                                    curr_file = open(data.split(';')[1] + "/scrt_" + datetime.datetime.now() + ".png", 'wb')
                                elif 'SCEND' in data:
                                    state = None
                                    curr_file.close()
                                elif state == 'SCRT':
                                    curr_file.write(data) 
                                elif 'EXSUB' in data:
                                    state = 'EXAM'
                                    curr_file = open(data.split(';')[1] + "/exam_" + datetime.datetime.now() + ".zip", 'wb')
                                elif 'EXFIN' in data:
                                    state = None
                                    curr_file.close()
                                elif state == 'EXAM':
                                    curr_file.write(data) 
                                if not data:
                                    break
                        except Exception as err:
                            print(err)
                            pass

                    if os.path.exists('exam.zip') and not sent_exam:
                        sleep(3)
                        send_exam('exam.zip', s)
                        sent_exam = True


    raise Exception('TinyUSB COM port not found')


if __name__ == '__main__':
    parse_serial()
