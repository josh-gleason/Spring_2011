#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include "eigface.h"

using namespace std;
using namespace cv;

void loadEigenfaces( string filename, vector<Mat>& eFaces, vector<vector<double> >& coeffs,
  vector<string>& images, Mat& mean, Size& imgSize )
{
  ifstream fin(filename.c_str());
  int dim, imgCount, pixels, i, j;

  // read useful information
  fin >> imgSize.height >> imgSize.width >> dim >> imgCount;


  pixels = imgSize.height*imgSize.width;
  
  // resize vectors for speed
  mean = Mat(pixels,1,CV_32FC1);
  
  eFaces.resize(dim);
  for ( i = 0; i < dim; i++ )
    eFaces[i] = Mat(pixels,1,CV_32FC1);

  images.resize(imgCount);
  coeffs.resize(imgCount);
  for ( i = 0; i < imgCount; i++ )
    coeffs[i].resize(dim);

  // read mean face
  for ( i = 0; i < pixels; i++ )
    fin >> mean.at<float>(i,0);

  // read eigenfaces
  for ( i = 0; i < dim; i++ )
    for ( j = 0; j < pixels; j++ )
      fin >> eFaces[i].at<float>(j,0);

  // read coefficients
  for ( i = 0; i < imgCount; i++ )
  {
    fin >> images[i];
    for ( j = 0; j < dim; j++ )
      fin >> coeffs[i][j];
  }

  fin.close();
}

int main(int argc, char *argv[])
{
  if ( argc < 2 )
    return -1;

  vector<Mat> eFaces;
  vector<vector<double> > coeffs;
  vector<string> imgList;
  Mat mean;
  Size imgSize;

  loadEigenfaces(argv[1],eFaces,coeffs,imgList,mean,imgSize);

  Mat back;

  backprojectFace(coeffs[0],eFaces,mean,imgSize,back,CV_8UC1);
  imshow("img",back);
  waitKey(0);

  return 0;
}

