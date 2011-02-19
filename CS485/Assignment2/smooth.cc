#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <cmath>

using namespace std;
using namespace cv;

void kerns(int sigma, Mat &x, Mat &y)
{
  // |x|=|y|=5*sigma
  x.create(1,sigma*5,CV_32FC1);
  y.create(sigma*5,1,CV_32FC1);

  // k = i-(5*sigma-1)/2
  for ( int i = 0; i <= 5*sigma; i++ )
    x.at<float>(0,i) = y.at<float>(i,0) =
      (exp(-(pow(i-(5.*sigma-1)/2,2)/(2*pow(sigma,2)))));

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
          if ( i+k-(kernRows-1)/2 < rows && j+l-(kernCols-1)/2 < cols
            && i+k-(kernRows-1)/2 >= 0   && j+l-(kernCols-1)/2 >= 0)
            output.at<float>(i,j) += kern.at<float>(k,l)*input.at<float>(i+k-(kernRows-1)/2,j+l-(kernCols-1)/2);
}

int main(int argc, char *argv[])
{
  if ( argc < 5 )
  {
    cout << "Command line arguments are as follows..." << endl
         << "<sigma> <input> <output1> <output2>" << endl
         << "sigma must be an interger value" << endl
         << "output1 is the input blurred using seperable kernels" << endl
         << "output2 is the input blurred using OpenCV's GaussianBlur function" << endl;
    return -1;
  }

  int sigma = atoi(argv[1]);

  Mat xKern,yKern,xGauss,smooth,show1,show2,cmp,
    img = imread(argv[2],0), fImg;

  // convert to floating point
  img.convertTo(fImg,CV_32FC1);

  // build kernels
  kerns(sigma,xKern,yKern);

  // filter image
  filter(fImg,xGauss,xKern);
  filter(xGauss,smooth,yKern);
  
  // create comparison image
  GaussianBlur(img,cmp,cvSize(5*sigma,5*sigma),sigma,sigma,BORDER_CONSTANT);

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
