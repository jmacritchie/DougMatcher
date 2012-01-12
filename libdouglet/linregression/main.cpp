

#include <iostream>
#include <fstream>
#include <freqfile.h>
#include <bfline.h>

using namespace std;

/*
  This program will simply test the linear regression
  class. Given an ascii file, it will extract the 
  value pairs on each line, add them to the BFLine
  class and report the best fit line through the points.
  Finally, the points and line will be plotted.
 */

void usage(){
  cout << "testbf <file1> <file2>\n"
       << "The files should contain a list of point pairs\n"
       << "The best fit line will be plotted for file1, then\n"
       << "file2 is added, plotted and  removed again\n";
  exit(1);
}

int main( int argc, char **argv ){
  
  if( argc != 3 )
    usage();
  ifstream in( argv[1] );
  ifstream in2( argv[2] );

  BFLine line;
  Point p;

  while( !in.eof() ){
    in >> p.x;
    in >> p.y;
    cout << p.x << " " << p.y << endl;
    line.appendPoint(p);
  }
  line.plot();
  sleep(3);

  cout << "Position " << in2.tellg() << endl;
  while( !in2.eof() ){
    in2 >> p.x;
    in2 >> p.y;
    cout << p.x << " " << p.y << endl;
    line.appendPoint(p);
  }
  line.plot();
  sleep(3);
  in2.clear();
  in2.close();
  in2.open(argv[2]);
  cout << "I am here\n";
  while( !in2.eof() ){
    in2 >> p.x;
    in2 >> p.y;
    cout << p.x << " " << p.y << endl;
    line.removePoint(p);
  }

  line.plot();

}
