#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace std;

template <class T>
void* multRowCol(void* params);

// class prototype
template <class T>
class Matrix;

template <class T>
class Matrix
{
public:
  // constructors
  Matrix(int r, int c, const vector<vector<T> >& d) :
    rows(r), cols(c), data(d) {}

  // initialize empty vector
  Matrix(int r, int c) :
    rows(r), cols(c), data(r, vector<T>(c)) {}

  Matrix(int r, int c, const T** d) :
    rows(r), cols(c), data(r, vector<T>(c))
  {
    int i,j;
    for ( i = 0; i < rows; i++ ) 
      for ( j = 0; j < cols; j++ )
        data[i][j] = d[i][j];
  }
  
  // shallow copy is fine because vectors have copy constructor
  // Matrix( const Matrix<T> rhs );

  // accessors
  vector<T> getRow(const int &row) const
  {
    return data[row];
  }

  vector<T> getCol(const int &col) const
  {
    static vector<T> retVal(rows);
    static int i;
    for ( i = 0; i < rows; i++ )
      retVal[i] = data[i][col];
    return retVal;
  }

  void setRow(const int &row, const vector<T> &v)
  {
    data[row] = v;
  }

  void setCol(const int &col, const vector<T> &v)
  {
    static int i;
    for ( i = 0; i < rows; i++ )
      data[i][col] = v[i];
  }

  int getCols() const
  {
    return cols;
  }

  int getRows() const
  {
    return rows;
  }

  // for setting values
  T& operator() (int row, int col)
  {
    return data[row][col];
  }
  
  // for reading values
  const T& operator() (int row, int col) const
  {
    return data[row][col];
  }

 // public functions
  Matrix<T> operator*( const Matrix<T>& rhs) const
  {
    int outCols = rhs.getCols(), r, c, ri;

    Matrix<T> retVal(rows,outCols);

    for ( r = 0; r < rows; r++ )
      for ( c = 0; c < outCols; c++ )
      {
        retVal(r,c) = 0;
        for ( ri = 0; ri < cols; ri++ )
          retVal(r,c) += (*this)(r,ri)*rhs(ri,c);
      }

    return retVal;
  }
  
protected:
  int rows, cols;
  vector<vector<T> > data;

};

int main(int argc, char *argv[])
{
  ifstream finA(argv[1]), finB(argv[2]);
  int rowsA, rowsB, colsA, colsB;

  finA >> rowsA >> colsA;
  finB >> rowsB >> colsB;
  
  vector<vector<float> > a( rowsA, vector<float> (colsA) );
  vector<vector<float> > b( rowsB, vector<float> (colsB) );

  for ( int r = 0; r < rowsA; r++ )
    for ( int c = 0; c < colsA; c++ )
      finA >> a[r][c];
  for ( int r = 0; r < rowsB; r++ )
    for ( int c = 0; c < colsB; c++ )
      finB >> b[r][c];

  Matrix<float> A(rowsA,colsA,a),B(rowsB,colsB,b);
  Matrix<float> C = A*B;
/*  for ( int i = 0; i < C.getRows(); i++ )
  {
    for ( int j = 0; j < C.getCols(); j++ )
      cout << C(i,j) << ' ';
    cout << endl;
  }
  cout << endl;*/
  return 0;
}
