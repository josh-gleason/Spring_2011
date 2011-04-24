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
//#define WRITE_EXTRAS

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
void topNMatches(const Mat& input, const vector<Mat>& coeffs, const Mat& eValues, int N, vector<int>& matching, vector<double>& matchValues)
{
  matchValues.resize(N,0);    // holds the match score
  matching.resize(N,-1);      // holds the index

  int setSize = coeffs.size(), i, j, k, val1;
  double e, val2;
  Mat diff, t;

  for ( i = 0; i < setSize; i++ )
  {

#ifdef USE_MAHALANOBIS
    diff = input - coeffs[i];
    diff = diff.mul(diff);
    diff = diff.mul(1.0/eValues);
    e = norm(diff,NORM_L1);
#else   // use L2 norm
    e = norm(input,coeffs[i],NORM_L2);
#endif  // USE_MAHALANOBIS
    
    for ( j = 0; j < N; j++ )
    {
      if ( matching[j] < 0 || e < matchValues[j] )
        break;
    }

    if ( j < N )  // insert element here
    {
      if ( matching[j] < 0 )  // special case
      {
        matching[j] = i;
        matchValues[j] = e;
      }
      else
      {
        val1 = i;
        val2 = e;
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

int matchTest(long ID, const vector<int>& indexMatches, const vector<long>& trainIDs)
{
  int count = 0;
  for ( int i = 0; i < indexMatches.size(); i++ )
    if ( ID == trainIDs[indexMatches[i]] )
      count++;
  return count;
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

  cout << (float)matchCount / testCount << endl;

  return 0;
}

