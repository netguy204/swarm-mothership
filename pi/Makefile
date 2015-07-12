all:  ptest pi2cface wtest


CXXFLAGS+=-std=c++0x -I../ -I../mothership_avr -I../ArduinoJson/include -Wall -Werror -pedantic

LDLIBS+=-lrt -lcurl -lArduinoJson

LDFLAGS+=-L../ArduinoJson/lib


OBJS=../mothership_avr/protocol.o joystick.o systemtime.o pfsm.o upstream.o wsfsm.o wsmsg.o


pi2cface: pi2cface.cpp $(OBJS)

ptest: ptest.cpp ../mothership_avr/protocol.cpp

wtest: wtest.cpp systemtime.o upstream.o wsmsg.o wsfsm.o ../mothership_avr/protocol.cpp

clean:
	@rm -f pi2cface ptest wtest *.o
