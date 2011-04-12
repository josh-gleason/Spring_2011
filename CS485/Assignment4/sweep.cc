#include <fstream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[])
{
  if ( argc < 7 )
    return -1;

  string inputImg = argv[1];
  string inputImg2 = argv[2];
  string hfile = argv[3];
  float t1 = atof(argv[4]);
  float t2 = atof(argv[5]);
  ofstream fout(argv[6],ios::app);

  fout << endl
       << "IMAGE " << inputImg << endl
       << "IMAGE_COMPARE " << inputImg2 << endl
       << "HFILE " << hfile << endl
       << "T1 " << t1 << endl
       << "T2 " << t2 << endl
       << endl;

  fout.close();

  return 0;
}
