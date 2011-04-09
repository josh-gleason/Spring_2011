#include <fstream>
#include <sstream>

using namespace std;

struct Settings
{
  Settings(string filename) : saveFile(false), useMax(false)
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
          else if ( param == "HAR_ALPHA" )
            sin >> alpha;
          else if ( param == "T1" )
            sin >> t1;
          else if ( param == "T2" )
            sin >> t2;
          else if ( param == "USE_SIGMA_MAX" )
            sin >> useMax;
          else if ( param == "SIGMA_MAX" )
          {
            sin >> sigmaMax;
            useMax = true;
          }
          else if ( param == "SAVE_IMAGE" )
          {
            sin >> saveFilename;
            saveFile = true;
          }
          else if ( param == "DOG_REGION_X" )
            sin >> doGRegionX;
          else if ( param == "DOG_REGION_Y" )
            sin >> doGRegionY;
          else if ( param == "DOG_REGION_Z" )
            sin >> doGRegionZ;
          else if ( param == "DMAX_ALLOW_EQUAL" )
            sin >> doGAllowEqual;
          else if ( param == "GAUSS_WIN_MULTIPLIER" )
            sin >> winMult;
          else if ( param == "DMAX_USE_DIFF" )
            sin >> doGUseDiff;
          else if ( param == "DOG_NORMALIZE" )
            sin >> doGNormalize;
          else if ( param == "HAR_SIGMA_D_RATIO" )
            sin >> sigmaDRatio;
          else if ( param == "HAR_REGION_X" )
            sin >> harRegionX;
          else if ( param == "HAR_REGION_Y" )
            sin >> harRegionY;
          else if ( param == "HAR_ALLOW_EQUAL" )
            sin >> harAllowEqual;
          else if ( param == "HAR_USE_DIFF" )
            sin >> harUseDiff;
          else if ( param == "ACCEPT_REGION_X" )
            sin >> acceptX;
          else if ( param == "ACCEPT_REGION_Y" )
            sin >> acceptY;
          else if ( param == "USE_HARRIS_LOCS" )
            sin >> useHarrisLocs;
          
          ///////////////////////////////////////////////////////////////
        }
      }
    }
    fin.close();
    if ( useMax )
      k = pow(sigmaMax/sigma0,1.0/(levels-1));
  }

  string imagename;
  string saveFilename;
  bool saveFile;
  bool useMax;
  bool doGAllowEqual;
  bool doGUseDiff;
  bool doGNormalize;
  int doGRegionX, doGRegionY, doGRegionZ;
  int harRegionX, harRegionY; 
  bool harAllowEqual;
  bool harUseDiff;
  bool useHarrisLocs;
  int acceptX, acceptY;
  float sigma0;
  float k;
  int levels;
  float sigmaMax;
  float winMult;
  float sigmaDRatio;
  
  float alpha;

  float t1;
  float t2;
};
