#!/usr/bin/python

import glob
import serial
import sys
import platform

DEBUG_PRINT = 0


###############################################################################
def main():
    port = serial_init()
    while True:
        # base = 0 or 1 (B or C)
        # axis = 0 or 1 (horizontal or vertical)
        # centroids = array of 4 floats in microseconds
        base, axis, centroids = parse_data(port)

        if not DEBUG_PRINT:
            print base, axis, centroids


###############################################################################
def serial_init():
    PLATFORM = platform.system()
    if "Linux" in PLATFORM:
        SERIAL_PATH = "/dev/ttyUSB*"
    elif "Darwin" in PLATFORM:
        SERIAL_PATH = "/dev/tty.usb*"   # TODO: test it
    else: # Windows
        SERIAL_PATH = "COM*"            # TODO: test it

    devices = glob.glob(SERIAL_PATH)

    if DEBUG_PRINT: print devices
    port = serial.Serial(devices[0], 115200 * 2)
    success = port.isOpen()

    if success:
        if DEBUG_PRINT: print "Port open."
        lookForHeader(port)
    else:
        print "\n!!! Error: serial device not found !!!"
        exit(-1)
    return port


###############################################################################
def lookForHeader(port):
    if DEBUG_PRINT: print("seeking header\n")

    # packets structure:
    # 2 headers + 1 base_axis + (4 photodiodes * 2 bytes) + (3 accel * 2 bytes)

    while True:

        while readByte(port) != 255:
            pass # consume

        if readByte(port) != 255:
            continue

        break


###############################################################################
def readByte(port):
    byte = ord(port.read(1))
    if DEBUG_PRINT: print byte
    return byte


###############################################################################
def parse_data(port):
    centroidNum = 4

    base_axis = readByte(port)
    base = (base_axis >> 1) & 1
    axis = (base_axis >> 0) & 1

    if DEBUG_PRINT: print "\nbase, axis:", base, axis

    centroids = [0 for i in range(centroidNum)]

    for i in range(centroidNum):
        centroids[i] = getCentroid(port)
        if DEBUG_PRINT: print "centroids[", i, "] =", centroids[i]

    # consumes header
    for i in range(2):
        b = readByte(port)
        if (b != 255):
            if DEBUG_PRINT: print "header problem", i
            lookForHeader(port)
            break

    return base, axis, centroids


###############################################################################
def getCentroid(port):
    startTime = decodeTime(port)
    endTime   = decodeTime(port)

    if (startTime == 0 or endTime == 0):
        return 0

    return ((endTime + startTime) / 2)


###############################################################################
def decodeTime(port):
    rxl = readByte(port)        # LSB first
    rxh = readByte(port)        # MSB last
    time = (rxh << 8) + rxl     # reconstruct packets
    time <<= 2                  # (non-significant) lossy decompression
    time /= 16.                 # convert to us

    if (time >= 6777 or time < 1222):
        time = 0
        if DEBUG_PRINT: print "INVALID TIME"

    return time



###############################################################################
if __name__ == "__main__":
    main()

