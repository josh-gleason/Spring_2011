#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include "readsettings.h"

using namespace std;
using namespace cv;

//#define __showdog
//#define __showdogmax
#define __showdogpoints

void findMinMax( const Mat& img, float& minVal, float& maxVal )
{
  minVal = maxVal = img.at<float>(0,0);
  float val;
  int r, c;
  for ( r = 0; r < img.rows; r++ )
    for ( c = 0; c < img.cols; c++ )
    {
      val = img.at<float>(r,c);
      if ( minVal > val )
        minVal = val;
      if ( maxVal < val )
        maxVal = val;
    }
}

// run harris corner detection
void harrisDetector(const Mat& img,Mat& corners,float sigmaD,float sigmaI,float alpha)
{
  Mat blurred, imgX, imgY, imgXX, imgYY, imgXY;
  Mat sobelX(3,3,CV_32FC1,Scalar(0)), sobelY;
  sobelX.at<float>(0,0) = -1.0;
  sobelX.at<float>(1,0) = -2.0;
  sobelX.at<float>(2,0) = -1.0;
  sobelX.at<float>(0,2) = 1.0;
  sobelX.at<float>(1,2) = 2.0;
  sobelX.at<float>(2,2) = 1.0;

  transpose(sobelX,sobelY);

  int win = sigmaD*6;
  if ( win % 2 == 0 )
    win++;
  
  // blur image before differentiation (sigmaD)
  GaussianBlur(img,blurred,Size(win,win),sigmaD);

  // apply Sobel (differentiation)
  filter2D(blurred,imgX,CV_32FC1,sobelX);
  filter2D(blurred,imgY,CV_32FC1,sobelY);

  // multiply then blur (sigmaI)
  win = sigmaI*6;
  if ( win % 2 == 0 )
    win++;

  GaussianBlur(imgX.mul(imgY),imgXY,Size(win,win),sigmaI);
  GaussianBlur(imgX.mul(imgX),imgXX,Size(win,win),sigmaI);
  GaussianBlur(imgY.mul(imgY),imgYY,Size(win,win),sigmaI);

  // create cornerness image
  corners = Mat(img.size(), CV_32FC1);

  // calculate cornerness values
  Mat A(2,2,CV_32FC1);
  for ( int r = 0; r < img.rows; r++ )
    for ( int c = 0; c < img.cols; c++ )
    {
      A.at<float>(0,0) = imgXX.at<float>(r,c);
      A.at<float>(1,1) = imgYY.at<float>(r,c);
      A.at<float>(1,0) = imgXY.at<float>(r,c);
      A.at<float>(0,1) = imgXY.at<float>(r,c);
      corners.at<float>(r,c) = determinant(A) - alpha * pow(trace(A)[0],2);
    }
}

// returns sigma max
// img should be of type CV_32FC1, as will the Mats in gauss and doG
void buildDoG( const Mat& img, vector<Mat>& doG, const Settings& s )
{
  // actually start lower than sigma0 for comparison later
  float sigma = s.sigma0 * pow(s.k, -s.doGRegionZ/2);

  // need this many levels to produce s.levels of COMPARABLE DoGs
  int dlevels = s.levels + s.doGRegionZ - 1;

  // round to nearest odd number
  int win = round(s.winMult * sigma);
  if ( win % 2 == 0 )
    win++;

  // gauss1 has sigma less than gauss2
  Mat gauss1, gauss2, doGImg;

  // build first layer
  GaussianBlur(img,gauss1,Size(win,win),sigma);

  float minVal, maxVal, minValt, maxValt;

  for ( int l = 0; l < dlevels; l++ )
  {
    // calculate next sigma
    sigma *= s.k;
    win = round(s.winMult * sigma);
    if ( win % 2 == 0 )
      win++;
    
    // build next layer
    GaussianBlur(img,gauss2,Size(win,win),sigma);

    doGImg = gauss2 - gauss1;

    // push doG back to vector
    doG.push_back(doGImg.clone());

    findMinMax(doGImg, minValt, maxValt);
    if ( l == 0 )
    {
      minVal = minValt;
      maxVal = maxValt;
    }
    else
    {
      if ( minVal > minValt )
        minVal = minValt;
      if ( maxVal < maxValt )
        maxVal = maxValt;
    }
#ifdef __showdog
    Mat temp;
    normalize(doGImg,temp,0.0,255.0,CV_MINMAX,CV_8UC1);
    imshow("",temp);
    cout << sigma/s.k << endl;
    waitKey(0);
#endif

    gauss1 = gauss2.clone();
  }

  if ( s.doGNormalize )
    for ( int i = 0; i < dlevels; i++ )
    {
      doG[i] += minVal;
      doG[i] *= 1.0/(maxVal-minVal);
    }
}
  
