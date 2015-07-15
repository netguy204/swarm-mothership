# depends on arduino-mk (installable from homebrew and other package managers)

# On OSX (where avr-gcc is harder to come by) depends on the location
# of an Arduino distribution. Yours will almost certainly be different
# than my preferred distribution.

CXXFLAGS+=-I./ArduinoJson

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

#AVR_TOOLS_DIR = /usr
FORCE_MONITOR_PORT = 1
MONITOR_PORT  = /dev/ttyACM0
BOARD_TAG     = uno
MCU           = atmega328p
ISP_PROG      = usbtiny

RESET_CMD     = echo

ifeq ($(uname_S), Linux)
ARDUINO_DIR   = /usr/share/arduino
ARDMK_DIR     = /usr/share/arduino
include /usr/share/arduino/Arduino.mk
else
ARDUINO_DIR   = /Applications/ArduinoDigistump1.5.8C.app/Contents/Resources/Java
ARDMK_DIR     = /usr/local/opt/arduino-mk
include /usr/local/opt/arduino-mk/Arduino.mk
endif


realupload: $(TARGET_HEX)
	avrdude -p $(MCU) -c $(ISP_PROG) -U flash:w:build-uno/test_v1.hex:i
