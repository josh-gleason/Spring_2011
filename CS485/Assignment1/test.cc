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

  Mat frame, fframe[FRAMES], tot(240.0,320.0, CV_32FC3);
  
  tot = 0.0;

  for ( int i = 0; i < FRAMES; i++ )
  {
    input >> frame;
    frame.convertTo(fframe[i], CV_32FC3, 1.0/FRAMES);
  }

  int c = 0;
  
  Mat norm;
  
  // sift test
  SIFT sift;
  vector<KeyPoint> keypoints; 
  Mat mask(320,240,CV_8UC1), img(320,240,CV_8UC1);
  mask = cvScalar(0.0);
  
  cout << img.type() << endl;
  
  cvtColor(frame,img,CV_BGR2GRAY);
  
  cout << img.type() << endl;

  sift(img, mask, keypoints);

  cout << keypoints.size() << endl;

  for ( int i = 0; i < keypoints.size(); i++ )
  {
    cout << keypoints[i].pt << ' ' << keypoints[i].size << ' ' << keypoints[i].response << endl;
    if ( keypoints[i].size > 10 && keypoints[i].size < 50 )
    circle(frame,keypoints[i].pt,keypoints[i].size,CV_RGB(0,200,0),1); 
  }

  imshow("Image", frame);
  cvWaitKey(0);
  /* 
  do {
    input >> frame;
    frame.convertTo(fframe[c], CV_32FC3, 1.0/FRAMES);
    tot = fframe[0];
    for ( int i = 1; i < FRAMES; i++ )
      tot += fframe[i];
    tot.convertTo(frame, CV_8UC3);
    tot /= FRAMES;
    imshow("Input", frame);
    
    c++;
    if ( c >= FRAMES )
      c = 0;
  } while( waitKey(56) <= 0 );
  */
  input.release();

  return 0;
}
