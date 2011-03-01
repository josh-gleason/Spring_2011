#include <sstream>
#include <iostream>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

using namespace std;
using namespace cv;

// dont show images
#define imshow(a,b); ;
#define waitKey(a); ;

int main(int argc, char *argv[])
{
  if ( argc < 4 )
  {
    cout << "./sobel <input> <threshold> <output>" << endl;
    return -1;
  }

  float sobelYdata[] = {-1,-2,-1,
                         0, 0, 0,
                         1, 2, 1};
  float sobelXdata[] = {-1,0,1,
                        -2,0,2,
                        -1,0,1};

  Mat kernX = Mat(Size(3,3),CV_32FC1,sobelXdata);
  Mat kernY = Mat(Size(3,3),CV_32FC1,sobelYdata);

  Mat img = imread(argv[1],0);
  Mat imgx,imgy,mag,dir,thresh;

  filter2D(img,imgx,CV_32FC1,kernX);
  filter2D(img,imgy,CV_32FC1,kernY);
  sqrt(imgx.mul(imgx)+imgy.mul(imgy),mag);
  dir = Mat(img.rows,img.cols,CV_32FC1);
  thresh = Mat(img.rows,img.cols,CV_8UC1);
  float tVal = atoi(argv[2]);
  for ( int r = 0; r < img.rows; r++ )
    for ( int c = 0; c < img.cols; c++ )
    {
      if ( mag.at<float>(r,c) > tVal )
        thresh.at<uchar>(r,c) = (uchar)255;
      else
        thresh.at<uchar>(r,c) = (uchar)0;
      dir.at<float>(r,c) = atan2(imgy.at<float>(r,c),imgx.at<float>(r,c));
    }

  Mat s1,s2,s3,s4,temp;
  temp = abs(imgx);
  normalize(temp,s1,0,255,CV_MINMAX,CV_8UC1);
  temp = abs(imgy);
  normalize(temp,s2,0,255,CV_MINMAX,CV_8UC1);
  normalize(mag,s3,0,255,CV_MINMAX,CV_8UC1);
  normalize(dir,s4,0,255,CV_MINMAX,CV_8UC1);

  imshow("Original",img);
  imshow("Sobel X",s1);
  imshow("Sobel Y",s2);
  imshow("Magnitude",s3);
  imshow("Direction",s4);
  imshow("Edges",thresh);

  ostringstream sout;
  sout.str("");
  sout << argv[3] << "sobelx.jpg";
  imwrite(sout.str().c_str(),s1);
  sout.str("");
  sout << argv[3] << "sobely.jpg";
  imwrite(sout.str().c_str(),s2);
  sout.str("");
  sout << argv[3] << "mag.jpg";
  imwrite(sout.str().c_str(),s3);
  sout.str("");
  sout << argv[3] << "dir.jpg";
  imwrite(sout.str().c_str(),s4);
  sout.str("");
  sout << argv[3] << "edges.jpg";
  imwrite(sout.str().c_str(),thresh);

  waitKey();

  return 0;
}
