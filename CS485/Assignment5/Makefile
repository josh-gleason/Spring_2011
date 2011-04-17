OPT=`pkg-config opencv --cflags --libs` -g

all: train test

train: train.cc Makefile eigface.h
	g++ train.cc -o train ${OPT}

test: test.cc Makefile eigface.h
	g++ test.cc -o test ${OPT}

clean:
	rm train test

.PHONY: clean all

