#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[])
{
  float t1;
  system("rm results.txt");
  // 5
  for ( int count = 0; count < 7; count++ )
  {
    switch (count)
    {
      case 0:
        t1 = 0;
        break;
      case 1:
        t1 = 40000;
        break;
      case 2:
        t1 = 400000;
        break;
      case 3:
        t1 = 4000000;
        break;
      default:
        t1 += 4e+06;
        break;
    }

    for ( int t2 = 3; t2 <= 15; t2+=2 )
    {
      system("cp settings.txt temp.txt");
      
      ostringstream sout;
      
      sout << "./sweep graf/img1.ppm graf/img5.ppm graf/H1to5p " << t1 << ' ' << t2 << ' ' << "temp.txt";

      cout << sout.str() << endl;

      system(sout.str().c_str());

      sout.clear();
      sout.str("");
      
      sout << "./main temp.txt results.txt" << endl;

      cout << sout.str() << endl;

      system(sout.str().c_str());
    }
  }

  return 0;
}
