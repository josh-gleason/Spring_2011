// keeps all eigenfaces and reconstructs each image them returns average MSE

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

double getMSE(const Mat& origImg, const Mat& newImg)
{
  // should both be 8UC1 types and same dimensions
  // skipping check here

  double val = 0.0;

  int rows = origImg.rows,
      cols = origImg.cols;

  for ( int y = 0; y < rows; y++ )
    for ( int x = 0; x < cols; x++ )
      val +=
        pow(static_cast<double>(origImg.at<uchar>(x,y)) -
            static_cast<double>(newImg.at<uchar>(x,y)), 2);

  return val / (rows*cols);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return -1;

  Size imgSize;
  vector<string> imgList;
  
  Mat mean, A, Atrans, lambda, eigVectors;

  vector<Mat> eigFaces;   // used the name "u" in train.cc
  vector<double> w;

  readImagesFile(argv[1],imgList,imgSize);

  // build eigenface (This should probably be it's own function)
  meanFace(imgSize,imgList,mean);
  buildA(mean,imgList,A);
  transpose(A,Atrans);
  eigen(Atrans*A, lambda, eigVectors);    // this is the function that takes forever
  
  cout << "Number of Eigenvectors : " << eigVectors.rows << endl;
  for ( int i = 0; i < eigVectors.rows; i++ )
  {
    static Mat t1, t2, t3, v;
    t1 = eigVectors.row(i).clone();
    transpose(t1,v);
    t2 = A*v;
    normalize(t2, t3);

    eigFaces.push_back(t3.clone());
  }
  
  // calculate coefficients for all faces
  vector<Mat> coeffs;
  
  for ( int i = 0; i < imgList.size(); i++ )  // probably could use iterator here
  {
    Mat image = imread(imgList[i],0);
    Mat m;
    projectFace(image,eigFaces,mean,m);
    coeffs.push_back(m.clone());
  }

  // reconstruct faces
  vector<double> MSE;

  for ( int i = 0; i < imgList.size(); i++ )
  {
    Mat orig = imread(imgList[i],0);
    Mat newImg;
    backprojectFace(coeffs[i],eigFaces,mean,imgSize,newImg,CV_8UC1);
    MSE.push_back(getMSE(orig,newImg));
    cout << MSE[i] << endl;
  }
  
  // get average MSE for all images
  double avg = 0.0;
  for ( int i = 0; i < MSE.size(); i++) 
    avg += MSE[i];
  avg /= (imgSize.height * imgSize.width);

  cout << "Average MSE : " << avg << endl;

  return 0;
}
