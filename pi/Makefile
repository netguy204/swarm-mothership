all:  ptest pi2cface #piface

#piface: piface.cpp protocol.cpp

CXXFLAGS+=-std=c++0x -I../ -I../mothership_avr -I../ArduinoJson/include -Wall -Werror -pedantic
LDLIBS+=-lrt -lcurl -lArduinoJson
LDFLAGS+=-L../ArduinoJson/lib

OBJS=../protocol.o joystick.o systemtime.o pfsm.o upstream.o wsfsm.o
pi2cface: pi2cface.cpp ../mothership_avr/protocol.cpp $(OBJS)

ptest: ptest.cpp ../mothership_avr/protocol.cpp

wtest: wtest.cpp systemtime.o upstream.o wsfsm.o

clean:
	@rm -f pi2cface ptest *.o
