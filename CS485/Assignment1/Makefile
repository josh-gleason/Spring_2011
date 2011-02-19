all: lkdemo labeler aviThresh cameraThresh capVideo test siftTest

FLAGS = `pkg-config opencv --cflags --libs` -g

lkdemo: lkdemo.cpp
	g++ -o lkdemo lkdemo.cpp $(FLAGS)

labeler: labeler.cc
	g++ -o labeler labeler.cc $(FLAGS)

aviThresh: aviThresh.cc
	g++ -o aviThresh aviThresh.cc $(FLAGS)

test: test.cc
	g++ -o test test.cc $(FLAGS)

cameraThresh: cameraThresh.cc
	g++ -o cameraThresh cameraThresh.cc $(FLAGS)

siftTest: siftTest.cc
	g++ -o siftTest siftTest.cc $(FLAGS)

# used for capturing video from the webcam to use later
capVideo: capVideo.cc
	g++ -o capVideo capVideo.cc $(FLAGS)

clean:
	rm lkdemo labeler aviThresh cameraThresh capVideo

.PHONY: clean
