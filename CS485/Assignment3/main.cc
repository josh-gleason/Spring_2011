#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <iostream>
#include <fstream>
#include <vector>

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

void getAverage(vector<Face>& faces, Mat& F)
{
    // initialize to zero
    for ( int i = 0; i < 5; i++ )
      for ( int j = 0; j < 2; j++ )
        F.at<float>(i,j) = 0;
    
    // get sum
    for ( vector<Face>::iterator it = faces.begin(); it != faces.end(); ++it)
      for ( int i = 0; i < 5; i++ )
        for ( int j = 0; j < 2; j++ )
          F.at<float>(i,j) += (*it).F.at<float>(i,j);
    
    // divide by total
    for ( int i = 0; i < 5; i++ )
      for ( int j = 0; j < 2; j++ )
        F.at<float>(i,j) /= (float)faces.size();
}

void setPMatrix(Face& face, Mat& P)
{
  for ( int i = 0; i < 5; i++ )
    for ( int j = 0; j < 2; j++ )
      face.F.at<float>(i,j)=P.at<float>(i,j);
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
  inv.at<float>(2,0) = (a21*b2-a22*b1)*mult;
  inv.at<float>(2,1) = (a12*b1-a11*b2)*mult;
  inv.at<float>(2,2) = (a11*a22-a21*a12)*mult;

  return inv;
}

void applyAffine(const Mat& c1, const Mat &c2, Mat& P)
{
  Mat px = Mat(5,1,CV_32FC1),
      py = Mat(5,1,CV_32FC1);
  px = P*c1,
  py = P*c2;
  P.col(0) = px.clone();
  P.col(1) = py.clone();

  cout << "px py" << endl;
  for ( int i = 0; i < 5; i++ )
  {
    P.at<float>(i,0) = px.at<float>(i,0);
    P.at<float>(i,1) = py.at<float>(i,0);
    cout << px.at<float>(i,0) << ' ' << py.at<float>(i,0) << endl;
  }
  cout << endl;
}

void applyTrans( const Mat& img, Mat &newImg, const Mat& T )
{
  Mat point = Mat(1,3,CV_32FC1), current = Mat(1,3,CV_32FC1);
  current.at<uchar>(0,2) = (uchar)1;
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

int main(int argc, char *argv[])
{
  // check arguments
  if ( argc < 3 )
  {
    cout << "Please enter the filename of the faces feature locations followed by"
         << "the filename containing the final face locations" << endl;
    return -1;
  }

  // initialize variables
  vector<Face> faces = readFeatures(argv[1]); 
  Mat F_final = readFinalLocations(argv[2]);
  Mat px = F_final.col(0).clone();
  Mat py = F_final.col(1).clone();
  /*Mat c1, c2, T, fullT;
  Mat P, A, b;
  for ( int I = 0; I < 10; I++ )
  {
    Mat img = imread(faces[I].fname,0);
    Mat newImg = Mat(img.rows,img.cols,CV_8UC1);
    for ( int x = 0; x < 8; x++ )
    {
      P = getPMatrix(faces[I]);

      // compute affine transform using SVD
      solve(P,px,c1,DECOMP_SVD);
      solve(P,py,c2,DECOMP_SVD);
      
      T = buildInvAffine(c1,c2);
      if ( x == 0 )
        fullT = T;
      else
        fullT *= T;

      // apply transformation
      applyTrans(img,newImg,fullT);
      applyAffine(c1,c2,P);
      
      for ( int i = 0; i < 5; i++ )
        cout << P.at<float>(i,0) << ' ' << P.at<float>(i,1) << endl;
      cout << endl;
      
      setPMatrix(faces[I],P);

      // display results (temporary)
      imshow("Orig",img);
      for ( int i = 0; i < 5; i++ )
      {
        circle(newImg,Point(px.at<float>(i,0),py.at<float>(i,0)),5,Scalar(125,125,125));
        circle(newImg,Point(P.at<float>(i,0),P.at<float>(i,1)),5,Scalar(255,255,255));
      }
      imshow("New",newImg);
      waitKey(0);
    }
  }// while ( false );
*/
  // save/display results

  // end
  return 0;
}

