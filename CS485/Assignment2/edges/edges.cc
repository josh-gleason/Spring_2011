#include <sstream>
#include <iostream>
#include "funcs.h"

// dont show images
#define imshow(a,b); ;
#define waitKey(a); ;

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{

  if ( argc < 3 )
  {
    cout << "./edges <input> <threshold> <output>" << endl;
    return -1;
  }
  
  Mat img = imread(argv[1],0);
  Mat gaussPyr[6], lapPyr[5], threshImg[5], zeroCrossings[5], edges[5];
  Mat temp,temp2;
  
  // for finding mean of 5x5 neighborhood
  float avgVals[25] = {1./25,1./25,1./25,1./25,1./25,
                       1./25,1./25,1./25,1./25,1./25,
                       1./25,1./25,1./25,1./25,1./25,
                       1./25,1./25,1./25,1./25,1./25,
                       1./25,1./25,1./25,1./25,1./25};
  Mat avgKern(5,5,CV_32FC1,(void*)avgVals);
  img.convertTo(gaussPyr[0],CV_32FC1);

  float sigma = 1;
  int window = roundOdd(5*sigma);
  
  // build Gaussian pyramid
  for ( int i = 0; i < 6; i++ )
    Blur(gaussPyr[i],gaussPyr[i+1],window,sigma);

  // build Laplacian pyramid
  for ( int i = 0; i < 5; i++ )
    lapPyr[i] = gaussPyr[i]-gaussPyr[i+1];

  float pixVal;
  int totRows = lapPyr[0].rows, totCols = lapPyr[0].cols;
  bool found = false;
  ostringstream sout;
  for ( int i = 0; i < 5; i++ )
  {
    threshold(lapPyr[i],threshImg[i],0,1.0,THRESH_BINARY);
    zeroCrossings[i] = Mat(cvSize(totRows, totCols),CV_32FC1);
    // build zero crossings image
    for ( int row = 0; row < totRows; row++ )
      for ( int col = 0; col < totCols; col++ )
      {
        pixVal = threshImg[i].at<float>(row,col);
        zeroCrossings[i].at<float>(row,col) = 0;
        found = false;
        for ( int r = row-1; r < row+1 && !found; r++ )
          for ( int c = col-1; c < col+1 && !found; c++ )
            if ( r != row && c != col && r >= 0 && r < totRows && c >= 0 && c < totCols )
              if ( pixVal != threshImg[i].at<float>(r,c) )
              {
                zeroCrossings[i].at<float>(row,col) = 1;
                found = true;
              }
      }
    // simple dialte on zeroCrossings to get neighbors
    temp = Mat(zeroCrossings[i].size(),CV_32FC1, Scalar(0));
    for ( int row = 0; row < totRows; row++ )
      for ( int col = 0; col < totCols; col++ )
      {
        pixVal = zeroCrossings[i].at<float>(row,col);
        if ( pixVal > 0 )
          for ( int r = row-1; r < row+1 && !found; r++ )
            for ( int c = col-1; c < col+1 && !found; c++) 
              if ( r >= 0 && r < totRows && c >= 0 && c < totCols )
                temp.at<float>(r,c) = 1;
      }
    zeroCrossings[i] = temp.clone();    // so that a MatExpr is returned an data is cloned

    filter(lapPyr[i],temp,avgKern);  // find 5x5 mean 
    edges[i] = Mat(temp.size(), CV_32FC1, Scalar(0));
    for ( int row = 0; row < totRows; row++ )
      for ( int col = 0; col < totCols; col++ )
      {
        float count = 0;
        float var = 0, val;
        if ( zeroCrossings[i].at<float>(row,col) > 0 )
        {
          float mean = temp.at<float>(row,col);
          for ( int r = row - 2; r <= row + 2; r++ )
            for ( int c = col - 2; c <= col + 2; c++ )
              if ( r >= 0 && r < totRows && c >= 0 && c < totCols )
              {
                val = lapPyr[i].at<float>(r,c);
                count++;
                var += ((mean - val)*(mean-val));
              }
          var /= count;
          var = sqrt(var);

          // threshold image
          if ( var >= atof(argv[2])/(i+1) )
            edges[i].at<float>(row,col) = 255;
        }
        else
          edges[i].at<float>(row,col) = 0.0;
      }
    
    normalize(edges[i],temp,0.0,255.0,CV_MINMAX,CV_8UC1);
    sout.str("");
    sout << argv[3] << i << ".jpg";
    char imName[20];
    sprintf(imName,"Level%i",i+1);
    imwrite(sout.str().c_str(),temp);
    imshow(imName,temp);
    waitKey(0);
  }
  
  return 0;
}
