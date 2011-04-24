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

//#define WRITE_EXTRAS

int findK(const Mat& eigValues, float thresh)
{
  if ( thresh >= 1 )
    return eigValues.rows;

  int i;
  float total = 0, sum = 0;
  // find sum of eigen values
  for ( i = 0; i < eigValues.rows; i++ )
    total += eigValues.at<float>(i,0);

  for ( i = 0; i < eigValues.rows; i++ )
  {
    sum += eigValues.at<float>(i,0);
    if ( sum / total >= thresh )
      return i+1;
  }

  return eigValues.rows;
}

void saveEigenfaces( string filename, const vector<Mat>& eFaces,
  const vector<vector<double> >& coeffs, const vector<string>& images,
  const Mat& mean, const Size& imgSize )
{
  ofstream fout(filename.c_str());
  
  int pixels = imgSize.height * imgSize.width,
      dim = eFaces.size(),
      imgCount = images.size(),
      i, j;

  // fout.precision(100);

  // write useful information
  fout << imgSize.height << ' ' << imgSize.width << ' ' << dim << ' ' << imgCount << endl;
  
  // write mean face
  for ( i = 0; i < pixels; i++ )
    fout << ' ' << mean.at<float>(i,0);
  fout << endl;

  // write eigenfaces
  for ( i = 0; i < dim; i++ )
  {
    for ( j = 0; j < pixels; j++ )
      fout << ' ' << eFaces[i].at<float>(j,0);
    fout << endl;
  }

  // write coefficients
  for ( i = 0; i < imgCount; i++ )
  {
    fout << ' ' << images[i];
    for ( j = 0; j < dim; j++ )
      fout << ' ' << coeffs[i][j];
    fout << endl;
  }

  fout.close();
}

// argv[1] : training images
// argv[2] : percentage acc
// argv[3] : output file
int main(int argc, char *argv[])
{
  if ( argc < 4 ) 
    return -1;

  // declare variables
  Size imgSize;
  vector<string> imgList;
  float thresh;
  int k, i;

  Mat mean, A, Atrans, lambda, eigVectors;

  vector<Mat> u;
  vector<double> w;
  
  // get desired reconstruction error
  thresh = atof(argv[2]);

  // read input file
  readImagesFile(argv[1],imgList,imgSize);

  // compute mean face
  meanFace(imgSize,imgList,mean);

  // build the A matrix and calculate transpose
  buildA(mean, imgList, A);
  transpose(A,Atrans);

  // calculate eigen vals/vectors
  eigen(Atrans*A, lambda, eigVectors);

  // find minimum k to satisfy threshold
  k = findK(lambda, thresh);
  cout << "Eigenvectors Kept: " << k << endl;

  // take top k eigen vectors, normalize and store into array
  for ( i = 0; i < k; i++ )
  {
    static Mat t1, t2, t3, v;
    t1 = eigVectors.row(i).clone();
    transpose(t1,v);
    t2 = A*v;
    normalize(t2, t3);

    u.push_back(t3.clone());
  }
  
#ifdef WRITE_EXTRAS
  // save mean face
  {
    Mat meany;
    delinearizeImage(mean,meany,imgSize,CV_8UC1);
    imwrite("results/mean_face.jpg",meany);
  }

  // save largest/smallest eigenfaces
  for ( i = 0; i < 10; i++ )
  {
    static Mat t1, t2;
    delinearizeImage(u[i],t1,imgSize);
    normalize(t1,t2,0,255,CV_MINMAX,CV_8UC1);
    ostringstream sout;
    sout << "results/eigenfaces/largest" << i+1 << ".jpg";
    imwrite(sout.str(), t2);
  }

  for ( i = 0; i < 10; i++ )
  {
    static Mat t1, t2, t3, v;
    int index = imgList.size()-i-1;
    t1 = eigVectors.row(index).clone();
    transpose(t1,v);
    t2 = A*v;
    normalize(t2,t3);
    delinearizeImage(t3,t1,imgSize);
    normalize(t1,t2,0,255,CV_MINMAX,CV_8UC1);
    ostringstream sout;
    sout << "results/eigenfaces/smallest" << i+1 << ".jpg";
    imwrite(sout.str(), t2);
  }
#endif // WRITE_EXTRAS

  // TODO : Temporary testing
  // Mat image = imread(imgList[0],0), back;
  // // project face to eigenspace (returns coefficients)
  // projectFace(image, u, mean, w);
  // // back project face from eigenspace
  // backprojectFace(w, u, mean, imgSize, back, CV_8UC1);
  // imshow("orig",image);
  // imshow("backproj",back);
  // waitKey(0);

  // TODO : calculate coefficients for all images
  vector<vector<double> > coeffs;

  for ( i = 0; i < imgList.size(); i++ )
  {
    Mat image = imread(imgList[i],0);
    vector<double> m;
    projectFace(image,u,mean,m);
    coeffs.push_back(m);
  }
  
  // save results to file
  saveEigenfaces(argv[3],u,coeffs,imgList,mean,imgSize);

  // exit
  return 0;
}

