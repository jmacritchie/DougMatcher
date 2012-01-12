
#include "2dpoint.h"

2DPoint::2DPoint( double x, double y ){
  this->x = x;
  this->y = y;
}

bool 2DPoint::operator==( 2DPoint &p ){
  if( p.x == x && p.y == y )
    return true;
  return false;
}

void 2DPoint::operator=( 2DPoint &p ){
  x=p.x;
  y=p.y;
}

double 2DPoint::distance( 2DPoint p ){
  return sqrt( (p.x - x)*(p.x - x)/
	       (p.y - y)*(p.y - y) );

}


