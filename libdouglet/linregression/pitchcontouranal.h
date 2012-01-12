
#include <list>

typedef enum{ PEAK, TROUGH, UP, DOWN, LEVEL_UP, 
		LEVEL_DOWN, UP_LEVEL, DOWN_LEVEL } Shape;

typedef struct{
  double x;
  double y;
}Point;


typedef std::list<Point> PointList;
typedef std::list<Point>::iterator PointsListIt;

class BFLine{
  
public:
  BFLine();
  BFLine( PointsList &points );
  
  void addPoint( Point p ); // add point and sort
  void appendPoint( Point p );
  void prependPoint( Point p );
  
  void removeLastPoint();
  void removeFirstPoint();
  void removePoint( Point &p );
  
  double getBFGradient();
  double getBFIntercept();

  double xFromY( double y );
  double yFromX( double x );
  
protected:
  
  double grad_;  // gradient
  double inter_; // y when x=0; 

  // hold total values for easy extension of line
  double m_sumX;
  double m_sumY;
  double m_sumX2;
  double m_sumY2;
  
}

