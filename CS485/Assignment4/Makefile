FLAGS=`pkg-config opencv --cflags --libs` -g -O2

main: main.cc readsettings.h
	g++ -o main main.cc ${FLAGS}

clean:
	rm main
