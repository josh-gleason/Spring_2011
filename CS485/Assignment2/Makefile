FLAGS = `pkg-config opencv --cflags --libs` -g

ALL = smooth

all: ${ALL}

smooth: smooth.cc
	g++ -o smooth smooth.cc ${FLAGS}

clean:
	rm ${ALL}

.PHONY: clean all
