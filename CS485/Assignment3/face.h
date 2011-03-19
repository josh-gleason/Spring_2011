
using namespace cv;

struct Face
{
  Face( Point lEye, Point rEye, Point nose, Point mouth, Point chin, string _fname ) :
    F(5,2,CV_32FC1), fname(_fname)
  {
    F.at<float>(0,0) = lEye.x;
    F.at<float>(0,1) = lEye.y;
    F.at<float>(1,0) = rEye.x;
    F.at<float>(1,1) = rEye.y;
    F.at<float>(2,0) = nose.x;
    F.at<float>(2,1) = nose.y;
    F.at<float>(3,0) = mouth.x;
    F.at<float>(3,1) = mouth.y;
    F.at<float>(4,0) = chin.x;
    F.at<float>(4,1) = chin.y;
  }
  
  string getFilename() const
  {
    return fname;
  }

  Point getLeftEye() const
  {
    return Point(F.at<float>(0,0),F.at<float>(0,1));
  }

  Point getRightEye() const
  {
    return Point(F.at<float>(1,0),F.at<float>(1,1));
  }
  
  Point getNose() const
  {
    return Point(F.at<float>(2,0),F.at<float>(2,1));
  }
  
  Point getMouth() const
  {
    return Point(F.at<float>(3,0),F.at<float>(3,1));
  }
  
  Point getChin() const
  {
    return Point(F.at<float>(4,0),F.at<float>(4,1));
  }
  
  void setFilename( string _fname )
  {
    fname = _fname;
  }

  void setLeftEye( Point p )
  {
    F.at<float>(0,0)=p.x;
    F.at<float>(0,1)=p.y;
  }

  void setRightEye( Point p )
  {
    F.at<float>(1,0)=p.x;
    F.at<float>(1,1)=p.y;
  }
  
  void setNose( Point p )
  {
    F.at<float>(2,0)=p.x;
    F.at<float>(2,1)=p.y;
  }
  
  void setMouth( Point p )
  {
    F.at<float>(3,0)=p.x;
    F.at<float>(3,1)=p.y;
  }
  
  void setChin( Point p )
  {
    F.at<float>(4,0)=p.x;
    F.at<float>(4,1)=p.y;
  }
  
  Mat F;
  string fname;
};

