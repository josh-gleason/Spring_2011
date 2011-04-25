OPT=`pkg-config opencv --cflags --libs` -O2

all: train testa testb 

train: train.cc Makefile eigface.h
	g++ train.cc -o train ${OPT}

testa: testa.cc Makefile eigface.h test.h
	g++ testa.cc -o testa ${OPT}

testb: testb.cc Makefile eigface.h test.h
	g++ testb.cc -o testb ${OPT}

clean:
	rm train testa testb

.PHONY: clean all

