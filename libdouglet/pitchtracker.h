/***************************************************************************
                               pitchtracker.h
                             -------------------
    email                : D.Mcgilvray@elec.gla.ac.uk
    website              : http://cmt.gla.ac.uk
    copyright            : (C) 2004 by D.McGilvray & CMT

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PITCHTRACKER_H
#define PITCHTRACKER_H


#include <math.h>
#include <complex>
#include <fftw3.h>
#include <fstream>
#include <string>
#include <vector>




class PitchTracker{

 public:

  typedef std::string Method;
 
  static const Method PARTIAL;
  static const Method AUTOCORRELATION;
  static const Method HPS;
  static const Method SQDIFF;
  static const Method CEPSTRUM;
  
  PitchTracker( int frameSize, int stepSize, int sampleRate );
  ~PitchTracker();

  double amdf();
  double autocorrelation();
  double hps();
  double sqdiff();
  double unwrapPhase( int fBin );

  double *getInBuffer();
  double getPitch();

  int getFrameSize();
  void setFrameSize( int frameSize );
  int getStepSize();
  void setStepSize( int stepSize );

  void setMethod( Method Method );
  Method getCurrentMethod(){ return method; }
  std::vector<Method> *getMethods();

 private:
 
  double *frame;
  std::vector<Method> *methods;
  
  double *cepstralIn, *in1, *in2;
  int frameSize;
  int stepSize;
  int sampleRate;

  Method method;
  fftw_complex *ft1, *ft2, *cepstralOut;
  fftw_plan p1, p2, pc;

};

#endif
