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
          else if ( param == "DISPLAY_DOG" )
            sin >> showDoG;
          else if ( param == "DISPLAY_HARRIS" )
            sin >> showHarris;
          else if ( param == "DISPLAY_ORIGINAL_POINTS" )
            sin >> showOrigPoints;
          else if ( param == "DISPLAY_IN_BOUNDS_POINTS" )
            sin >> showInBoundsPoints;
          else if ( param == "DISPLAY_MATCHING" )
            sin >> showMatching;
          else if ( param == "IMAGE_COMPARE" )
            sin >> image2name;
          else if ( param == "HFILE" )
            sin >> hfilename;
          else if ( param == "MATCH_THRESHOLD" )
            sin >> matchThresh;
          else if ( param == "USE_OPENCV_HARRIS" )
            sin >> useOpenCVHarris;
          else if ( param == "OPENCV_HAR_I_MULT" )
            sin >> openCVHarI;
          else if ( param == "OPENCV_HAR_D_MULT" )
            sin >> openCVHarD;
          else if ( param == "ENABLE_HARRIS" )
            sin >> enableHarris;
          else if ( param == "DMAX_THRESH" )
            sin >> doGMaxThresh;
          else if ( param == "HAR_MAX_THRESH" )
            sin >> harrisMaxThresh;
          else if ( param == "ENABLE_DOG" )
            sin >> enableDoG;
          else if ( param == "BOUNDS_CHECK" )
            sin >> boundsCheck;
          else if ( param == "HAR_SCALE_SIGMA" )
            sin >> harScaleSigma;
          else if ( param == "REMOVE_REPEATS" )
            sin >> removeRepeats;
          else if ( param == "SCALE_MULTIPLIER" )
            sin >> scaleMultiplier;
          else if ( param == "HAR_ADAPTIVE_SCALE" )
            sin >> harAdaptiveScale;
          else if ( param == "HAR_ADD_SIGMA" )
            sin >> harAddSigma;
          else if ( param == "HAR_REGION_SCALE" )
            sin >> harRegionScale;
          else if ( param == "SIGMA_D_WINDOW" )
            sin >> harSigmaDWin;
          else if ( param == "SIGMA_I_WINDOW" )
            sin >> harSigmaIWin;
          else if ( param == "USE_SHORTEST_DIST" )
            sin >> shortestDist;
          else if ( param == "CIRCLE_THICKNESS" )
            sin >> circleThickness;
          else if ( param == "CIRCLE_RADIUS_MULT" )
            sin >> circleMult;
          else if ( param == "CROSS_THICKNESS" )
            sin >> crossThickness;
          else if ( param == "CROSS_SIZE" )
            sin >> crossSize;
          else if ( param == "CROSS_COLOR" )
            sin >> crossB >> crossG >> crossR;
          else if ( param == "CIRCLE_COLOR" )
            sin >> circleB >> circleG >> circleR;
          
          ///////////////////////////////////////////////////////////////
        }
      }
    }
    fin.close();
    if ( useMax )
      k = pow(sigmaMax/sigma0,1.0/(levels-1));
    else
      sigmaMax = sigma0 * pow(k,levels-1);
  }

  string imagename, image2name, hfilename;
  string saveFilename;
  int circleThickness, crossThickness, crossSize;
  float circleMult, crossB, crossG, crossR;
  float circleB, circleG, circleR;
  bool saveFile;
  bool useMax;
  bool doGAllowEqual;
  bool doGUseDiff;
  bool doGNormalize;
  bool enableHarris;
  bool enableDoG;
  bool boundsCheck;
  bool removeRepeats;
  bool shortestDist;
  float harAdaptiveScale;
  float harRegionScale;
  float scaleMultiplier;
  float harScaleSigma;
  float doGMaxThresh;
  float harrisMaxThresh;
  float matchThresh;
  float openCVHarI;
  float openCVHarD;
  float harAddSigma;
  float harSigmaDWin;
  float harSigmaIWin;
  int doGRegionX, doGRegionY, doGRegionZ;
  int harRegionX, harRegionY; 
  bool harAllowEqual;
  bool harUseDiff;
  bool useHarrisLocs;
  int useOpenCVHarris;
  int acceptX, acceptY;
  float sigma0;
  float k;
  int levels;
  float sigmaMax;
  float winMult;
  float sigmaDRatio;
  bool showDoG, showHarris, showOrigPoints, showInBoundsPoints, showMatching;
  float alpha;

  float t1;
  float t2;
};
