#!/usr/bin/python

import glob
import serial
import sys

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
    devices = glob.glob("/dev/ttyUSB*")           # TODO make it cross platform
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
    #if DEBUG_PRINT: print "seeking header\n"
    headers_observed = 0 # we should not need more than 4
    bytes_cnt = 0 # to check for several headers before validation
    base_axis = 0 # to leave at the end of a full cycle

    while (headers_observed < 4 and base_axis != 3):
        b = readByte(port)
        bytes_cnt += 1

        if (b == 255):
            b = readByte(port)
            bytes_cnt += 1

            if (b == 255 and bytes_cnt > 18):
                headers_observed += 1
                if DEBUG_PRINT:
                    print "\nHEADER:", headers_observed, "cnt:", bytes_cnt
                bytes_cnt = 0

                if base_axis < 2: # stop at 3 without "consuming" it
                    base_axis = readByte(port)
                    bytes_cnt += 1
                    if DEBUG_PRINT: print "base_axis:", base_axis

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

