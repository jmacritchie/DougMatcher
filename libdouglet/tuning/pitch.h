

#ifndef PITCH_H_
#define PITCH_H_


#include <string>
#include <iostream>
#include <sstream>
#include <utility>



class Tuning;

typedef unsigned int MidiNo;

typedef std::string Accidental;


namespace Accidentals{

  extern const  Accidental NoAccidental;
  extern const  Accidental DoubleSharp;
  extern const  Accidental ThreeQuarterSharp;
  extern const  Accidental Sharp;
  extern const  Accidental QuarterSharp;
  extern const  Accidental Natural;
  extern const  Accidental QuarterFlat;
  extern const  Accidental Flat;
  extern const  Accidental ThreeQuarterFlat;
  extern const  Accidental DoubleFlat;
};



/// PitchSpelling defines a pitch without octave.
typedef std::pair< char, Accidental > PitchSpelling;




class Pitch{

  char m_note;
  Accidental m_acc;
  int m_oct;


 public:

  Pitch( char name, int oct, Accidental acc = Accidentals::NoAccidental );
  Pitch( PitchSpelling spelling, int oct );

  Pitch& operator= ( const Pitch &p );
  bool operator==( const Pitch &orig );
  bool operator!=( const Pitch &orig );
  char getNoteName() const;
  int getOctave() const ;
  Accidental getAccidental() const;
  PitchSpelling getSpelling() const;
  std::string getAsString() const;

};


std::ostream& operator<<(std::ostream& os, PitchSpelling& ps); 
std::ostream& operator<<(std::ostream& os, Pitch& ); 


#endif
