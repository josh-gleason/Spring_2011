#include <fstream>
#include <sstream>

using namespace std;

struct Settings
{
  Settings(string filename)
  {
    ifstream fin(filename.c_str());
    istringstream sin;

    string line, param;
    
    while ( fin.good() )
    {
      getline(fin,line);
      if ( fin.good() )
      {
        if ( line[0] != '#' && line[0] != '\n' )
        {
          sin.clear();
          sin.str(line);

          sin >> param;
    
          ///////////////////////////////////////////////////////////////

          if ( param == "IMAGE" )
            sin >> imagename;
          else if ( param == "SIGMA_0" )
            sin >> sigma0;
          else if ( param == "K" )
            sin >> k;
          else if ( param == "LEVELS" )
            sin >> levels;
          else if ( param == "ALPHA" )
            sin >> alpha;
          else if ( param == "T1" )
            sin >> t1;
          else if ( param == "T2" )
            sin >> t2;
          
          ///////////////////////////////////////////////////////////////
        }
      }
    }
    fin.close();
  }

  string imagename;
  
  float sigma0;
  float k;
  int levels;
  
  float alpha;

  float t1;
  float t2;
};
