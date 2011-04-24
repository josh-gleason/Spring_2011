#ifndef JOSH_EIGFACE
#define JOSH_EIGFACE

#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

using namespace cv;
using namespace std;

void readImagesFile(string filename, vector<string>& images, Size& size)
{
  ifstream fin(filename.c_str());
  string a;

  if (!fin.good())
    return;

  fin >> size.width >> size.height;
  getline(fin,a);

  while ( fin.good() )
  {
    getline(fin,a);
    if ( fin.good() )
      images.push_back(a);
  }
  fin.close();
}

// returns column vector of image (output is 32FC1)
void linearizeImage(const Mat& input, Mat& output)
{
  int rows = input.rows, cols = input.cols;
  int size = rows*cols, r, c, x;
  output = Mat(size,1,CV_32FC1);
  Mat in;

  if ( input.type() != CV_32FC1 )
    input.convertTo(in, CV_32FC1);
  else
    in = input;

  for ( c = 0, x = 0; c < cols; c++ )
    for ( r = 0; r < rows; r++, x++) 
      output.at<float>(x,0) = in.at<float>(r,c);
}

// turns a column vector back to an image
void delinearizeImage(const Mat&,Mat&,const Size&, int outputType=CV_32FC1);
void delinearizeImage(const Mat& input, Mat& output, const Size& imgSize, int outputType)
{
  int rows = imgSize.height, cols = imgSize.width, r, c, x;
  Mat out, in;

  if ( outputType != CV_32FC1 )
    out = Mat(imgSize, CV_32FC1);
  else // out alias output now
  {
    output = Mat(imgSize, CV_32FC1);
    out = output;
  }
  
  if ( input.type() != CV_32FC1 )
    input.convertTo(in, CV_32FC1);
  else
    in = input;

  for ( c = 0, x = 0; c < cols; c++ )
    for ( r = 0; r < rows; r++, x++ )
      out.at<float>(r,c) = in.at<float>(x,0);

  if ( outputType != CV_32FC1 )
    out.convertTo(output, outputType);
}

void buildA(const Mat& mean, const vector<string>& img, Mat& A)
{
  int imgCount = img.size();
  int rows, cols;
  A = Mat(mean.rows, imgCount, CV_32FC1);

  int r, c, aR, aC;
  Mat face;

  for ( aC = 0; aC < imgCount; aC++ )
  {
    face = imread(img[aC],0);
    for ( c = 0, aR = 0; c < face.cols; c++ )
      for ( r = 0; r < face.rows; r++, aR++ )
        A.at<float>(aR,aC) = static_cast<float>(face.at<uchar>(r,c)) - mean.at<float>(aR,0);
  }
}

void projectFace(const Mat& image, const vector<Mat>& eFaces, const Mat& mean,
  Mat& coeff)
{
  int dim = eFaces.size(), i;
  coeff = Mat(dim,1,CV_32FC1);

  Mat imgVector;

  linearizeImage(image, imgVector);

  imgVector -= mean;

  for ( i = 0; i < dim; i++ )
  {
    Mat t1, t2;
    transpose(eFaces[i],t1);

    t2=t1*imgVector;

    coeff.at<float>(i,0) = t2.at<float>(0,0);
  }
}

void backprojectFace(const Mat&, const vector<Mat>&, const Mat&, Size&, Mat&,
  int outputType=CV_32FC1);
void backprojectFace(const Mat& coeff, const vector<Mat>& eFaces,
  const Mat& meanFace, Size& imgSize, Mat& image, int outputType)
{
  int dim = eFaces.size(), i;

  Mat linImg = meanFace.clone();

  for ( i = 0; i < dim; i++ )
    linImg += eFaces[i]*coeff.at<float>(i,0);
  
  delinearizeImage(linImg, image, imgSize, outputType);
}

bool meanFace(const Size& imgSize, const vector<string>& img, Mat& mean)
{
  int imgCount = img.size();
  Mat face;

  // initialize mean face to all zeros
  mean = Mat(imgSize.height*imgSize.width,1,CV_32FC1,Scalar(0));

  int i, r, c, x;

  for ( i = 0; i < imgCount; i++ )
  {
    face = imread(img[i],0);
    for ( c = 0, x = 0; c < imgSize.width; c++ )
      for ( r = 0; r < imgSize.height; r++, x++ )
        mean.at<float>(x,0) += static_cast<float>(face.at<uchar>(r,c));
  }

  mean *= (1.0/imgCount);
}

#endif // JOSH_EIGFACE

