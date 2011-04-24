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

#define USE_MAHALANOBIS

bool loadEigenfaces( string filename, vector<Mat>& eFaces, Mat& eValues, vector<Mat>& coeffs,
  vector<string>& images, Mat& mean, Size& imgSize )
{
  ifstream fin(filename.c_str());

  if ( !fin.good() )
    return false;

  int dim, imgCount, pixels, i, j;

  // read useful information
  fin >> imgSize.height >> imgSize.width >> dim >> imgCount;

  pixels = imgSize.height*imgSize.width;
  
  // resize vectors for speed
  mean = Mat(pixels,1,CV_32FC1);
  
  eValues = Mat(dim,1,CV_32FC1);
  eFaces.resize(dim);
  for ( i = 0; i < dim; i++ )
    eFaces[i] = Mat(pixels,1,CV_32FC1);

  images.resize(imgCount);
  coeffs.resize(imgCount);
  for ( i = 0; i < imgCount; i++ )
    coeffs[i] = Mat(dim,1,CV_32FC1);

  // read mean face
  for ( i = 0; i < pixels; i++ )
    fin >> mean.at<float>(i,0);

  // read eigenvalues
  for ( i = 0; i < dim; i++ )
    fin >> eValues.at<float>(i,0);

  // read eigenfaces
  for ( i = 0; i < dim; i++ )
    for ( j = 0; j < pixels; j++ )
      fin >> eFaces[i].at<float>(j,0);

  // read coefficients
  for ( i = 0; i < imgCount; i++ )
  {
    fin >> images[i];
    for ( j = 0; j < dim; j++ )
      fin >> coeffs[i].at<float>(j,0);
  }

  fin.close();

  return true;
}

template<class T>
void my_swap(T& a, T& b)
{
  T temp = a;
  a = b;
  b = temp;
}

// complicated but fast for relatively small N (doesn't need to sort entire list)
void topNMatches(const Mat& input, const vector<Mat>& coeffs, const Mat& eValues, int N, vector<int>& matching)
{
  vector<double> matchValues(N); // holds the match score
  matching.resize(N,-1);      // holds the index

  int setSize = coeffs.size(), i, j, k, val1;
  double score, val2;
  Mat diff, t;

  for ( i = 0; i < setSize; i++ )
  {

#ifdef USE_MAHALANOBIS
    diff = input - coeffs[i];
    diff = diff.mul(diff);
    diff = diff.mul(1.0/eValues);
    score = norm(diff,NORM_L1);
#else   // use L2 norm
    score = norm(input,coeffs[i],NORM_L2);
#endif  // USE_MAHALANOBIS
    
    for ( j = 0; j < N; j++ )
    {
      if ( matching[j] < 0 || score < matchValues[j] )
        break;
    }

    if ( j < N )  // insert element here
    {
      if ( matching[j] < 0 )  // special case
      {
        matching[j] = i;
        matchValues[j] = score;
      }
      else
      {
        val1 = i;
        val2 = score;
        for ( k = j; k < N; k++ )
        {
          if ( matching[k] < 0 ) // unfilled position, place here
          {
            matching[k] = val1;
            matchValues[k] = val2;
            break;
          } 
          else if ( val2 < matchValues[k] ) // swap down
          {
            swap(val1,matching[k]);
            swap(val2,matchValues[k]);
          }
          else
            break;
        }
      }
    }
  }
}

bool matchTest(long ID, const vector<int>& indexMatches, const vector<long>& trainIDs)
{
  for ( int i = 0; i < indexMatches.size(); i++ )
    if ( ID == trainIDs[indexMatches[i]] )
      return true;
  return false;
}

// returns the ID based on file path
long getID( const string pathname )
{
  // find the leftmost '/' (or from beginning if none)
  int lastFSlash = -1;
  for ( int l = pathname.length()-1; l >= 0; l-- )
    if ( pathname[l] == '/' )
    {
      lastFSlash = l;
      break;
    }
  
  // read the first 5 after the last '/'
  long id = 0;
  int tens = 1; // never gets over 10,000 so no need for long
  for ( int j = lastFSlash+5; j > lastFSlash; j-- )
  {
    id += (long)(pathname[j]-'0')*tens;
    tens *= 10;
  }
  return id;
}

// figure out the unique face ID based on the filename
void buildIDs(const vector<string>& images, vector<long>& idList)
{
  int size = images.size();
  idList.resize(size);

  for ( int i = 0; i < size; i++ )
    idList[i] = getID(images[i]);
}

int main(int argc, char *argv[])
{
  if ( argc < 2 )
    return -1;

  int N = atoi(argv[3]);

  vector<Mat> eFaces, trainCoeffs, testCoeffs;
  vector<string> trainImgs, testImgs;
  Mat mean, lambda;
  Size imgSize;
  vector<long> trainIDs, testIDs;
  vector<vector<int> > trainMatches;
  int trainCount, testCount, i ,j;
  vector<bool> matchValue;

  // load training data
  if ( !loadEigenfaces(argv[1],eFaces,lambda,trainCoeffs,trainImgs,mean,imgSize) )
    return -1;
  
  // build ID list from image names
  buildIDs(trainImgs,trainIDs);

  // get training set size
  trainCount = trainImgs.size();

  // read test images folder
  {
    Size tempSize;
    readImagesFile(argv[2], testImgs, tempSize);
    if ( tempSize != imgSize )
    {
      cout << "Error: Training and Testing images of different dimensions" << endl;
      return -1;
    }
  }

  // build ID list from image names
  buildIDs(testImgs,testIDs);

  // get testing set size
  testCount = testImgs.size();

  testCoeffs.resize(testCount);
  trainMatches.resize(testCount);
  matchValue.resize(testCount);

  int matchCount = 0;

  // build list of test set coefficients
  for ( i = 0; i < testCount; i++ )
  {
    static Mat input;
    input = imread(testImgs[i],0);
    projectFace(input, eFaces, mean, testCoeffs[i]);
    topNMatches(testCoeffs[i], trainCoeffs, lambda, N, trainMatches[i]);
    
    matchValue[i] = matchTest(testIDs[i], trainMatches[i], trainIDs);
    if ( matchValue[i] )
      matchCount++;

    // TODO : temporary
    // Mat back;
    // backprojectFace(testCoeffs[i], eFaces, mean, imgSize, back, CV_8UC1);
    // imshow("a",back);
    // imshow("b",input);
    // waitKey(0);
  }

  cout << (float)matchCount / testCount << endl;

  return 0;
}

