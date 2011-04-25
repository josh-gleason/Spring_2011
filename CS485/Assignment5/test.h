#ifndef TEST_H_JOSH
#define TEST_H_JOSH

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
void topNMatches(const Mat& input, const vector<Mat>& coeffs, const Mat& eValues, int N,
  vector<int>& matching, vector<double>& matchValues)
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

#endif // TEST_H_JOSH

