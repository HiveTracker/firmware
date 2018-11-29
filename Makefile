BIN_DIR = /tmp/arduino_build

ELF = $(BIN_DIR)/*elf

GDB_SRV = JLinkGDBServer
ARM_GDB = arm-none-eabi-gdb

ARG = -if swd -speed 1000 -device NRF52832_xxAA

TERM = gnome-terminal --
BOARD = sandeepmistry:nRF5:Generic_nRF52832:softdevice=s132

.PHONY: all upload compile pref_list debug startgdbserver local_startgdbserver \
		stopgdbserver clean clean_cache c l d


all: upload


# TODO: make install (upload could depend on it too) examples:
#
# Install AVR board support, 1.6.2
# arduino --install-boards "arduino:avr:1.6.2"
#
# Install Bridge and Servo libraries
# arduino --install-library "Bridge:1.0.0,Servo:1.2.0"


# Documentation:
# https://github.com/arduino/Arduino/blob/master/build/shared/manpage.adoc

OPENOCD=$(HOME)/.arduino15/packages/sandeepmistry/tools/openocd/0.10.0-dev.nrf5/bin/openocd

upload_SD:
	@echo "\n - - -UPLOADING SOFT DEVICE - BE PATIENT - - - \n\n"
	$(OPENOCD) -d2 -f interface/jlink.cfg -c "transport select swd; set WORKAREASIZE 0;" -f target/nrf52.cfg -c "init; halt; nrf51 mass_erase; program {{/home/drix/Arduino/hardware/sandeepmistry/nRF5/cores/nRF5/SDK/components/softdevice/s132/hex/s132_nrf52_2.0.1_softdevice.hex}} verify reset; shutdown"

upload: *.ino *.cpp *.h stopgdbserver clean_cache
	arduino  --pref build.path=$(BIN_DIR) --preserve-temp-files --board $(BOARD) \
			 --upload --verbose-upload --useprogrammer --verbose *.ino

upload_all: upload_SD upload

# optional

compile: *.ino *.cpp *.h clean_cache
	arduino --pref build.path=$(BIN_DIR) --preserve-temp-files --board $(BOARD) \
			--verify --verbose *ino

factory_loop:
	@while true; do \
		read -p "connect new board and press enter to flash test firmware" typedResult; \
		git co test_jig &&   colormake upload_all; \
		mplayer /tmp/notification.mp3; \
		\
		read -p "check BLE and press enter to flash master firmware" typedResult; \
		git co master &&     colormake upload; \
		mplayer /tmp/notification.mp3; \
	done;


# optional
pref_list:
	arduino --get-pref

# TODO: use precise ELF name
debug: $(ELF) startgdbserver
	$(ARM_GDB) $(ELF)

startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || { $(TERM) $(GDB_SRV) $(ARG) & sleep 1 ; }

#optional
local_startgdbserver:
	@pidof $(GDB_SRV) > /dev/null || $(GDB_SRV) $(ARG)

stopgdbserver:
	@pidof $(ARM_GDB) > /dev/null && killall $(ARM_GDB) || true
	@pidof $(GDB_SRV) > /dev/null && killall $(GDB_SRV) || true

clean_cache:
	rm -rf /tmp/arduino_cache*

clean: clean_cache
	rm -rf $(BIN_DIR)

###############################################################################
# Lazy aliases:

c: compile

l: local_startgdbserver

d: debug


