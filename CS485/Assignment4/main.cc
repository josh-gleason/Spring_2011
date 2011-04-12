#include <iostream>
#include <fstream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include "readsettings.h"

using namespace std;
using namespace cv;

//#define __showdog
//#define__showdogmax
//#define __showdogpoints

struct Pointf
{
  Pointf(Point _a) : x(_a.x), y(_a.y) {}
  Pointf(float _x, float _y) : x(_x), y(_y) {}
  Pointf(const Pointf& _a) : x(_a.x), y(_a.y) {}
  Pointf() {}

  operator Point() const { return Point(round(x),round(y)); }

  float x, y;
};

struct Pixel
{
  Pixel() {}
  Pixel(Point _p, float _val) : p(_p), val(_val) {}
  Pointf p;
  float val;
};

struct Pair
{
  Pair() {}
  Pair(int _a, int _b) : a(_a), b(_b) {}
  int a, b;
};

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
void harrisDetector(const Mat& img,Mat& corners,float sigmaD,float sigmaI,float alpha,
  const Settings& s)
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

  int win = sigmaD*s.harSigmaDWin;
  if ( win % 2 == 0 )
    win++;
  
  // blur image before differentiation (sigmaD)
  GaussianBlur(img,blurred,Size(win,win),sigmaD);

  // apply Sobel (differentiation)
  filter2D(blurred,imgX,CV_32FC1,sobelX);
  filter2D(blurred,imgY,CV_32FC1,sobelY);

  // multiply then blur (sigmaI)
  win = sigmaI*s.harSigmaIWin;
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
    
  float minVal, maxVal, minValt, maxValt;

  // round to nearest odd number
  int win = round(s.winMult * sigma);
  if ( win % 2 == 0 )
    win++;

  // gauss1 has sigma less than gauss2
  Mat gauss1, gauss2, doGImg;

  // build first layer
  GaussianBlur(img,gauss1,Size(win,win),sigma);

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
    if ( s.showDoG )
    {
      Mat temp;
      normalize(doGImg,temp,0.0,255.0,CV_MINMAX,CV_8UC1);
      imshow("",temp);
      cout << sigma/s.k << endl;
      waitKey(0);
    }

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
              if ( x2 != x || y2 != y || z2 != z )
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
        if ( minima && (smallest - val >= s.doGMaxThresh) )
          validPoints.at<float>(y,x) = (s.doGUseDiff ? largest - val : 1.0);
        else if ( maxima && (val - largest >= s.doGMaxThresh) )
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
  float sigma = s.sigma0;

  for ( int i = 0; i < s.levels; i++ )
  {
    sigmaI = s.harScaleSigma*sigma+s.harAddSigma;
    sigmaD = s.sigmaDRatio * sigmaI;
   
    if ( s.useOpenCVHarris == 1 )
    {
      Mat blurred;
      int win = sigmaD*s.openCVHarD;
      if ( win % 2 == 0 )
        win++;
      GaussianBlur(img,blurred,Size(win,win),sigmaD);
      win = sigmaI*s.openCVHarI;
      if ( win % 2 == 0 )
        win++;
      cornerHarris(blurred,corners,win,0,s.alpha);
    }
    else if ( s.useOpenCVHarris == 2 )
    {
      int winD = round(sigmaD*s.openCVHarD);
      if ( winD > 31 )
        winD = 31;
      if ( winD % 2 == 0 )
        winD++;

      int winI = round(sigmaI*s.openCVHarI);
      if ( winI % 2 == 0 )
        winI++;

      cornerHarris(img,corners,winI,winD,s.alpha);
    }
    else // use mine
      harrisDetector(img,corners,sigmaD,sigmaI,s.alpha,s);  

    if ( s.harNormalize )
    {
      Mat temp;
      normalize(corners, temp, 0.0, 1000000.0, CV_MINMAX, CV_32FC1);
      harris.push_back(temp.clone());
    }
    else
    {
      harris.push_back(corners.clone());
    }

    sigma *= s.k;
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
  
  float val, current, 
        thresh = s.t1,
        sigma = s.harScaleSigma*s.sigma0+s.harAddSigma;

  int i, x, y, x2, y2;

  Mat validPoints(harris[0].size(), CV_32FC1, Scalar(-1.0));
  
  for ( i = 0; i < s.levels; i++ )
  {
    // set addaptive threshold
    if ( s.harAdaptiveScale > 0 )
      thresh = s.t1 * s.harAdaptiveScale/sigma;

    // set addaptive window size
    if ( s.harRegionScale > 0 )
    {
      xOff = (s.harRegionScale*sigma*s.harRegionX)/2,
      yOff = (s.harRegionScale*sigma*s.harRegionY)/2;
    
      minX = xOff;
      maxX = harris[0].cols - xOff;
      minY = yOff;
      maxY = harris[0].rows - yOff;
    }

    for ( x = minX; x < maxX; x++ )
      for ( y = minY; y < maxY; y++ )
      {
        // get center value
        val = harris[i].at<float>(y,x);

        // only check if val >= threshold
        maxima = (fabs(val) >= thresh);

        // start at first value to find min/max values
        largest = harris[i].at<float>(y-yOff,x-xOff);

        // check for maxima/minima
        for ( x2 = x-xOff; x2 <= x+xOff && maxima; x2++ )
          for ( y2 = y-yOff; y2 <= y+yOff && maxima; y2++ )
            if ( x2 != x || y2 != y )
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
        if ( maxima && (val - largest >= s.harrisMaxThresh ) )
          validPoints.at<float>(y,x) = (s.harUseDiff ? val - largest : 1.0);
        else  // -1 means no extrema
          validPoints.at<float>(y,x) = -1.0;
      }
    
    // push new points to back of vector
    harrisMaxima.push_back(validPoints.clone());
    
    sigma *= s.k;
  }
}

