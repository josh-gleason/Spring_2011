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


  for ( int i = 0; i < 5; i++ )
  {
    P.at<float>(i,0) = px.at<float>(i,0);
    P.at<float>(i,1) = py.at<float>(i,0);
  }
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

Mat getAvg(const Mat& P)
{
  Mat retVal = Mat(1,2,CV_32FC1,Scalar(0));
  for ( int i = 0; i < P.rows; i++ )
  {
    retVal.at<float>(0,0) += P.at<float>(i,0);
    retVal.at<float>(0,1) += P.at<float>(i,1);
  }
  retVal.at<float>(0,0) *= 1.0/P.rows;
  retVal.at<float>(0,1) *= 1.0/P.rows;

  return retVal;
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

  float thresh = 1.0;

  // initialize variables
  vector<Face> faces = readFeatures(argv[1]); 
  Mat F_final = readFinalLocations(argv[2]);
  Mat px = F_final.col(0).clone();
  Mat py = F_final.col(1).clone();
  Mat c1, c2, T, fullT, P;
  Mat img, newImg = Mat(48,40,CV_8UC1);
  vector<Mat> F;

  int iter = 0;
  bool cont;

  for ( int index = 0; index < (int)faces.size(); index++ )
  {
    // read image from file
    img = imread(faces[index].fname,0);

    // build P matrix for face
    P = getPMatrix(faces[index]);

    // clear and push new F matrix to list
    F.clear();
    F.push_back(getAvg(P));
   
    iter = 1;

    do {
      // compute affine transform using SVD
      solve(P,px,c1,DECOMP_SVD);
      solve(P,py,c2,DECOMP_SVD);
      
      // apply transformation
      applyAffine(c1,c2,P);
      
      // push new F matrix to list
      F.push_back(getAvg(P));
      
      // construct transform
      T = buildInvAffine(c1,c2);
      ( iter == 1 ? fullT = T : fullT *= T );
      
      // check to see if we should continue
      cont = ( mag(F[iter]-F[iter-1]) < thresh ? false : true );

      iter++;
    } while ( cont ); 
    
    // build new image
    applyTrans(img,newImg,fullT);
    imshow("Orig",img);
    imshow("New",newImg);
    waitKey(0);
   
    // save results
    ostringstream sout;
    sout << "./results/" << index << ".jpg";
    imwrite(sout.str(),newImg);
  }
  
  // end
  return 0;
}

