# depends on arduino-mk (installable from homebrew and other package managers)

# On OSX (where avr-gcc is harder to come by) depends on the location
# of an Arduino distribution. Yours will almost certainly be different
# than my preferred distribution.

ARDUINO_DIR   = /Applications/ArduinoDigistump1.5.8C.app/Contents/Resources/Java
ARDMK_DIR     = /usr/local/opt/arduino-mk
#AVR_TOOLS_DIR = /usr
FORCE_MONITOR_PORT = 1
MONITOR_PORT  = /dev/ttyACM0
BOARD_TAG     = uno
MCU           = atmega328
ISP_PROG      = usbtiny

RESET_CMD     = echo


include /usr/local/opt/arduino-mk/Arduino.mk


realupload: $(TARGET_HEX)
	avrdude -p atmega328p -c usbtiny -U flash:w:build-uno/test_v1.hex:i
