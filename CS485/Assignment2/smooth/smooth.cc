#include <iostream>
#include <cmath>
#include <iomanip>
#include "funcs.h"

using namespace std;

// dont show images
#define imshow(a,b); ;
#define waitKey(a); ;

int main(int argc, char *argv[])
{
  if ( argc < 5 )
  {
    cout << "Command line arguments are as follows..." << endl
         << "<windowsize> <input> <output1> <output2>" << endl
         << "sigma must be an interger value" << endl
         << "output1 is the input blurred using seperable kernels" << endl
         << "output2 is the input blurred using OpenCV's GaussianBlur function" << endl;
    return -1;
  }

  float sigma = atoi(argv[1])/5.;

  Mat xKern,yKern,xGauss,smooth,show1,show2,cmp,
    img = imread(argv[2],0), fImg;

  // convert to floating point
  img.convertTo(fImg,CV_32FC1);

  // build kernels
  kerns(atoi(argv[1]),atoi(argv[1])/5.,xKern,yKern);

  // filter image
  filter(fImg,xGauss,xKern);
  filter(xGauss,smooth,yKern);
  
  // create comparison image
  if ( (int)(5*sigma) % 2 != 0 )
    GaussianBlur(img,cmp,cvSize(5*sigma,5*sigma),sigma,sigma,BORDER_CONSTANT);
  else
    cmp = smooth;

  // display
  normalize(smooth,show1,0,255,CV_MINMAX,CV_8UC1);
  imshow("My Blur",show1);  

  normalize(cmp,show2,0,255,CV_MINMAX,CV_8UC1);
  imshow("OpenCV's Blur",show2);

  // save results
  imwrite(argv[3],show1);
  imwrite(argv[4],show2);

  waitKey(0);

  // calculate mean square error
  float mse = 0.0;
  for ( int i = 0; i < img.rows; i++ )
    for ( int j = 0; j < img.cols; j++ )
      mse += pow((float)(show1.at<uchar>(i,j))-(float)(show2.at<uchar>(i,j)),2);
  mse /= img.rows*img.cols;

  cout << "Mean Square Error: " << mse << endl;
  return 0;
}
