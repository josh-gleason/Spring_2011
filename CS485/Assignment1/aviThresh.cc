#include <iostream>
#include <highgui.h>
#include <cv.h>
#include <cvaux.h>

using namespace cv;
using namespace std;

void barCallback(int val, void* params)
{
  *((int*)params) = val;
}

int main(int argc, char *argv[])
{
  // convert and save the videos
  if ( argc < 4 )
  {
    cout << "Please enter one input file and two output files <input> <threshold> <adaptive thresh>" << endl;
    return -1;
  }
 
  cout << "Initializing Videos..." << endl;

  VideoCapture input(argv[1]);

  if (!input.isOpened())
  {
    cout << "Input file did not open" << endl;
    return -1;
  }

  Size s((int)input.get(CV_CAP_PROP_FRAME_WIDTH),(int)input.get(CV_CAP_PROP_FRAME_HEIGHT));

  VideoWriter *out1 = new VideoWriter,
              *out2 = new VideoWriter;
 
  out1->open(argv[2], CV_FOURCC('I','Y','U','V'), (int)input.get(CV_CAP_PROP_FPS), s);
  out2->open(argv[3], CV_FOURCC('I','Y','U','V'), (int)input.get(CV_CAP_PROP_FPS), s);

  if (!out1->isOpened() || !out2->isOpened())
  {
    cout << "An output file did not open" << endl;
    return -1;
  }
  
  Mat frame, gray, color, thresh, adapt;
  
  int frames = static_cast<int>(input.get(CV_CAP_PROP_FRAME_COUNT));
  int delay = static_cast<int>(1000/input.get(CV_CAP_PROP_FPS));

  cout << "Complete" << endl;
  cout << "Converting Videos..." << endl;

  for ( int i = 0; i < frames; i++ )
  {
    input >> frame;
    cvtColor(frame,gray,CV_BGR2GRAY);
    threshold(gray,thresh,70.0,255,THRESH_BINARY);
    adaptiveThreshold(gray,adapt,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
    cvtColor(thresh,color,CV_GRAY2BGR);
    (*out1) << color;
    cvtColor(adapt,color,CV_GRAY2BGR);
    (*out2) << color;
  }

  cout << "Complete!" << endl;
  cout << "Saving videos..." << endl;

  // close/save the videos
  delete out1;
  delete out2;

  cout << "Complete!" << endl;

  // reset input vid
  input.set(CV_CAP_PROP_POS_AVI_RATIO, 0.0);
  
  // Add sliders to window and display videos
  
  namedWindow("Normal Threshold");
  namedWindow("Adaptive Threshold");
  namedWindow("Input");

  int currentFrame = 0, pause = 0;

  createTrackbar("Position", "Input", &currentFrame, frames-1, barCallback, &currentFrame);
  createTrackbar("Pause/Play", "Input" , &pause, 1, barCallback, &pause);

  VideoCapture threshVid(argv[2]), adaptiveVid(argv[3]);

  do {
    input.set(CV_CAP_PROP_POS_FRAMES, currentFrame);
    threshVid.set(CV_CAP_PROP_POS_FRAMES, currentFrame);
    adaptiveVid.set(CV_CAP_PROP_POS_FRAMES, currentFrame);

    input >> frame;
    threshVid >> thresh;
    adaptiveVid >> adapt;

    imshow("Input", frame);
    imshow("Normal Threshold", thresh);
    imshow("Adaptive Threshold", adapt);
    
    if ( pause > 0 )
    {
      if ( currentFrame < frames-1 )
        currentFrame++;
      setTrackbarPos( "Position", "Input", currentFrame );
    }
    // break on key press
  } while ( waitKey(delay) <= 0 );

  return 0;
}
