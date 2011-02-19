#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cvaux.h>

using namespace cv;
using namespace std;

const int FRAMES = 9;

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

  namedWindow("Input");

  // sift test
  SIFT sift;
  vector<KeyPoint> keypoints; 
  Mat frame, mask(320,240,CV_8UC1), img(320,240,CV_8UC1);
  mask = cvScalar(1.0);
  for ( int i = 100; i < 200; i++ )
    for ( int j = 100; j < 200; j++ )
      mask = cvScalar(0.0);

  do {
    input >> frame;
    cvtColor(frame,img,CV_BGR2GRAY);
    keypoints.clear();
    sift(img, mask, keypoints);
    for ( int i = 0; i < keypoints.size(); i++ )
    {
      circle(frame,keypoints[i].pt,2,CV_RGB(0,200,0),-1); 
    }
    imshow("Input", frame);
  } while( waitKey(200) <= 0 );
  input.release();

  return 0;
}
