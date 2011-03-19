#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  ofstream fout(argv[3]);
  int rows = atoi(argv[1]);
  int cols = atoi(argv[2]);

  srand( time(0) );

  fout << rows << ' ' << cols << endl;

  for ( int i = 0; i < rows; i++ )
  {
    for ( int j = 0; j < cols; j++ )
      fout << rand() % 20 << ' ';
    fout << endl;
  }

  fout.close();
  return 0;
}
