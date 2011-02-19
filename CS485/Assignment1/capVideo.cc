#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cvaux.h>

using namespace cv;

int main(int argc, char *argv[])
{
  VideoCapture cap(0);
  
  if (!cap.isOpened())
    return -1;
  
  cap.set(CV_CAP_PROP_FRAME_WIDTH,320.0);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT,240.0);

  Size s((int)cap.get(CV_CAP_PROP_FRAME_WIDTH),(int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));

  VideoWriter out(argv[1],CV_FOURCC('I','Y','U','V'), 25, s);

  if ( !out.isOpened() )
    return -1;

  Mat frame;
  namedWindow("edges",1);
  while ( waitKey(5) <= 0 )
  {
    cap >> frame;
    imshow("edges", frame);
    out << frame;
  }
  cap.release();
  return 0;
}