void doGPoints( const vector<Mat>& doG, vector<Mat>& doGPoints, const Settings& s )
{
  // doG[i] has sigma = sigma0*k^(i-(doGRegionZ/2))
  
  // final level to examine
  int xOff = s.doGRegionX/2,
      yOff = s.doGRegionY/2,
      zOff = s.doGRegionZ/2;

  int minX = xOff,
      maxX = doG[0].cols - xOff,
      minY = yOff,
      maxY = doG[0].rows - yOff,
      minZ = zOff,
      maxZ = doG.size() - zOff;

  int x, y, z, x2, y2, z2;

  // find the largest and smallest in the region and compare to val
  float val, largest, smallest, current;
  bool maxima, minima;
    
  // holds -1 for non-extrema or the difference between largest/smallest for extrema
  Mat validPoints(doG[0].size(),CV_32FC1,Scalar(-1));
  
  for ( z = minZ; z < maxZ; z++ )
  {
    for ( x = minX; x < maxX; x++ )
      for ( y = minY; y < maxY; y++ )
      {
        // get center value
        val = doG[z].at<float>(y,x);

        // only check if val >= threshold
        maxima = minima = (fabs(val) >= s.t2);

        // start at first value to find min/max values
        smallest = largest = doG[z-zOff].at<float>(y-yOff,x-xOff);

        // check for maxima/minima
        for ( x2 = x-xOff; x2 <= x+xOff && (maxima || minima); x2++ )
          for ( y2 = y-yOff; y2 <= y+yOff && (maxima || minima); y2++ )
            for ( z2 = z-zOff; z2 <= z+zOff && (maxima || minima); z2++ )
            {
              current = doG[z2].at<float>(y2,x2);
              
              // smallest only valid if point is a maxima 
              if ( minima && ( smallest > current ) )
                smallest = current;

              // largest only valid if point is minima
              if ( maxima && ( largest < current ) )
                largest = current;

              // allow values equal to center count as extrema
              if ( s.doGAllowEqual )
              {
                if ( val < current )
                  maxima = false;
                if ( val > current )
                  minima = false;
              }
              else  // equal values means no extrema
              {
                if ( val <= current )
                  maxima = false;
                if ( val >= current )
                  minima = false;
              }
            }

        // save point if extrema
        if ( minima )
          validPoints.at<float>(y,x) = (s.doGUseDiff ? largest - val : 1.0);
        else if ( maxima )
          validPoints.at<float>(y,x) = (s.doGUseDiff ? val - smallest : 1.0);
        else  // -1 means no extrema
          validPoints.at<float>(y,x) = -1.0;
      }
    
    // push new points to back of vector
    doGPoints.push_back(validPoints.clone());

#ifdef __showdogmax
    Mat temp;
    normalize(validPoints,temp,0.0,255.0,CV_MINMAX,CV_8UC1);
    imshow("",temp);
    cout << "index: " << z << " sigma: " << s.sigma0*pow(s.k,z-zOff) << endl;
    waitKey(0);
#endif
  }
}

void buildHarris(const Mat& img, vector<Mat>& harris, const Settings& s)
{
  float sigmaD, sigmaI;
  int neighSize, maskSize;
  Mat corners;
  float sigma0 = s.sigma0;

  for ( int i = 0; i < s.levels; i++ )
  {
    sigmaI = sigma0;
    sigmaD = s.sigmaDRatio * sigmaI;
    
    harrisDetector(img,corners,sigmaD,sigmaI,s.alpha);
    
    harris.push_back(corners.clone());
    
    sigma0 *= s.k;
  }
}

