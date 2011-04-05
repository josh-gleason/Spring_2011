ALL = bettermult multiplier multnothread

all: ${ALL}

multiplier: multiplier.cc
	g++ multiplier.cc -o multiplier -pthread -O2

bettermult: bettermult.cc
	g++ bettermult.cc -o bettermult -pthread -O2

multnothread: multnothread.cc
	g++ multnothread.cc -o multnothread -O2

clean:
	rm ${ALL}
