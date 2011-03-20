#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include "face.h"

using namespace std;
using namespace cv;

vector<Face> readFeatures(string fname)
{
  ifstream fin(fname.c_str());

  vector<Face> faces;

  if ( !fin.good() )
    return faces;

  float x[5],y[5];
  string imageloc;

  while (fin.good())
  {
    for ( int i = 0; i < 5; i++ )
      fin >> x[i] >> y[i];
    fin >> imageloc;
    if ( fin.good() )
      faces.push_back(
        Face(Point(x[0], y[0]),
             Point(x[1], y[1]),
             Point(x[2], y[2]),
             Point(x[3], y[3]),
             Point(x[4], y[4]),
             imageloc));

  }
}

Mat readFinalLocations(string fname)
{
  ifstream fin(fname.c_str());

  Mat F(5,2,CV_32FC1,Scalar(-1));

  if ( !fin.good() )
    return F;

  fin >> F.at<float>(0,0)
      >> F.at<float>(0,1)
      >> F.at<float>(1,0)
      >> F.at<float>(1,1)
      >> F.at<float>(2,0)
      >> F.at<float>(2,1)
      >> F.at<float>(3,0)
      >> F.at<float>(3,1)
      >> F.at<float>(4,0)
      >> F.at<float>(4,1);

  return F;
}

Mat getPMatrix(Face& face)
{
  Mat ret = Mat(5,3,CV_32FC1);

  for ( int i = 0; i < 5; i++ )
  {
    for ( int j = 0; j < 2; j++ )
      ret.at<float>(i,j)=face.F.at<float>(i,j);
    ret.at<float>(i,2)=1.0;
  }

  return ret;
}

// c1 = [a11,a12,b1]^T
// c2 = [a21,a22,b2]^T
Mat buildInvAffine(Mat& c1, Mat& c2)
{
  float a11 = c1.at<float>(0,0),
        a12 = c1.at<float>(1,0),
        b1 = c1.at<float>(2,0),
        a21 = c2.at<float>(0,0),
        a22 = c2.at<float>(1,0),
        b2 = c2.at<float>(2,0);
  float mult = 1.0/(a11*a22-a21*a12);
  Mat inv = Mat(3,3,CV_32FC1);

  
  inv.at<float>(0,0) = a22*mult;
  inv.at<float>(0,1) = -a21*mult;
  inv.at<float>(0,2) = 0;
  inv.at<float>(1,0) = -a12*mult;
  inv.at<float>(1,1) = a11*mult;
  inv.at<float>(1,2) = 0;
  inv.at<float>(2,0) = (a12*b2-a22*b1)*mult;
  inv.at<float>(2,1) = (a21*b1-a11*b2)*mult;
  inv.at<float>(2,2) = (a11*a22-a21*a12)*mult;

  return inv;
}

void applyTrans( const Mat& img, Mat &newImg, const Mat& T )
{
  Mat point = Mat(1,3,CV_32FC1), current = Mat(1,3,CV_32FC1);
  current.at<float>(0,2) = 1.0;
  float newR, newC;
  for ( int r = 0; r < newImg.rows; r++ )
    for ( int c = 0; c < newImg.cols; c++ )
    {
      current.at<float>(0,0) = (float)c;
      current.at<float>(0,1) = (float)r;
      point = current*T;
      newC = point.at<float>(0,0);
      newR = point.at<float>(0,1);
      
      if ( (int)newR >= img.rows || (int)newR < 0 ||
           (int)newC >= img.cols || (int)newC < 0 )
        newImg.at<uchar>(r,c) = (uchar)0;
      else
        newImg.at<uchar>(r,c) = img.at<uchar>((int)newR,(int)newC);
    }
}

float mag(const Mat& F)
{
  float retVal = 0.0;
  for ( int i = 0; i < F.rows; i++ )
    for ( int j = 0; j < F.cols; j++ )
      retVal += (F.at<float>(i,j)*F.at<float>(i,j));
  return sqrt(retVal);
}

float getDiff(const Mat& F1, const Mat& F2)
{
  Mat diff = F1-F2;
  return (float)(norm(diff));
}

