import serial.tools.list_ports
import time
import csv
import os
import re
from concurrent.futures import ThreadPoolExecutor, as_completed


######################################################################
# INPUT PARAMETERS
######################################################################
filepath = "C:/Users/oferc/OneDrive/Documents/1_Projects/2_Offshore_Turbine/Power/"
baudrate = 2000000
header = ["us", "Pot", "BV", "SV", "I", "T"]


######################################################################
# DEFINE FUNCTIONS
######################################################################

# Convert String to List
def convert(string):
    data_list = list(string.split(" "))
    return data_list


def turbine_row_input():
    print('\n' + ': Enter File Name.')
    filename = input("**ENTER ->")
    last_param = "TO START DATA COLLECTION"
    last_temp = re.compile(last_param)
    re_param = "LET'S TRY THAT AGAIN!"
    re_temp = re.compile(re_param)
    input_flag = True

    # CHECK IF FILE ALREADY EXISTS
    file = filepath + "/" + filename + ".csv"
    is_file = os.path.isfile(file)
    if is_file is False:                                 # IF NOT, CREATE NEW FILE
        with open(file, 'x', newline='') as f:
            writer = csv.writer(f, delimiter=",", skipinitialspace=True)
            writer.writerow(header)
            time.sleep(1)
            f.close()

    else:                                               # IF YES, ASK FOR NEW FILE NAME
        print("\n**CSV ALREADY EXISTS! [CNTRL-C] to RESTART or CONTINUE in 3 SEC...")
        time.sleep(3)

    # LIST AVAILABLE SERIAL PORTS
    ports = serial.tools.list_ports.comports()
    ports_list = []
    print("\n" + ": Available Serial Ports:")

    for onePort in ports:
        ports_list.append(str(onePort))
        print(str(onePort))

    # CHOOSE SERIAL PORT
    select_port = ': Select Available Serial Port #, COM[], from Above!'
    print('\n' + select_port)
    val = input("**ENTER -> COM")

    # ESTABLISH ARDUINO CONNECTION
    port_var = "COM" + str(val)
    arduino = serial.Serial()
    arduino.baudrate = baudrate
    arduino.port = port_var
    arduino.open()

    # ASK FOR USER INPUTS
    while input_flag:
        if arduino.in_waiting:
            packet = arduino.readline().strip()
            packet = str(packet)[2:-1]

            if packet:
                print("\n" + ": " + packet)
                re_do = re_temp.search(packet)
                if re_do is None:
                    start = input("**ENTER -> ")
                    arduino.write(bytes(start, 'utf-8'))
                    arduino.readline()

                    last = last_temp.search(packet)
                    if last and start == str(0):
                        # COLLECT DATA & SAVE TO FILE
                        print("\n" + ": READY TO COLLECT DATA...")
                        break

    return file, arduino


def turbine_row_data(file, arduino):
    data_flag = True
    end_param = "DATA COLLECTION SUCCESSFUL! SAVING DATA..."
    end_temp = re.compile(end_param)

    while data_flag:
        if arduino.in_waiting:
            packet = arduino.readline().strip()
            packet = str(packet)[2:-1]
            r_ind = re.search("r", packet)

            if r_ind:
                sub_packet = packet[0:r_ind.span()[0] - 1]
                print(sub_packet)
                with open(file, "a", newline='') as i:
                    writer_func = csv.writer(i, delimiter=",", skipinitialspace=True)
                    packet_list = convert(sub_packet)
                    writer_func.writerows([packet_list])

                packet = packet[r_ind.span()[1]:]

            print(packet)

            end = end_temp.search(packet)
            if end:
                time.sleep(1)
                i.close()
                break

            with open(file, "a", newline='') as i:
                writer_func = csv.writer(i, delimiter=",", skipinitialspace=True)
                packet_list = convert(packet)
                writer_func.writerows([packet_list])


def ar1(start1, file1, arduino1):
    arduino1.write(bytes(start1, 'utf-8'))
    turbine_row_data(file1, arduino1)


######################################################################
# MAIN
######################################################################


def main():
    file1, arduino1 = turbine_row_input()
    file2, arduino2 = turbine_row_input()
    # file3, arduino3 = turbine_row_input()
    # file4, arduino4 = turbine_row_input()
    # file5, arduino5 = turbine_row_input()
    # file6, arduino6 = turbine_row_input()
    # file7, arduino7 = turbine_row_input()
    # file8, arduino8 = turbine_row_input()
    # file9, arduino9 = turbine_row_input()
    # file10, arduino10 = turbine_row_input()
    # file11, arduino11 = turbine_row_input()
    # file12, arduino12 = turbine_row_input()

    execute = input("\n PRESS ANY KEY TO START...")
    srt_time = time.perf_counter()

    with ThreadPoolExecutor(max_workers=4) as executor:
        executor.submit(ar1, execute, file1, arduino1)
        executor.submit(ar1, execute, file2, arduino2)
        # executor.submit(ar1, execute, file3, arduino3)
        # executor.submit(ar1, execute, file4, arduino4)
        # executor.submit(ar1, execute, file5, arduino5)
        # executor.submit(ar1, execute, file6, arduino6)
        # executor.submit(ar1, execute, file7, arduino7)
        # executor.submit(ar1, execute, file8, arduino8)
        # executor.submit(ar1, execute, file9, arduino9)
        # executor.submit(ar1, execute, file10, arduino10)
        # executor.submit(ar1, execute, file11, arduino11)
        # executor.submit(ar1, execute, file12, arduino12)

    return srt_time


if __name__ == '__main__':
    start_time = main()
    end_time = time.perf_counter()
    print(f"Total Time = {end_time - start_time} [sec]")

    time.sleep(1)
    exit(0)
