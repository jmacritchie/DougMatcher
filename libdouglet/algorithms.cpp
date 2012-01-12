

#include "algorithms.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;



string filename = "temp.dat"; //fix later
  
string Algorithms::itoa( int i ){
  std::stringstream ss;
  ss <<  i;
  return ss.str();
}

string Algorithms::ftoa( double f ){
  std::stringstream ss;
  ss <<  f;
  return ss.str();
}


void Algorithms::autocorrelation( int lags, double *ac, double *frame, int length ){


  //find mean value
  double  mean=0;
  for( int i=0; i<length; i++ ) mean += frame[i];
  mean /= length;


  for( int t=0; t<lags; t++ ){
    ac[t]=0;
    for( int i=0; i<(length-t); i++ ){
      ac[t] += ( frame[i]-mean ) * ( frame[i+t]-mean );
    }

    // should it be divided by length or length-t.
    // scilab does length, but this has a weighting effect .. hmmm
    ac[t] /= (length); 
  }

}




void Algorithms::plotComplexData( string title, complex double *data, int size ){


    //open datafile
    ofstream datafile(filename.c_str());
    
    for( int i=0; i<size; i++ ){
      datafile << i << "\t" << data[i] << endl;
    }
    
    datafile.close();
    
    //open gnuplot
    GnuStream gnu;
    
    string title1(title);
    title1.append("(real)");

    gnu << "plot '" << filename << "' using 1:2 title '" << title << "' with lines\n";
    gnu.plot();
    
}

void Algorithms::hamming( double *data, int wSize, int size ){
  hamming( data, data, wSize, size );
}

void Algorithms::hamming( double *inData, double *outData, int wSize, int size ){

  if( size==0 ) size = wSize;

  cout << "Windowed Frame ";
  for( int i=0; i<5; i++ )
    cout << 0.5 - 0.5*cos( 2.0*M_PI*i/(wSize-1) ); //(wSize-1 is wrong, but gives same results as scilab)
  cout << endl;

  for( int i=0; i<size; i++ )
    outData[i] = inData[i] * ( 0.5 - 0.5*cos( 2.0*M_PI*i/(wSize-1) ) );
  
}


Scilab::Scilab(){
    //open scilab
    scilab = popen( "scilab -nw", "w" );
    if( scilab == NULL ){
      cerr << "Unable to open gnuplot pipe!\n";
    }
}
Scilab::~Scilab(){
  flush();
  if( scilab ){
    fprintf( scilab, rdbuf()->str().c_str() );
    fclose( scilab );
  }
}

GnuStream::GnuStream(){

    //open gnuplot
    gnuplot = popen( "gnuplot -persist", "w" );
    if( gnuplot == NULL ){
      cerr << "Unable to open gnuplot pipe!\n";
    }
}

void GnuStream::outputPNG( string filename ){
  *this << "set terminal png\n";
  *this << "set output '" << filename << "'\n";
}

void GnuStream::outputX11(){
  *this << "set terminal x11\n";
}

void GnuStream::plot(){
  flush();
  if( gnuplot ){
    fprintf( gnuplot, rdbuf()->str().c_str() );
    fflush( gnuplot );
  }
  clear();
}

GnuStream::~GnuStream(){
  flush();
  if( gnuplot ){
    fprintf( gnuplot, rdbuf()->str().c_str() );
    pclose( gnuplot );
  }
}


//plots a single 1D data list
void Algorithms::plotDoubleData( string title, double *data, int size ){
  
  
  //open datafile
  ofstream dataFile(filename.c_str());
  
     
    for( int i=0; i<size; i++ ){
      dataFile << i << "\t" << data[i] << endl;
    }
    
    dataFile.close();
    
    //open gnuplot
    GnuStream gnu;
    
    gnu << "plot '" << filename << "' title '" << title << "' with lines\n";
    gnu.plot();
}


double Algorithms::centsDiff( double f1, double f2, bool ignoreOctave ){

  double x = log(f2/f1)/log(2.0);
  double cd;
  if( ignoreOctave ){
    double cd = 1200 * (x - (int)x);
    if( cd > 600 ) cd -= 1200;      //if > 1/2 octave above
    if( cd < -600) cd += 1200;      //if > 1/2 octave below
    return cd;
  }
  else
    return 1200 * x;



  return cd;
}


//plots a 2D data list - if no outfilename given then plots to screen
void Algorithms::plotDoubleData( string title, vector< pair<double,double> > *data, string outfilename ){
 
      //open datafile
      ofstream dataFile(filename.c_str());
    
      for( int i=0; i<data->size(); i++ ){
	dataFile << data->at(i).first << "\t" << data->at(i).second << endl;
      }
      
      dataFile.close();
      
      //open gnuplot
      FILE *gnuplot;
      gnuplot = popen( "gnuplot -persist", "w" );
      GnuStream gnu;
      
      if(outfilename.size() == 0){
	//gnu << "set terminal x11\n";
	gnu << "plot '" << filename << "' using 1:2 title '" << title << "'\n";   
      }
      else{
      gnu << "set terminal png\n";
      gnu << "set output \"" << outfilename << ".png\"\n";
      gnu << "set pointsize 4\n";
      gnu << "plot '" << filename << "' using 1:2 lw 2 title '" << title << "'\n";
      }
      
      fflush( gnuplot );
      pclose( gnuplot );

}






void Algorithms::getPitch( double freq, char* step, int *octave, char *acc, double refC ){

 int refOct = 4; //reference pitch is C4

  //calculate pitch from frequency
  //require tuning
  //calculate cents from A, then determine pitch

  // tuning system should be used to determine pitch
  // and decide whether two pitches are equal eg. A# Bb
  // for now just assume 12ET

  //cout << "PitchEl::PitchEl" << endl;

  double cents = centsDiff( refC, freq );
  //cout << "cents "<< cents << endl;

  int octaveDiff = (int)( cents/1200 );
  cents -= octaveDiff*1200;               // remove octaves from cents
  if( cents < 0 ) octaveDiff--;           // decrement octave by one if note is below
  int steps = (int) round( cents/100.0 );         // calculate no of steps
  if( steps < 0 ) steps += 12;

  //cout << "steps" << steps << endl;

  bool altered = false;
  if( steps == 1 |
      steps == 3 |
      steps == 6 |
      steps == 8 |
      steps == 10 ){

    steps--;
    altered=true;
  }
  else if( steps == 12 ) steps = 0; //if steps rounded up to next C, round back down

  switch (steps) {
  case 0:
    *step = 'C';
    break;
  case 2:
    *step = 'D';
    break;
  case 4:
    *step = 'E';
    break;
  case 5:
    *step = 'F';
    break;
  case 7:
    *step = 'G';
    break;
  case 9:
    *step = 'A';
    break;
  case 11:
    *step = 'B';
    break;
  default:{
    cerr << "Algorithms::getPitch Something really wrong here\n";
    cerr << "freq " << freq << endl;
    cerr << "steps " << steps << endl;
    cerr << "cents " << cents << endl;
    
  }
  }

  if( altered ) *acc = '#';

  *octave = refOct+octaveDiff;


}


int Algorithms::gcd( int a, int b ){
  int r;
  while(b!=0){
    r = a%b;
    a=b;
    b=r;
  }
  return abs(a);
}
int Algorithms::lcm( int a, int b ){
  return abs((a*b)/gcd(a,b));
}