int main(int argc, char *argv[])
{
  // check arguments
  if ( argc < 3 )
  {
    cout << "Please enter the filename of the faces feature locations followed by"
         << "the filename containing the final face locations" << endl;
    return -1;
  }

  float thresh = 0.001;

  // initialize variables
  vector<Face> faces = readFeatures(argv[1]), faces2 = readFeatures(argv[1]);
  Mat F_final = readFinalLocations(argv[2]);
  Mat c1, c2, T, fullT, P, F_bar, F_bar_prime, F_bar_prev, F_i_prime, F_tot, temp, T_temp, px, py;
  Mat img, newImg = Mat(48,40,CV_8UC1);

  vector<Mat> T_inv;

  for ( int i = 0; i < (int)faces.size(); i++ )
  {
    T_inv.push_back(Mat(3,3,CV_32FC1));
    setIdentity(T_inv[i]);
  }

  int iter = 0;
  bool cont;

  int ASDF = 0;
  
  F_tot = Mat(5,2,CV_32FC1);

  P = Mat(5,3,CV_32FC1);
  F_bar_prime = Mat(2,5,CV_32FC1);
    
  // initialize F_bar to first image
  F_bar = faces[0].F.clone();

  do {
    cout << ASDF << endl;
    ASDF++;
    // make copy of F_bar for later
    F_bar_prev = F_bar.clone();

    // set P
    for ( int i = 0; i < 5; i++ )
    {
      P.at<float>(i,0) = F_bar.at<float>(i,0);
      P.at<float>(i,1) = F_bar.at<float>(i,1);
      P.at<float>(i,2) = 1.0;
    }

    // set px and py
    px = F_final.col(0).clone();
    py = F_final.col(1).clone();

    // solve for transform
    solve(P,px,c1,DECOMP_SVD);
    solve(P,py,c2,DECOMP_SVD);

    // build transform matrix
    T = Mat(3,2,CV_32FC1);
    for ( int i = 0; i < 3; i++ )
    {
      T.at<float>(i,0) = c1.at<float>(i,0);
      T.at<float>(i,1) = c2.at<float>(i,0);
    }

    // apply on F_bar (P)
    F_bar_prime = P*T;

    // Update F_bar by setting equal to F_bar_prime
    F_bar = F_bar_prime.clone();
    
    // zero matrix
    F_tot = Mat(5,2,CV_32FC1,Scalar(0.0));

    // For every face image F_i use SVD to align F_i with F_bar to make F_i_prime
    for ( int index = 0; index < (int)faces.size(); index++ )
    {
      // set P
      P = getPMatrix(faces[index]);

      px = F_bar.col(0).clone();
      py = F_bar.col(1).clone();

      // perform SVD
      solve(P,px,c1,DECOMP_SVD);
      solve(P,py,c2,DECOMP_SVD);

      // build the inverse Affine
      temp = buildInvAffine(c1,c2);
      T_temp = T_inv[index].clone();
      T_inv[index] = temp*T_temp;

      // build transform matrix
      for ( int i = 0; i < 3; i++ )
      {
        T.at<float>(i,0) = c1.at<float>(i,0);
        T.at<float>(i,1) = c2.at<float>(i,0);
      }

      // apply on F_i to get F_i_prime
      F_i_prime = P*T;

      // keep running total
      F_tot += F_i_prime;

      // set F_i equal to F_i_prime
      faces[index].F = F_i_prime.clone();
    }

    // update F by average the values of F_i for each image
    F_bar = F_tot/(int)faces.size();

  } while ( getDiff(F_bar,F_bar_prev) > thresh );

  // show results
  for ( int index = 0; index < (int)faces.size(); index++ )
  {
    // read image
    img = imread(faces[index].fname,0);
    
    // apply transform
    applyTrans(img,newImg,T_inv[index]);
   
    //for ( int i = 0; i < 5; i++ )
    //{
    //  circle(newImg,Point(faces[index].F.at<float>(i,0),faces[index].F.at<float>(i,1)),
    //    5,Scalar(255,255,255),1);
    //  circle(newImg,Point(F_final.at<float>(i,0),F_final.at<float>(i,1)),
    //    4,Scalar(0,0,0),1);
    //}

    // save
    ostringstream sout;
    sout << "./results/" << index << ".jpg";
    imwrite(sout.str(),newImg);
  }

  // end
  return 0;
}

