#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include "readsettings.h"

using namespace std;
using namespace cv;

// returns sigma max
// img should be of type CV_32FC1, as will the Mats in gauss and doG
float buildPyramids( const Mat& img, float sigma0, float k, int levels,
  vector<Mat>& gauss, vector<Mat>& doG )
{
  gauss.clear();
  doG.clear();

  float sigma = sigma0;
  int win;
  Mat blurred;
  for ( int i = 0; i < levels; i++ )
  {
    win = sigma * 5;
    (win % 2 == 0 ? win-- : true);
    
    if ( i == 0 )
      cout << "\nGaussian\nWindow size :" << win << endl << "Sigma : " << sigma << endl;

    GaussianBlur(img, blurred, Size(win,win), sigma, sigma);
    gauss.push_back(blurred.clone());

    sigma *= k;
  }

  for ( int i = 0; i < levels-1; i++ )
  {
    blurred = gauss[i] - gauss[i+1];
    doG.push_back(blurred.clone());
  }

  return sigma / k;
}
  
void harrisPyramid(const Mat& img, float sigma0, float k, int levels, float alpha,
  vector<Mat>& harris)
{
  harris.clear();

  float sigmaD, sigmaI;
  int neighSize, maskSize;
  Mat corners;

  for ( int i = 0; i < levels; i++ )
  {
    sigmaI = sigma0;
    sigmaD = 0.7 * sigmaI;
    sigma0 *= k;
    
    neighSize = 5*sigmaI;
    maskSize = 5*sigmaD;

    (neighSize % 2 == 0 ? neighSize-- : true);
    (maskSize % 2 == 0 ? maskSize-- : true);
    (maskSize > 31 ? maskSize = 31 : true);

    if ( i == 0 )
      cout << "\nHarris\nneigh size: " << neighSize << endl
           << "mask size: " << maskSize << endl
           << "alpha: " << alpha << endl;

    cornerHarris(img,corners,neighSize,maskSize,alpha);

    harris.push_back(corners.clone());
  }
}

void findMaxima(const vector<Mat>& harris, float t1, vector<vector<Point> >& harrisMaxima)
{
  // assume all Mat in harris are same size
  int size = static_cast<int>(harris.size());
  
  int rmax = harris[0].rows - 1;
  int cmax = harris[0].cols - 1;
  bool largest;
  float val;

  // there is a hard coded 3x3 window
  for ( int i = 0; i < size; i++ )
  {
    // push new vector onto back
    harrisMaxima.push_back(vector<Point>());
    for ( int r = 1; r < rmax; r++ )
      for ( int c = 1; c < cmax; c++ )
      {
        val = harris[i].at<float>(r,c);

        // check for threshold
        if ( abs(val) < t1 )
          largest = false;
        else
          largest = true;

        // wont enter loop if largest is false
        for ( int rw = r-1; rw <= r+1 && largest; rw++ )
          for ( int cw = c-1; cw <= c+1 && largest; cw++ )
            if ( rw != r || cw != c )
              if ( harris[i].at<float>(rw,cw) > val )
                largest = false;
        
        // if largest then push back
        if ( largest )
          harrisMaxima[i].push_back(Point(c,r));
      }
  }
}

void doGMaxima( const vector<Mat>& doG, const vector<vector<Point> >& maxima,
  float t2, vector<vector<Point> >& dogMaxima )
{
  int maxLevel = static_cast<int>(doG.size())-1;
  int numPoints;
  float val;

  Point currentPoint;
  int cX, cY;
  bool largest = true;

  for ( int i = 1; i < maxLevel; i++ )
  {
    dogMaxima.push_back(vector<Point>());
    numPoints = static_cast<int>(maxima[i+1].size());
    for ( int j = 0; j < numPoints; j++ )
    {
      cX = maxima[i+1][j].x;
      cY = maxima[i+1][j].y;
      
      val = doG[i].at<float>(cY,cX);

      if ( abs(val) < t2 )
        largest = false;
      else
        largest = true;

      for ( int x = cX-1; x <= cX+1 && largest; x++ )
        for ( int y = cY-1; y <= cY+1 && largest; y++ )
          for ( int z = -1; z <= 1 && largest; z++ )
            if ( x != 0 || y != 0 || z != 0 )
              if ( val < doG[i-z].at<float>(y,x) )
                largest = false;

      if ( largest )
        dogMaxima[i-1].push_back( maxima[i+1][j] );
    }
  }
}

int main(int argc, char *argv[])
{
  if ( argc < 2 )
  {
    cout << "Please input the settings file" << endl;
    return -1;
  }

  Settings s(argv[1]);

  Mat img8bit = imread(s.imagename,0);
  Mat img;
  vector<Mat> doG, gauss, harris;
  vector<vector<Point> > harrisMaxima, dogMaxima;
  float sigmaMax;

  // convert to floating point
  img8bit.convertTo(img,CV_32FC1);

  // build pyramids
  sigmaMax = buildPyramids(img, s.sigma0, s.k, s.levels, gauss, doG);

  // TODO: temporary
  //for ( int i = 0; i < doG.size(); i++ )
  //{
  //  Mat temp;
  //  normalize(doG[i],temp,0.0,255.0,CV_MINMAX,CV_8UC1);
  //  imshow("",temp);
  //  waitKey(0);
  //}
  //TODO: end temporary

  // run Harris on gaussian
  harrisPyramid(img, s.sigma0, s.k, s.levels, s.alpha, harris);

  // find maxima (maxima)
  findMaxima(harris, s.t1, harrisMaxima);

  // verify maxima using the DoG pyramid
  doGMaxima(doG, harrisMaxima, s.t2, dogMaxima);

  // TODO: temporary
  // DISPLAY RESULTS THUS FAR
  for ( int i = 0; i < dogMaxima.size(); i++ )
    cout << i << " : " << dogMaxima[i].size() << endl;
  
  Mat colorImg = imread(s.imagename,1);

  for ( int i = 0; i < dogMaxima.size(); i++ )
    for ( int j = 0; j < dogMaxima[i].size(); j++ )
    {
      circle(colorImg,dogMaxima[i][j],s.sigma0*pow(s.k,i+1)*sqrt(2.0),Scalar(0,255,0));
    }
  imshow("",colorImg);
  waitKey(0);
  // TODO: end temporary
  
  return 0;
}

