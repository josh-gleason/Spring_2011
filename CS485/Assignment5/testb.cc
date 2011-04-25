#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include "eigface.h"
#include "test.h"

using namespace std;
using namespace cv;

// argv[1] : training data
// argv[2] : test image file
// argv[3] : N
int main(int argc, char *argv[])
{
  if ( argc < 4 )
    return -1;

  int N = 1; //atoi(argv[3]);
  double thresh = atof(argv[3]);

  vector<Mat> eFaces, trainCoeffs, testCoeffs;
  vector<string> trainImgs, testImgs;
  Mat mean, lambda;
  Size imgSize;
  vector<long> trainIDs, testIDs;
  vector<vector<int> > trainMatches;
  int trainCount, testCount, i ,j;
  vector<int> matchValue;
  vector<vector<double> > topNError;

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
      cout << "Error: Training and Testing images have different dimensions" << endl;
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
  topNError.resize(testCount);

  int matchCount = 0;

  int fp = 0,
      tp = 0,
      intruder = 0,
      nonintruder = 0;

  // build list of test set coefficients
  for ( i = 0; i < testCount; i++ )
  {
    static Mat input;
    input = imread(testImgs[i],0);
    projectFace(input, eFaces, mean, testCoeffs[i]);
    topNMatches(testCoeffs[i], trainCoeffs, lambda, N, trainMatches[i], topNError[i]);
  
    if ( testIDs[i] < 146 ) // intruder
      intruder++;
    else
      nonintruder++;

    if ( topNError[i][0] < thresh )
      if ( testIDs[i] < 146 ) // false positive
        fp++;
      else  // true positive
        tp++;
  }
  
  // output results to terminal
  cout << (float)tp/nonintruder << ' ';
  cout << (float)fp/intruder << endl;
  
  return 0;
}

