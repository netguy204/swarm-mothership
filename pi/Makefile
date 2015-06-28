all:  ptest pi2cface #piface

#piface: piface.cpp protocol.cpp
CXXFLAGS=-I../ -Wall -Werror -pedantic
pi2cface: pi2cface.cpp ../protocol.cpp
ptest: ptest.cpp ../protocol.cpp

clean:
	@rm -f pi2cface ptest *.o
