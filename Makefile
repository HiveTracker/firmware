BIN = /tmp/arduino_build*

ELF = $(BIN)/*elf

GDB_SRV = /usr/bin/JLinkGDBServer

ARG = -if swd -speed 1000 -device NRF52832_xxAA

TERM = gnome-terminal -e
BOARD = sandeepmistry:nRF5:Generic_nRF52832:softdevice=s132

.PHONY: all upload compile pref_list debug startgdbserver local_startgdbserver stopgdbserver clean


all: upload


# TODO: make install! upload could depend on it too...


# Documentation:
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

upload: *.ino *.cpp *.h stopgdbserver clean
	arduino --preserve-temp-files --board $(BOARD) --upload --verbose-upload --useprogrammer --verbose *.ino

# optional
compile: *.ino *.cpp *.h clean
	arduino --verify --preserve-temp-files --board $(BOARD) -v *ino

# optional
pref_list:
	arduino --get-pref

# TODO: use precise ELF name
debug: $(ELF) startgdbserver
	arm-none-eabi-gdb $(ELF)

startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || { $(TERM) "$(GDB_SRV) $(ARG)" & sleep 1 ; }

#optional
local_startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || $(GDB_SRV) $(ARG)

stopgdbserver:
	@pidof arm-none-eabi-gdb > /dev/null && killall arm-none-eabi-gdb || true
	@pidof JLinkGDBServer > /dev/null && killall $(GDB_SRV) || true

clean:
	rm -rf $(BIN)/*