void getValidPoints(const vector<Mat>& keepPoints, const vector<Mat>& matchPoints, vector<Pixel>& points, const Settings& s )
{
  float sigma = s.sigma0;
  float val;
  int xOff = s.acceptX/2;
  int yOff = s.acceptY/2;
  int pointCount = 0;
  bool leave;
  int rows = keepPoints[0].rows,
      cols = keepPoints[0].cols;

  s.scaleMultiplier;
  for ( int i = 0; i < s.levels; i++ )
  {
    if ( s.scaleMultiplier > 0.0 )
    {
      xOff = round(s.acceptX/2 * s.scaleMultiplier*sigma);
      yOff = round(s.acceptY/2 * s.scaleMultiplier*sigma);
    }

    for ( int r = yOff; r < rows-yOff; r++ )
      for ( int c = xOff; c < cols-xOff; c++ )
      {
        leave = false;

        if ( keepPoints[i].at<float>(r,c) > -0.00001 )
        {
          if ( s.enableHarris && s.enableDoG )
            for ( int r2 = r-yOff; r2 <= r+yOff && !leave; r2++ )
              for ( int c2 = c-xOff; c2 <= c+xOff && !leave; c2++ )
              {
                val = matchPoints[i].at<float>(r2,c2);
                if ( val > -0.00001 )
                {
                  points.push_back(Pixel(Point(c,r),sigma*sqrt(2)));
                  leave = true;
                }
              }
          else
            points.push_back(Pixel(Point(c,r),sigma*sqrt(2)));
        }
      }
    sigma *= s.k;
  }
}

