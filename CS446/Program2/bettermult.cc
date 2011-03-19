#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <pthread.h>

using namespace std;

// the total number threads to be spawned, should be
// equal to number of cores in the CPU
#define TOT_THREADS 4

// comment out if you don't want to display the matrix
#define SHOW_MAT

template <class T>
void* multRowCol(void* params);

// class prototype
template <class T>
class Matrix;

template <class T>
struct funcInfo
{
  funcInfo() {}
  funcInfo(int r1, int r2, const Matrix<T> *left, const Matrix<T> *right) :
    row1(r1), row2(r2), lMat(left), rMat(right)
  {
    col1 = 0; col2 = right->getCols()-1;
    if ( r2 >= r1 )
    {
      rows.resize(r2-r1+1);
      for ( int i = 0; i < (int)rows.size(); i++ )
        rows[i].resize(col2+1);
    }
  }

  int row1, row2, col1, col2;
  const Matrix<T> *lMat, *rMat;
  vector<vector<T> > rows;
};

template <class T>
class Matrix
{
public:
  // constructors
  Matrix() {}

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
    int outCols = rhs.getCols(),
        r1, r2, i, j;

    vector<pthread_t> thrd(TOT_THREADS);
    vector<funcInfo<T> > f(TOT_THREADS);
    void *param, *status;
         
    // create threads
    for ( i = 0; i < TOT_THREADS; i++ )
    {
      r1 = i*(rows-1)/TOT_THREADS;
      if ( i != 0 )
        r1++;
      r2 = (i+1)*(rows-1)/TOT_THREADS;
      f[i] = funcInfo<T>(r1,r2,this,&rhs);
      param = (void*)(&(f[i]));
      pthread_create(&(thrd[i]),NULL,multRowCol<T>,param);
    }
  
    // wait for threads to finish (join)
    for ( i = 0; i < TOT_THREADS; i++ )
      pthread_join(thrd[i],&status);
    
    Matrix retVal(rows,outCols);

    // copy results
    int index = 0, count = 0;
    for ( i = 0; i < rows; i++ )
    {
      while ( i > f[index].row2 && index < TOT_THREADS )
      {
        count = 0;
        index++;
      }
      retVal.setRow(i, f[index].rows[count]);
      count++;
    }
    
    return retVal;
  }
  
protected:
  int rows, cols;
  vector<vector<T> > data;

};

// actually does the multiplication
template <class T>
void* multRowCol(void* params)
{
  funcInfo<T> *f = (funcInfo<T>*)params;
  const Matrix<T> *lhs = f->lMat,
                  *rhs = f->rMat;

  int r,c,i,tempR,tempC,
      row1 = f->row1,
      row2 = f->row2,
      col1 = f->col1,
      col2 = f->col2,
      count = lhs->getCols();  // = rhs->getRows()

  for ( r = row1; r <= row2; r++ )
    for ( c = col1; c <= col2; c++ )
    {
      tempR = r-row1;
      tempC = c-col1;
      (f->rows)[tempR][tempC] = 0;
      for ( int i = 0; i < count; i++ )
        (f->rows)[tempR][tempC] += (*lhs)(r,i)*(*rhs)(i,c);
    }
}

int main(int argc, char *argv[])
{
  if ( argc < 3 )
  {
    cout << "./bettermult <matrix1> <matrix2>" << endl;
    return -1;
  }

  ifstream finA(argv[1]), finB(argv[2]);
  int rowsA, rowsB, colsA, colsB, r, c;

  finA >> rowsA >> colsA;
  finB >> rowsB >> colsB;
  
  vector<vector<float> > a( rowsA, vector<float> (colsA) );
  vector<vector<float> > b( rowsB, vector<float> (colsB) );

  for ( r = 0; r < rowsA; r++ )
    for ( c = 0; c < colsA; c++ )
      finA >> a[r][c];

  for ( r = 0; r < rowsB; r++ )
    for ( c = 0; c < colsB; c++ )
      finB >> b[r][c];

  Matrix<float> A(rowsA,colsA,a),B(rowsB,colsB,b),C;
  C = A*B;

  // print matrix
#ifdef SHOW_MAT
  for ( r = 0; r < C.getRows(); r++ )
  {
    for ( c = 0; c < C.getCols(); c++ )
      cout << C(r,c) << ' ';
    cout << endl;
  }
  cout << endl;
#endif
  return 0;
}
