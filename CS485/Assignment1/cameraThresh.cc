#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cvaux.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
  VideoCapture input(0);

  if (!input.isOpened())
  {
    cout << "Camera did not open" << endl;
    return -1;
  }
  
  input.set(CV_CAP_PROP_FRAME_WIDTH,320.0);
  input.set(CV_CAP_PROP_FRAME_HEIGHT,240.0);

  namedWindow("Normal Threshold");
  namedWindow("Adaptive Threshold");
  namedWindow("Input");

  Mat frame, gray, thresh, adapt;
  
  do {
    input >> frame;
    cvtColor(frame,gray,CV_BGR2GRAY);
    threshold(gray,thresh,90.0,255,THRESH_BINARY);
    adaptiveThreshold(gray,adapt,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
    imshow("Input", frame);
    imshow("Normal Threshold", thresh);
    imshow("Adaptive Threshold", adapt);
  } while (waitKey(58) <= 0);

  input.release();
  return 0;
}
