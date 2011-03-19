FLAGS = `pkg-config opencv --cflags --libs` -I./

ALL = smooth/smooth gauss/gauss laplacian/laplacian edges/edges sobel/sobel

all: ${ALL}

smooth/smooth: smooth/smooth.cc
	g++ -o smooth/smooth smooth/smooth.cc ${FLAGS}

gauss/gauss: gauss/gauss.cc
	g++ -o gauss/gauss gauss/gauss.cc ${FLAGS}

laplacian/laplacian: laplacian/laplacian.cc
	g++ -o laplacian/laplacian laplacian/laplacian.cc ${FLAGS}

edges/edges: edges/edges.cc
	g++ -o edges/edges edges/edges.cc ${FLAGS}

sobel/sobel: sobel/sobel.cc
	g++ -o sobel/sobel sobel/sobel.cc ${FLAGS}

clean:
	rm ${ALL}

.PHONY: clean all
