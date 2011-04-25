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

#define USE_MAHALANOBIS
//#define WRITE_EXTRAS

// argv[1] : training data
// argv[2] : test image file
// argv[3] : N
int main(int argc, char *argv[])
{
  if ( argc < 4 )
    return -1;

  int N = atoi(argv[3]);

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

  // build list of test set coefficients
  for ( i = 0; i < testCount; i++ )
  {
    static Mat input;
    input = imread(testImgs[i],0);
    projectFace(input, eFaces, mean, testCoeffs[i]);
    topNMatches(testCoeffs[i], trainCoeffs, lambda, N, trainMatches[i], topNError[i]);
    
    matchValue[i] = matchTest(testIDs[i], trainMatches[i], trainIDs);
    if ( matchValue[i] > 0 )
      matchCount++;
  }
  
  // output accuracy to terminal
  cout << (float)matchCount / testCount << endl;

// write top matches for 3 valid and 3 invalid (optional)
#ifdef WRITE_EXTRAS
  int max = matchValue[0], count=0;
  int usedIDs[] = {0,0,0,0,0,0};
  for ( i = 0; i < testCount; i++ )
    if ( matchValue[i] > max )
      max = matchValue[i];

  // write best 3 matches with top N matches ensuring 3 different IDs
  for ( i = max; i >= 0 && count < 3; i-- )
    for ( j = 0; j < testCount && count < 3; j++ )
      if ( matchValue[j] == i &&
           usedIDs[0] != testIDs[j] && 
           usedIDs[1] != testIDs[j])
      {
        usedIDs[count] = testIDs[j];
        cout << i << endl;
        count++;
        // save image j from test set with best matches
        string path;
        {
          ostringstream sout;
          string a = argv[1];
          sout << "results/correct" << a[a.length()-2] << a[a.length()-1] << '/'
               << count << "/";
          path = sout.str();
        }
        Mat tImg;
        backprojectFace(testCoeffs[j],eFaces,mean,imgSize,tImg,CV_8UC1);
        imwrite(path+(string)"testImg.jpg",tImg);
        
        // write top N matches
        for ( int k = 0; k < N; k++ )
        {
          ostringstream sout;
          sout << path << setfill('0') << setw(2) << k+1 << '_'
               << setw(4) << topNError[j][k]*100000;
          if ( testIDs[j] == trainIDs[trainMatches[j][k]] )
            sout << "_c.jpg";
          else
            sout << "_i.jpg";
          backprojectFace(trainCoeffs[trainMatches[j][k]],eFaces,mean,imgSize,tImg,CV_8UC1);
          imwrite(sout.str(),tImg);
        }
      }

  // write 3 non-matching with top N matches ensuring all have a different ID
  for ( i = 0; i < testCount && count < 6; i++ )
  {
    if ( matchValue[i] <= 0 &&
      usedIDs[0] != testIDs[i] &&
      usedIDs[1] != testIDs[i] &&
      usedIDs[2] != testIDs[i] &&
      usedIDs[3] != testIDs[i] &&
      usedIDs[4] != testIDs[i] )
    {
      usedIDs[count] = testIDs[i];
      count++;
      // save image i from test set with best matches
      string path;
      {
        ostringstream sout;
        string a = argv[1];
        sout << "results/incorrect" << a[a.length()-2] << a[a.length()-1] << '/'
             << count-3 << "/";
        path = sout.str();
      }
      Mat tImg;
      backprojectFace(testCoeffs[i],eFaces,mean,imgSize,tImg,CV_8UC1);
      imwrite(path+(string)"testImg.jpg",tImg);

      for ( int k = 0; k < N; k++ )
      {
        ostringstream sout;
        sout << path << setfill('0') << setw(2) << k+1 << '_'
             << setw(4) << topNError[i][k]*100000 << "_" << trainIDs[trainMatches[i][k]]
             << ".jpg";
        backprojectFace(trainCoeffs[trainMatches[i][k]],eFaces,mean,imgSize,tImg,CV_8UC1);
        imwrite(sout.str(),tImg);
      }
    }
  }

#endif

  return 0;
}

