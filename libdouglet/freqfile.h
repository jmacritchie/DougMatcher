#ifndef FFT_FILE_H_
#define FFT_FILE_H_

#include <vector>
#include <list>
#include <stdlib.h>
 
class SoundFile;
class PitchTracker;


/*
  Later change code to implement SDIF...
  CNMAT code currently too alpha-ish
 */

typedef std::pair<unsigned long, double> FreqEst;
//typedef std::list<FreqEst>::iterator FreqListIt;

typedef std::vector<FreqEst>::iterator FreqIt;

class FreqFile{

 public:
  FreqFile( SoundFile *sf, PitchTracker *pt, int stepSize  );
  FreqFile( const char *fftFile );
  ~FreqFile();

  //get start and end iterators that 
  FreqIt const start();
  FreqIt const end();

  FreqEst operator[]( const int);
  std::vector<FreqEst>* getFreqs(){return &freqs_;}

  double sampleRate(){ return sampleRate_; }
  double frameSize(){ return frameSize_; }
  double frameRate(){ return stepSize_; }

 private:

  std::vector<FreqEst> freqs_;  

  size_t sampleRate_;
  size_t frameSize_;
  size_t stepSize_;

};

#endif
