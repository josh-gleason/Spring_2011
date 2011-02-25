FLAGS = `pkg-config opencv --cflags --libs` -g

ALL = smooth gauss laplacian edges

all: ${ALL}

smooth: smooth.cc
	g++ -o smooth smooth.cc ${FLAGS}

gauss: gauss.cc
	g++ -o gauss gauss.cc ${FLAGS}

laplacian: laplacian.cc
	g++ -o laplacian laplacian.cc ${FLAGS}

edges: edges.cc
	g++ -o edges edges.cc ${FLAGS}

clean:
	rm ${ALL}

.PHONY: clean all