void harrisPoints( const vector<Mat>& harris, vector<Mat>& harrisMaxima, const Settings& s )
{
  // assume all Mat in harris are same size
  int xOff = s.harRegionX/2,
      yOff = s.harRegionY/2;

  int minX = xOff;
  int maxX = harris[0].cols - xOff;
  int minY = yOff;
  int maxY = harris[0].rows - yOff;

  bool largest;
  bool maxima;
  
  float val, current;
  int i, x, y, x2, y2;

  Mat validPoints(harris[0].size(), CV_32FC1, Scalar(-1.0));

  for ( i = 0; i < s.levels; i++ )
  {
    for ( x = minX; x < maxX; x++ )
      for ( y = minY; y < maxY; y++ )
      {
        // get center value
        val = harris[i].at<float>(y,x);

        // only check if val >= threshold
        maxima = (fabs(val) >= s.t1);

        // start at first value to find min/max values
        largest = harris[i].at<float>(y-yOff,x-xOff);

        // check for maxima/minima
        for ( x2 = x-xOff; x2 <= x+xOff && maxima; x2++ )
          for ( y2 = y-yOff; y2 <= y+yOff && maxima; y2++ )
          {
            current = harris[i].at<float>(y2,x2);
            
            if ( largest < current )
              largest = current;

            // allow values equal to center count as maxima
            if ( s.harAllowEqual )
            {
              if ( val < current )
                maxima = false;
            }
            else  // equal values means no maxima
            {
              if ( val <= current )
                maxima = false;
            }
          }

        // save point if extrema
        if ( maxima )
          validPoints.at<float>(y,x) = (s.harUseDiff ? val - largest : 1.0);
        else  // -1 means no extrema
          validPoints.at<float>(y,x) = -1.0;
      }
    
    // push new points to back of vector
    harrisMaxima.push_back(validPoints.clone());
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
  vector<Mat> doG, harris, doGExtrema, harrisMaxima;

  // convert to floating point
  img8bit.convertTo(img,CV_32FC1);

  // build pyramids
  buildDoG(img, doG, s);

  // run Harris on gaussian
  buildHarris(img, harris, s);

  // find harris maxima
  harrisPoints(harris, harrisMaxima, s);

  // verify maxima using the DoG pyramid
  doGPoints(doG, doGExtrema, s);

#ifdef __showdogpoints
  Mat colorImg = imread(s.imagename,1);

  int featCount = 0;
  float sigma = s.sigma0;
  bool firstCheck, secondCheck;
  for ( int i = 0; i < s.levels; i++ )
  {
    int xOff = s.acceptX/2;
    int yOff = s.acceptY/2;
    bool leave;
    for ( int r = yOff; r < doGExtrema[i].rows-yOff; r++ )
      for ( int c = xOff; c < doGExtrema[i].cols-xOff; c++ )
      {
        leave = false;
        if ( s.useHarrisLocs )
          firstCheck = harrisMaxima[i].at<float>(r,c) > -0.00001;
        else
          firstCheck = doGExtrema[i].at<float>(r,c) > -0.00001;

        if ( firstCheck )
          for ( int r2 = r-yOff; r2 <= r+yOff && !leave; r2++ )
            for ( int c2 = c-xOff; c2 <= c+xOff && !leave; c2++ )
            {
              if ( s.useHarrisLocs )
                secondCheck = doGExtrema[i].at<float>(r2,c2) > -0.00001;
              else
                secondCheck = harrisMaxima[i].at<float>(r2,c2) > -0.00001;
              if ( secondCheck )
              {
                featCount++;
                circle(colorImg,Point(c,r),sqrt(2)*sigma+1,Scalar(0,0,255));
                leave = true;
              }
            }
      }
    sigma *= s.k;
  }
  cout << featCount << endl;

  imshow("",colorImg);
  waitKey(0);
#endif

  return 0;
}

