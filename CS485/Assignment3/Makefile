FLAGS=`pkg-config opencv --cflags --libs` -g

main: main.cc FaceCoordinates.txt
	g++ -o main main.cc ${FLAGS}

clean:
	rm main
