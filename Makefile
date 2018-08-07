BIN = /tmp/arduino_build*

ELF = $(BIN)/*elf

GDB_SRV = /usr/bin/JLinkGDBServer

ARG = -if swd -speed 1000 -device NRF52832_xxAA

TERM = gnome-terminal -e
BOARD = sandeepmistry:nRF5:Generic_nRF52832:softdevice=s132

.PHONY: all debug startgdbserver startgdbserver_here clean


all: upload

# Documentation:
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

upload: stopgdbserver
	arduino --preserve-temp-files --upload --verbose-upload --useprogrammer --verbose *.ino

# optional
compilation: *.ino *.cpp *.h
	arduino --verify --preserve-temp-files --board $(BOARD) -v *ino

# optional
pref_list:
	arduino --get-pref

debug: $(ELF) startgdbserver
	arm-none-eabi-gdb $(ELF)

startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || { $(TERM) "$(GDB_SRV) $(ARG)" & sleep 1 ; }

#optional
local_startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || $(GDB_SRV) $(ARG)

stopgdbserver:
		@pidof JLinkGDBServer > /dev/null && killall $(GDB_SRV) || true

clean:
	rm -rf $(BIN)/*

