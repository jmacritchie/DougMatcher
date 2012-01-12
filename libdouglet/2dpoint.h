#ifndef 2DPOINT_H_
#define 2DPOINT_H_

class 2DPoint {
 public:
  double x;
  double y;

  2DPoint( double, double );
  bool operator==(2DPoint& p );	
  2DPoint &operator=( 2DPoint& p );
  double distance( 2DPoint p );
};



#endif


