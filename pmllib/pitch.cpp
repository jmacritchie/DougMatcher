
/***************************************************************************
                                 tuning.cpp
                             -------------------
    email                : D.Mcgilvray@elec.gla.ac.uk
    website              : http://cmt.gla.ac.uk
    copyright            : (C) 2004 D.McGilvray

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pitch.h"
#include "tuning.h"

#include <string>
#include <utility>
#include <iostream>

using std::ostream;

namespace Accidentals
{
  const Accidental NoAccidental = "NoAccidental";
  const Accidental DoubleSharp = "DoubleSharp";
  const Accidental ThreeQuarterSharp = "ThreeQuarterSharp";
  const Accidental Sharp = "Sharp";
  const Accidental QuarterSharp = "QuarterSharp";
  const Accidental Natural = "Narural";
  const Accidental QuarterFlat = "QuarterFlat";
  const Accidental Flat = "Flat";
  const Accidental ThreeQuarterFlat = "ThreeQuarterFlat";
  const Accidental DoubleFlat = "DoubleFlat";
}



Pitch::Pitch( char note, int oct, Accidental acc ) :
  m_note( note ),
  m_acc( acc ),
  m_oct( oct ) {}

Pitch::Pitch( PitchSpelling spelling, int oct ) :
  m_note( spelling.first ),
  m_acc( spelling.second ),
  m_oct( oct ) {}

Pitch& Pitch::operator=( const Pitch &orig ){
  m_note = orig.getNoteName();
  m_acc = orig.getAccidental();
  m_oct = orig.getOctave();
  return *this;
}

bool Pitch::operator==( const Pitch &orig ){
  if( m_note == orig.getNoteName() &&
      m_acc == orig.getAccidental() && 
      m_oct == orig.getOctave() )
    return true;
  else
    return false;
}


bool Pitch::operator!=( const Pitch &orig ){
  if( m_note != orig.getNoteName() ) return true;
  if( m_acc != orig.getAccidental() ) return true;
  if( m_oct != orig.getOctave() ) return true;
  else return false;
}



char Pitch::getNoteName() const{ return m_note; }
int Pitch::getOctave() const{ return m_oct; }
Accidental Pitch::getAccidental() const{ return m_acc; }
PitchSpelling Pitch::getSpelling() const{ return PitchSpelling( m_note, m_acc ); }

std::string Pitch::getAsString() const{

  std::ostringstream s;

  s << m_note;
  if( m_acc != Accidentals::NoAccidental ) s << m_acc;
  s << m_oct;

  return s.str();

}

ostream& operator<<(ostream& os, PitchSpelling ps){
  os << ps.first;
  if( ps.second != Accidentals::NoAccidental ) os << ps.second;
  return os;
}

ostream& operator<<(ostream& os, Pitch p){
  os << p.getAsString();
  return os;
}
