#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

using namespace std;
using namespace cv;

int roundOdd(float a)
{
  int val = int(a+0.5);
  if (val % 2 == 0)
    val++;
  return val;
}

void kerns(int size, float sigma, Mat &x, Mat &y)
{
  x.create(1,size,CV_32FC1);
  y.create(size,1,CV_32FC1);

  // 0.39894228 = 1/sqrt(2*pi)
  for ( int i = 0; i < size; i++ )
    x.at<float>(0,i) = y.at<float>(i,0) =
      0.39894228/sigma*(exp(-(pow(i-(size-1)/2.,2)/(2*pow(sigma,2)))));
}

// returns subpixel values using linear interpolation
float getVal(const Mat& img, float row, float col)
{
  // zero because outside of bounds
  if ( row <= -1.0 || row >= img.rows || col <= -1.0 || col >= img.cols )
    return 0.;
  else if ( (int)row == row && (int)col == col )  // using whole numbers
    return img.at<float>((int)row,(int)col);
  else
  {
    // rather ugly linear interpolation
    static int lRow, hRow, lCol, hCol;
    static float q12, q22, q11, q21, rOff, cOff, r1, r2;
  
    if ( (int)row == row )
      lRow = hRow = (int)row;
    else
    {
      if ( row < 0 )
        lRow = (int)row - 1;
      else
        lRow = (int)row;
      hRow = lRow + 1;
    }

    if ( (int)col == col )
      lCol = hCol = (int)col;
    else
    {
      if ( col < 0 )
        lCol = (int)col-1;
      else
        lCol = (int)col;
      hCol = lCol + 1;
    }
    
    if ( lRow < 0 || lCol < 0 || lRow >= img.rows || lCol >= img.cols )
      q11 = 0;
    else
      q11 = img.at<float>(lRow,lCol);

    if ( hRow < 0 || lCol < 0 || hRow >= img.rows || lCol >= img.cols )
      q12 = 0;
    else
      q12 = img.at<float>(hRow,lCol);

    if ( lRow < 0 || hCol < 0 || lRow >= img.rows || hCol >= img.cols )
      q21 = 0;
    else
      q21 = img.at<float>(lRow,hCol);

    if ( hRow < 0 || hCol < 0 || hRow >= img.rows || hCol >= img.cols )
      q22 = 0;
    else
      q22 = img.at<float>(hRow,hCol);
 
    if ( hCol == lCol )
    {
      r1 = q11;
      r2 = q12;
    }
    else
    {
      r1 = (hCol - col)*q11 + (col - lCol)*q21;
      r2 = (hCol - col)*q12 + (col - lCol)*q22;
    }
    
    if ( hRow == lRow )
      return r1;
    else
      return (hRow - row)*r1 + (row - lRow)*r2;
  }

}

// assumes CV_32FC1 type Matricies
void filter(const Mat &input, Mat &output, const Mat &kern)
{
  int rows = input.rows,
      cols = input.cols,
      kernRows = kern.rows,
      kernCols = kern.cols;

  output.create(rows,cols,CV_32FC1);
  output = cvRealScalar(0.);

  int i, j, k, l;

  for ( i = 0; i < rows; i++ )
    for ( j = 0; j < cols; j++ )
      for ( k = 0; k < kernRows; k++ )
        for ( l = 0; l < kernCols; l++ )
          output.at<float>(i,j) += kern.at<float>(k,l)*
            getVal(input,i+k-(kernRows-1)/2.,j+l-(kernCols-1)/2.);
}

void Blur(const Mat& input, Mat& output, int window, float sigma)
{
  output = Mat(input.size(),CV_32FC1);
  Mat xKern, yKern,temp;
  kerns(window,sigma,xKern,yKern);
  filter(input,temp,xKern);
  filter(temp,output,yKern);
}

int main(int argc, char *argv[])
{

  if ( argc < 2 )
    cout << "Please enter an image" << endl;
  
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
        {
          for ( int r = row-1; r < row+1 && !found; r++ )
            for ( int c = col-1; c < col+1 && !found; c++) 
              if ( r >= 0 && r < totRows && c >= 0 && c < totCols )
                temp.at<float>(r,c) = 1;
        }
      }
    zeroCrossings[i] = temp.clone();    // so that a MatExpr is returned an data is cloned

    //normalize(zeroCrossings[i],temp2,0.0,255.0,CV_MINMAX,CV_8UC1);
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
          if ( var >= atof(argv[2])/(i+1) )
            edges[i].at<float>(row,col) = 255;
//          edges[i].at<float>(row,col) = var;
        }
        else
          edges[i].at<float>(row,col) = 0.0;
      }
//    edges[i] = lapPyr[i]-temp;
//    temp2 = edges[i].clone();
//    edges[i] = edges[i].mul(temp2);// = edges[i]*edges[i];
//    edges[i] = edges[i].mul(zeroCrossings[i]);

    for ( int row = 0; row < totRows; row++ )
    {
      edges[i].at<float>(row,totCols-1) = 0;
      edges[i].at<float>(row,totCols-2) = 0;
      edges[i].at<float>(row,totCols-3) = 0;
      edges[i].at<float>(row,totCols-4) = 0;
      edges[i].at<float>(row,0) = 0;
      edges[i].at<float>(row,1) = 0;
      edges[i].at<float>(row,2) = 0;
      edges[i].at<float>(row,3) = 0;
    }
    for ( int col = 0; col < totCols; col++ )
    {
      edges[i].at<float>(totCols-1,col) = 0;
      edges[i].at<float>(totCols-2,col) = 0;
      edges[i].at<float>(totCols-3,col) = 0;
      edges[i].at<float>(totCols-4,col) = 0;
      edges[i].at<float>(0,col) = 0;
      edges[i].at<float>(1,col) = 0;
      edges[i].at<float>(2,col) = 0;
      edges[i].at<float>(3,col) = 0;
    }
    float min,max,val;
    min = max = edges[i].at<float>(0,0);
    for ( int row = 0; row < totRows; row++ )
      for ( int col = 0; col < totCols; col++ )
      {
        val = edges[i].at<float>(row,col);
        if ( val < min )
          min = val;
        if ( val > max )
          max = val;
      }
    cout << min << ' ' << max << endl;
    
    normalize(edges[i],temp,0.0,255.0,CV_MINMAX,CV_8UC1);
//    threshold(temp,temp2,0,255,THRESH_BINARY);
    char imName[20];
    sprintf(imName,"Level%i",i+1);
    imshow(imName,temp);//edges[i]);
    waitKey(0);
  }
  /*
  for ( int i = 0; i < 5; i++ )
  {
    normalize(lapPyr1[i],temp,0.0,255,CV_MINMAX,CV_8UC1);
    imshow("Image1",temp);
    normalize(lapPyr2[i],temp,0.0,255,CV_MINMAX,CV_8UC1);
    imshow("Image2",temp);
    waitKey(0);
  }*/

  return 0;
}
