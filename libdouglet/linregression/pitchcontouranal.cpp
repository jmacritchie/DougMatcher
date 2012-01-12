
#include <math.h>
#include <stdio>

using namespace std;

#define LOG cout;

/* ANALYSE PITCH CONTOUR: 
    peak, trough, up/down slope
    long note, short note
*/






/// Find best fit line for points list
BFLine::BFLine( std::list<Point> &points ) :
  m_sumX(0),
  m_sumY(0),
  m_sumX2(0),
  m_sumY2(0),
  points_(points)
{

  for( LineIt it=points_.begin(); it<points_.size(); it++ ){
    double x = it->x;
    double y = it->y;
    sumX_ += x;
    sumY_ += y;
    sumX2_ += x*x;
    sumY2_ += y*y;
  }
  calculate();
}


void BFLine::calculate(){

  grad_ = ( points_.size()*sumXY - sumX * sumY ) /
                    ( points_.size()*sumX2 - sumX * sumX );

  inter_ = ( sumY - points_.size()*sumX ) / points_.size();

  //iterate through points, find distance from line
  totalErr_=0;
  for( LineIt it=points_.begin(); it<points_.size(); it++ )
    totalErr_ += it->y - yFromX( it->x );

  //normalise distance wrt gradient and length
  double theta = atan( grad_ );
  totalErr_ *= cos( theta );
  totalErr_ /= points.size();

  LOG << "Total Error = " << totalErr_ << endl;
}

// Add point, later to sort by line rather than by x
void BFLine::addPoint( Point p ){
  //points_.append( p );
  //calculate();
  //sortPoints();
}

void BFLine::appendPoint( Point p ){points_.push_back(p);calculate();}
void BFLine::prependPoint( Point p ){points_.push_front(p);calculate();}
void BFLine::removeLastPoint(){points_.pop_back();calculate();}
void BFLine::removeFirstPoint(){points_.pop_front():calculate();}

void BFLine::removePoint( Point &p ){

  PointsListIt it = points_.begin();
  while( it != points_end() ){
    if( *it == p ){
      points.erase( it );
      return;
    }
  }
  LOG << "Point not found, cannot remnove from list\n";


}

double BFLine::xFromY( double y ){ return ( y - inter_ ) / grad_; }
double BFLine::yFromX( double x ){ return ( x * grad_ ) + inter_; }

// Vector quantisation using distance from BF line

// Take whole contour, 
/* For all groups:
 
     calculate Best Fit line
     calculate error
     
     if iteration Error < threshold 
       recalculate

     if total improvement < threshold stop

   else
*/


/*  Or - regressive 
Get line - minimum length.

If error over threshold, advance window till threshold is met.

Iterate:
  Advance end of line till until error is over threshold.
  Then add new line and increase size to minimum size
  Increase this line to *some point*. 
        (Maybe some threshold length error threshold)
  Re-evaluate boundary between this line and last
*/




void analysePitchContour(){
  
  Shape shape;
  
  if( prev < current > next )        shape = PEAK;
  else if( prev > current < prev  )  shape = TROUGH;
  else if( prev < current < prev  )  shape = UP;
  else if( prev > current > prev  )  shape = DOWN;
  else if( prev == current < prev  ) shape = LEVEL_UP;
  else if( prev == current > prev  ) shape = LEVEL_DOWN;
  else if( prev < current == prev  ) shape = UP_LEVEL;
  else if( prev > current == prev  ) shape = DONW_LEVEL;



}


/* ANALYSE NOTE CONTOUR

    average frequency,
    starting frequency
    contour shape:
*/
