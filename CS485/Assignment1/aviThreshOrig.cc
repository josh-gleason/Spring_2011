#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cvaux.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
  if ( argc < 4 )
  {
    cout << "Please enter one input file and two output files <input> <threshold> <adaptive thresh>" << endl;
    return -1;
  }
  
  VideoCapture input(argv[1]);

  if (!input.isOpened())
  {
    cout << "Input file did not open" << endl;
    return -1;
  }

  Size s((int)input.get(CV_CAP_PROP_FRAME_WIDTH),(int)input.get(CV_CAP_PROP_FRAME_HEIGHT));

  VideoWriter out1(argv[2], CV_FOURCC('I','Y','U','V'), (int)input.get(CV_CAP_PROP_FPS), s),
              out2(argv[3], CV_FOURCC('I','Y','U','V'), (int)input.get(CV_CAP_PROP_FPS), s);

  if (!out1.isOpened() || !out2.isOpened())
  {
    cout << "An output file did not open" << endl;
    return -1;
  }
  
  namedWindow("Normal Threshold");
  namedWindow("Adaptive Threshold");
  namedWindow("Input");

  cout << "Press a key with a window in focus to begin!" << endl;

  waitKey(0);

  Mat frame, gray, color, thresh, adapt;
  
  int frames = static_cast<int>(input.get(CV_CAP_PROP_FRAME_COUNT));
  int delay = static_cast<int>(1000/input.get(CV_CAP_PROP_FPS));

  for ( int i = 0; i < frames; i++ )
  {
    input >> frame;
    cvtColor(frame,gray,CV_BGR2GRAY);
    threshold(gray,thresh,70.0,255,THRESH_BINARY);
    adaptiveThreshold(gray,adapt,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
    cvtColor(thresh,color,CV_GRAY2BGR);
    out1 << color;
    cvtColor(adapt,color,CV_GRAY2BGR);
    out2 << color;
    imshow("Input", frame);
    imshow("Normal Threshold", thresh);
    imshow("Adaptive Threshold", adapt);
    waitKey(delay);
  }

  input.release();
  return 0;
}
