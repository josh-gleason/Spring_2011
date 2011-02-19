#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <queue>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

// class to pass to the mouse callback function
struct ParamSet
{
    ParamSet() : mouseDown(false) {}
    Mat orig; // holds the original image
    Mat temp; // temporary image
    string winName; // name of the image window
    Rect rect; // the rectangle that is to be drawn
    queue<Rect> rectList;
    bool mouseDown;

    void pushRect()
    {
      // adjust so that negative heights/widths are saved
      if ( rect.width < 0 )
      {
        rect.x += rect.width;
        rect.width *= -1;
      }
      if ( rect.height < 0 )
      {
        rect.y += rect.height;
        rect.height *= -1;
      }
      rectList.push(rect);
    }

    void popRect()
    {
      rect = rectList.front();
      rectList.pop();
    }

    void drawRectOrig()  // draw the current rectangle
    {
      rectangle( orig, rect.tl(), rect.br(), CV_RGB(125,255,70) );
    }
    
    void drawRectTemp()  // draw the current rectangle
    {
      rectangle( temp, rect.tl(), rect.br(), CV_RGB(125,255,70) );
    }

    void setTopLeft(int _x, int _y)
    {
      rect.x = _x;
      rect.y = _y;
    }

    void setBottomRight(int _x, int _y)
    {
      rect.width = _x-rect.x;
      rect.height = _y-rect.y;
    }

    void showOrig()
    {
      imshow( winName.c_str(), orig );
    }

    void showTemp()
    {
      imshow( winName.c_str(), temp );
    }
};

void mouseEvent( int event, int x, int y, int flags, void* params )
{
  // point to the function parameters
  ParamSet* info = (ParamSet*)params;
  
  switch ( event )
  {
    case CV_EVENT_MOUSEMOVE:
      if ( info->mouseDown )  // so you can see your rectangles as you draw
      {
        info->setBottomRight(x,y);
        info->temp = info->orig.clone();
        info->drawRectTemp();
        info->showTemp();
      }
      break;
    case CV_EVENT_LBUTTONDOWN:  // save the point as the corner of the new rectangle
      info->mouseDown = true;
      info->setTopLeft(x,y);
      break;
    case CV_EVENT_LBUTTONUP:  // draw and save the rectangle to the queue
      if ( info->mouseDown )
      {
        info->mouseDown = false;
        info->setBottomRight(x,y);
        info->temp = info->orig.clone();
        info->drawRectOrig();
        info->pushRect();     // push rectangle to queue
        info->showOrig();
      }
      break;
  }

}

int main(int argc, char *argv[])
{
  if ( argc < 3 )
  {
    cout << "Please provide an image file and output file." << endl;
    return 0;
  }

  ParamSet info;  // holds all the parameters of the program
  info.orig = imread(argv[1]);
  info.temp = info.orig.clone();  // temporary image to draw intermediate rectangles in
  info.winName = "Image";
  info.mouseDown = false;

  namedWindow(info.winName.c_str());
  imshow(info.winName.c_str(),info.orig);

  // register callback function for mouse event
  cvSetMouseCallback( info.winName.c_str(), mouseEvent, (void*)(&info) );

  char key;
  // main loop
  do {
    key = waitKey(0);
  } while ( key != 27 );  // esc key

  // save locations
  ofstream fout(argv[2]);

  if ( !fout.good() )
  {
    cout << argv[2] << " can not be opened for output!" << endl;
    return 0;
  }

  while ( !info.rectList.empty() )
  {
    info.popRect();
    fout << info.rect.x << ' ' << info.rect.y << ' ' << info.rect.width << ' ' << info.rect.height << endl;
  }

  fout.close();

  return 0;
}
