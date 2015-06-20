all:  ptest pi2cface #piface

#piface: piface.cpp protocol.cpp
pi2cface: pi2cface.cpp protocol.cpp
ptest: ptest.cpp protocol.cpp

clean:
	rm -f piface *.o