void drawPoints( const vector<Pixel>& points, Mat& img, const Settings &s )
{
  int size = points.size(), i;
  for ( i = 0; i < points.size(); i++ )
  {
    line(img,Point(points[i].p.x-s.crossSize,points[i].p.y-s.crossSize),
      Point(points[i].p.x+s.crossSize,points[i].p.y+s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    line(img,Point(points[i].p.x+s.crossSize,points[i].p.y-s.crossSize),
      Point(points[i].p.x-s.crossSize,points[i].p.y+s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    circle(img,(Point)points[i].p,points[i].val*s.circleMult,
      Scalar(s.circleB,s.circleG,s.circleR),s.circleThickness);
  }
}

void findInterestPoints(const Mat& img, vector<Pixel>& points, const Settings& s)
{
  vector<Mat> doG, harris, doGExtrema, harrisMaxima;
  
  if ( s.enableDoG )
  {
    // build pyramids
    buildDoG(img, doG, s);

    // verify maxima using the DoG pyramid
    doGPoints(doG, doGExtrema, s);
  }

  if ( s.enableHarris )
  {
    // build Harris pyramid
    buildHarris(img, harris, s);

    // find harris maxima
    harrisPoints(harris, harrisMaxima, s);
  }

  // matching points
  if ( (s.useHarrisLocs && s.enableHarris) || !s.enableDoG )
    getValidPoints( harrisMaxima, doGExtrema, points, s );
  else
    getValidPoints( doGExtrema, harrisMaxima, points, s );
}

void readHMat( const string& filename, Mat& H )
{
  ifstream fin(filename.c_str());

  H = Mat(3,3,CV_32FC1);
  for ( int r = 0; r < 3; r++ )
    for ( int c = 0; c < 3; c++ )
      fin >> H.at<float>(r,c);

  fin.close();
}

void transform( const vector<Pixel>& points, vector<Pixel>& output,
  const Mat& H )
{
  Mat xa(3,1,CV_32FC1), xt(3,1,CV_32FC1);
  int size = points.size();
  Pixel temp;

  xa.at<float>(2,0) = 1;

  for ( int i = 0; i < size; i++ )
  {
    xa.at<float>(0,0) = points[i].p.x;
    xa.at<float>(1,0) = points[i].p.y;
    xt = H*xa;

    temp.p.x = xt.at<float>(0,0) / xt.at<float>(2,0);
    temp.p.y = xt.at<float>(1,0) / xt.at<float>(2,0);
    temp.val = points[i].val;

    output.push_back(temp);
  }
}

// does the transform to check bounds only, does not save transformed point
void removeOB( const Size& bounds, const vector<Pixel>& points, vector<Pixel>& output,
  const Mat& H )
{
  Mat xa(3,1,CV_32FC1), xt(3,1,CV_32FC1);
  int size = points.size();
  Pixel temp;

  int maxX = bounds.width,
      maxY = bounds.height;

  xa.at<float>(2,0) = 1;

  for ( int i = 0; i < size; i++ )
  {
    xa.at<float>(0,0) = points[i].p.x;
    xa.at<float>(1,0) = points[i].p.y;
    xt = H*xa;

    temp.p.x = xt.at<float>(0,0) / xt.at<float>(2,0);
    temp.p.y = xt.at<float>(1,0) / xt.at<float>(2,0);
    temp.val = points[i].val;

    // bounds check then push ORIGINAL POINT
    if ( temp.p.x < maxX && temp.p.x >= 0 && temp.p.y < maxY && temp.p.y >= 0 )
      output.push_back(points[i]);
  }
}

void getMatches( const vector<Pixel>& p1, const vector<Pixel>& p2, vector<Pair>& matchLocs,
  const Settings& s )
{
  int size1 = p1.size(), size2 = p2.size();
  float dist, bestDist;
  for ( int i = 0; i < size1; i++ )
  {
    bestDist = s.matchThresh;
    for ( int j = 0; j < size2; j++ )
    {
      dist = sqrt((p1[i].p.x-p2[j].p.x)*(p1[i].p.x-p2[j].p.x)+(p1[i].p.y-p2[j].p.y)*(p1[i].p.y-p2[j].p.y));

      if ( dist <= bestDist )
      {
        bestDist = dist;
        bool dontCount = false;

        if ( s.removeRepeats )  // don't count if j has already been matched to
          for ( int k = 0; k < matchLocs.size() && !dontCount; k++ )
            if ( matchLocs[k].b == j )
              dontCount = true;

        if ( !dontCount )
        {
          matchLocs.push_back(Pair(i,j));

          // dont look for shortest distance just be happy with this and go to next i
          if ( !s.shortestDist )
            j = size2+1;
        }
      }
    }
  }
}

void drawMatches(Mat& display1, Mat& display2, const vector<Pixel>& points,
  const vector<Pixel>& points2, const vector<Pair>& matches, const Settings& s)
{
  int size = matches.size();
  int p1, p2;
  for ( int i = 0; i < size; i++ )
  {
    p1 = matches[i].a;
    p2 = matches[i].b;

    line(display1,Point(points[p1].p.x-s.crossSize,points[p1].p.y-s.crossSize),
      Point(points[p1].p.x+s.crossSize,points[p1].p.y+s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    line(display1,Point(points[p1].p.x-s.crossSize,points[p1].p.y+s.crossSize),
      Point(points[p1].p.x+s.crossSize,points[p1].p.y-s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    circle(display1, points[p1].p, points[p1].val*s.circleMult,
      Scalar(s.circleB,s.circleG,s.circleR), s.circleThickness);
    
    line(display2,Point(points2[p2].p.x-s.crossSize,points2[p2].p.y-s.crossSize),
      Point(points2[p2].p.x+s.crossSize,points2[p2].p.y+s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    line(display2,Point(points2[p2].p.x-s.crossSize,points2[p2].p.y+s.crossSize),
      Point(points2[p2].p.x+s.crossSize,points2[p2].p.y-s.crossSize),
      Scalar(s.crossB,s.crossG,s.crossR),s.crossThickness);

    circle(display2, points2[p2].p, points2[p2].val*s.circleMult,
      Scalar(s.circleB,s.circleG,s.circleR), s.circleThickness);
  }
}

int main(int argc, char *argv[])
{
  if ( argc < 3 )
  {
    cout << "Please input the settings file" << endl;
    return -1;
  }

  cout.precision(10);

  Settings s(argv[1]);

  ofstream fout(argv[2],ios::app);

  // print out k
  fout << s.t1 << ' ' << s.t2 << ' ';

  Mat img8bit = imread(s.imagename,0);
  Mat img, img2, H;
  vector<Pixel> points1, points2, points1to2, points2inBounds, points1inBounds;
  vector<Pair> matches;

  // convert to floating point
  img8bit.convertTo(img,CV_32FC1);
  img8bit = imread(s.image2name,0);
  img8bit.convertTo(img2,CV_32FC1);

  // detect interest points in both images
  findInterestPoints(img, points1, s);
  findInterestPoints(img2, points2, s);
  
  // mark points
  if ( s.showOrigPoints )
  {
    Mat display1 = imread( s.imagename,1 ),
        display2 = imread( s.image2name,1 );
    drawPoints( points1, display1, s );
    imshow( "Interest Points (Img 1)",display1 );
    drawPoints( points2, display2, s );
    imshow( "Interest Points (Img 2)",display2 );
    waitKey(0);
    cvDestroyWindow("Interest Points (Img 1)");
    cvDestroyWindow("Interest Points (Img 2)");
  }
  
  // get transformation matrix
  readHMat(s.hfilename, H);

  // transform points and remove points that are out of bounds
  if ( s.boundsCheck )
  {
    removeOB(img.size(),points1,points1inBounds,H);
    removeOB(img.size(),points2,points2inBounds,H.inv());
    
    // mark points
    if ( s.showInBoundsPoints )
    {
      Mat display1 = imread( s.imagename,1 ),
          display2 = imread( s.image2name,1 );
      drawPoints( points1inBounds, display1, s );
      imshow( "In Bounds Interest Points (Img 1)",display1 );
      drawPoints( points2inBounds, display2, s );
      imshow( "In Bounds Interest Points (Img 2)",display2 );
      waitKey(0);
      cvDestroyWindow("In Bounds Interest Points (Img 1)");
      cvDestroyWindow("In Bounds Interest Points (Img 2)");
      
    }
      
    fout << points1inBounds.size() << ' ';
    fout << points2inBounds.size() << ' ';
      
    // transform points from Image 1 to Image 2 for comparison
    transform(points1inBounds,points1to2,H);

    // count matches (matches: ordered pairs of matching indecies) 
    getMatches(points1to2,points2inBounds,matches,s);
  
    fout << matches.size() << ' ';
    fout << matches.size()*100.0 / min(points1inBounds.size(),points2inBounds.size()) << ' ';
  }
  else    // don't check bounds (not recomended)
  {
    // transform points from Image 1 to Image 2 for comparison
    transform(points1,points1to2,H);

    // count matches (matches: ordered pairs of matching indecies)
    getMatches(points1to2, points2, matches, s);

    fout << matches.size() << ' ';
    fout << matches.size()*100.0 / min(points1.size(),points2.size()) << ' ';
  }

  if ( s.showMatching )
  {
    Mat display1, display2;
    display1 = imread( s.imagename,1 );
    display2 = imread( s.image2name,1 );
    if ( s.boundsCheck )
      drawMatches(display1, display2, points1inBounds, points2inBounds, matches, s);
    else
      drawMatches(display1, display2, points1, points2, matches, s);
    imshow("Image 1 Matches",display1);
    imshow("Image 2 Matches",display2);
    waitKey(0);
    cvDestroyWindow("Image 1 Matches");
    cvDestroyWindow("Image 2 Matches");
  }

  fout << endl;
  fout.close();

  return 0;
}

