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
    cout << "./gauss <input> <output1> <output2>" << endl;
    return -1;
  }
  
  Mat img = imread(argv[1],0);

  Mat pyramid1[6];
  Mat pyramid2[6];
  Mat showA,showB;

  img.convertTo(pyramid1[0],CV_32FC1);

  pyramid2[0] = pyramid1[0];

  float sigma1 = 1;
  float sigma2 = sigma1;
  int window = roundOdd(5*sigma1);
  
  for ( int i = 0; i < 5; i++ )
    Blur(pyramid1[i],pyramid1[i+1],window,sigma1);

  for ( int i = 0; i < 5; i++ )
  {
    window = roundOdd(5*sigma2*sqrt(i+1));
    Blur(pyramid2[0],pyramid2[i+1],window,sigma2*sqrt(i+1));
  }
  
  ostringstream sout;
  float mse;
  for ( int index = 0; index < 6; index++ )
  {
    mse = 0.0;

    // normalize and equalize histograms to compare better
    normalize(pyramid1[index],showA,0.0,255.0,CV_MINMAX,CV_8UC1);
    equalizeHist(showA,pyramid1[index]);

    normalize(pyramid2[index],showB,0.0,255.0,CV_MINMAX,CV_8UC1);
    equalizeHist(showB,pyramid2[index]);

    // calculate mean square error
    for ( int i = 0; i < pyramid1[index].rows; i++ )
      for ( int j = 0; j < pyramid2[index].cols; j++ )
        mse += pow((float)(pyramid1[index].at<uchar>(i,j))-
                   (float)(pyramid2[index].at<uchar>(i,j)),2);

    mse /= pyramid1[index].rows*pyramid2[index].cols;
    
    cout << "Mean Square Error Level " << index+1 << ": " << mse << endl;
    sout.str("");
    sout << argv[2] << index << ".jpg";

    cout << sout.str().c_str() << endl;

    imwrite(sout.str().c_str(),showA);
    sout.str("");
    sout << argv[3] << index << ".jpg";
    
    cout << sout.str().c_str() << endl;
   
    imwrite(sout.str().c_str(),showB);
    
    imshow("Method 1",showA);
    imshow("Method 2",showB);
    waitKey(0);
  }

  return 0;
}
