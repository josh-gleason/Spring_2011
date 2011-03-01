#include <iostream>
#include <sstream>
#include "funcs.h"

using namespace std;
using namespace cv;

// dont show images
#define imshow(a,b); ;
#define waitKey(a); ;

int main(int argc, char *argv[])
{

  if ( argc < 4 )
  {
    cout << "./laplacian <input> <output1> <output2>" << endl;
    return -1;
  }
  
  Mat img = imread(argv[1],0);
  Mat gaussPyr[6], lapPyr1[5], lapPyr2[5];
  Mat temp,temp2;

  float kernVals[9] = {-1./8,-1./8,-1./8,
                       -1./8,  1. ,-1./8,
                       -1./8,-1./8,-1./8};
  Mat lapKern(3,3,CV_32FC1,(void*)kernVals);
  
  img.convertTo(gaussPyr[0],CV_32FC1);

  float sigma = 1;
  int window = roundOdd(5*sigma);
  
  // build Gaussian pyramid
  for ( int i = 0; i < 6; i++ )
    Blur(gaussPyr[i],gaussPyr[i+1],window,sigma);

  for ( int i = 0; i < 5; i++ )
  {
    filter(gaussPyr[i],temp,lapKern);
    normalize(temp,lapPyr1[i],0.0,255.0,CV_MINMAX);
    lapPyr2[i] = gaussPyr[i]-gaussPyr[i+1];
  }

  ostringstream sout;
  for ( int i = 0; i < 5; i++ )
  {
    sout.str("");
    sout << argv[2] << i << ".jpg";
    normalize(lapPyr1[i],temp,0.0,255,CV_MINMAX,CV_8UC1);
    imwrite(sout.str().c_str(),temp);
    imshow("Image1",temp);
    sout.str("");
    sout << argv[3] << i << ".jpg";
    normalize(lapPyr2[i],temp,0.0,255,CV_MINMAX,CV_8UC1);
    imwrite(sout.str().c_str(),temp);
    imshow("Image2",temp);
    waitKey(0);
  }

  return 0;
}
