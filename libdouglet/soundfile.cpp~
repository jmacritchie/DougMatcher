
/**
 * Sound file C++ wrapper. Version 0.1, D.McGilvray http://cmt/gla/ac/uk
 * Requires libsndfile. Released under the GNU General Public License.
 */



#include "error.h"
#include "soundfile.h"
#include "algorithms.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

SoundFile::SoundFile():
  m_data(0) {
}

SoundFile::SoundFile( std::string name ):
  m_data(0) {
  m_filename = name;
  openFile( m_filename );
}


SoundFile::~SoundFile(){
  delete m_data;
}

int SoundFile::sampleRate(){ return m_sampleRate; }
int SoundFile::channels(){ return m_channels; }
int SoundFile::length(){ return m_samples; }
double *SoundFile::data(){ return m_data; }


bool SoundFile::openFile(){
  // should remove this sometime
  cout << "Enter filename:\n";
  string filename;
  cin >> filename;
  return openFile( filename );
}



bool SoundFile::openFile( std::string filename ){
  

  m_filename = filename;

  SF_INFO sfInfo;
  m_sf = sf_open(m_filename.c_str(), SFM_READ, &sfInfo) ;
  
  if( sf_error(m_sf) ){
    cout << "error opening file\n";
  }

  m_sampleRate = sfInfo.samplerate;
  m_channels = sfInfo.channels;
  m_samples = sfInfo.frames;

  cout << "sample rate\t= " << m_sampleRate << endl
       << "channels   \t= " << m_channels << endl
       << "samples     \t= " << m_samples << endl;
  
  if( m_channels > 1 )
    cout << "Only first channel will be analysed!\n";

  cout << "get data\n";
  getData(); //extract audio data
  cout << "got it.\n";
  return true;

}


void SoundFile::getData(){
  
  if(m_data) // free existing data
    free(m_data);
  
  m_data = (double*)malloc( sizeof(double)*m_samples );
  double frame[m_channels];
  for( int i=0; i<m_samples; i++ ){
    if( !sf_readf_double( m_sf, frame, 1 ) ) cerr << "Premature EOF!\n";
    m_data[i] = frame[0];
    if( m_data[i] > 1 || m_data[i] < -1 ){
      cout << i << " over thing\n";
    }

  }
  sf_close( m_sf );
  //plot();

}



void SoundFile::plot(){
  plot( 0, m_samples ); 
}

void SoundFile::plot( long start, long end ){
  
  if( !m_data ) cerr << "cannot open file!";
  else{
    
    //open datafile
    ofstream dataFile;
    dataFile.open("temp.dat");
    
    long length = end - start;

    for( double i=start; i<end; i++ ){
      long currentSample = i;
      dataFile << currentSample << "\t" 
	       << m_data[currentSample] << endl;
    }
    
    dataFile.close();
    
    //open gnuplot
    GnuStream gnu;
    
    gnu << "set xlabel 'Sample (Sample rate=" << m_sampleRate << "Hz)'\n"
	<< "plot 'temp.dat' using 1:2 title 'Waveform' with boxes\n";
    
  }
  
}


long SoundFile::sampleFromTime( double time ){
  return m_sampleRate * time;
}

double SoundFile::timeFromSample( long sample ){
  return double(sample) / m_sampleRate;
}

