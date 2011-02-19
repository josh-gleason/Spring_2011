#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
  int delay;
  if ( argc < 2 )
    delay = 60;
  else
    delay = atoi(argv[1]);

  VideoCapture camera(0);

  if ( !camera.isOpened() ) 
  {
    cout << "Camera did not open" << endl;
    return -1;
  }

  camera.set(CV_CAP_PROP_FRAME_WIDTH, 320.0);
  camera.set(CV_CAP_PROP_FRAME_HEIGHT, 240.0);

  namedWindow("Camera");

  Mat frame, blur;

  do {
    camera >> frame;
    GaussianBlur(frame,blur,cvSize(5,5),0,3);
    //Sobel(blur,frame,5,1,1);
    imshow("Camera", frame);
  } while ( waitKey(delay) <= 0 );

  return 0;
}
