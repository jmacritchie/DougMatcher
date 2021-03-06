/***************************************************************************
                                 tuning.cpp
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

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>
#include <iostream>
#include <math.h>
#include <string>
#include <map>
#include <utility>
#include "tuning.h"
#include "pitch.h"


#define TUNING_DEBUG 0

using std::pair;
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::multimap;
using std::vector;



typedef pair<int, string> accPair;
vector<Tuning*> Tuning::m_tunings;

#define OCT_MIN 1
#define OCT_MAX 9

/*********************************
 *        Constructors           *
 *********************************/


Tuning::Tuning(string name, IntervalList *intervals, SpellingList *spellings) :
  m_name(name),
  m_rootPitch( 'C', Accidentals::NoAccidental ),
  m_refPitch( 'A', 4 ),
  m_refFreq( 440 ),
  m_midiRefNo(69),
  m_midiRefPitch( 'A', 4 ),
  m_spellings(spellings),
  m_intervals(intervals)
{
  cout << "\nTuning::Tuning(string name, IntervalList *intervals, SpellingList *spellings)\n";
  m_size = intervals->size() - 1;
  createTuningMaps();
}


Tuning::Tuning( const Tuning *tuning ) :
  m_intervals( tuning->getIntervalList() ),
  m_spellings ( tuning->getSpellingList() ),
  m_rootPitch( tuning->getRootPitch() ),
  m_refPitch( tuning->getRefPitch() ),
  m_midiRefNo( tuning->getMidiRef() ),
  m_midiRefPitch( tuning->getMidiRefPitch() )
{

  cout << "Tuning::Tuning( const Tuning *tuning )\n";

  m_size = m_intervals->size()-1;
  m_name = tuning->getName();
  

  createTuningMaps();

}




/*********************************
 *        Conversion Methods     *
 *********************************/


double Tuning::getFrequency( MidiNo midiNo ){
  return m_freqMap[midiNo];
}

double Tuning::getFrequency( Pitch p ){
  return getFrequency( getMidiNumber(p) );
}


MidiNo Tuning::getMidiNumber( Pitch p ){

  PitchMapIterator pit = m_pitchMap.begin();

  while( pit->second != p  ){
    pit++;
    if( pit == m_pitchMap.end() ){
      cerr << "Pitch " << p << " not found in pitch map!\n";
      return 0;
    }
  }

  return pit->first;

  /* Eventually worth calculating the number directly, 
     will avoid lengthy iterations. something like below
     ( but below not working, need octave check )
  int pos = findPitchSpelling( p.getSpelling() )->first;
  int relpos = pos - m_cPosition;
  if( relpos < 0 ) relpos += m_size;

  int refpos = findPitchSpelling( m_midiRefPitch.getSpelling() )->first;
  int relrefpos = refpos - m_cPosition;
  if( relrefpos < 0 ) relrefpos += m_size;
  

  int diff = (relpos - relrefpos) + m_size*(p.getOctave() - m_midiRefPitch.getOctave());
  MidiNo m = m_midiRefNo + diff;

  return m;
  */
}

// Binary chop search 
MidiNo Tuning::getMidiNumber( double freq, double *diff ){
  if(TUNING_DEBUG) cout << "Tuning::getMidiNumber( double freq, double *diff )\n";
  int min=m_freqMap.begin()->first;
  int max=(--m_freqMap.end()) ->first;
  int mid;
  double cents=0;

  while( (max-min) != 1 ){
    if(TUNING_DEBUG)cout << "Min " << min << " Max " << max << endl;
    mid = (max+min)/2;
    cents = log(m_freqMap[mid]/freq)/log(2.0); 
    if(TUNING_DEBUG)cout << "mid " << mid  << " " << m_freqMap[mid] << " " << cents << endl;

    if( cents < 0 )
      min = mid;
    else
      max = mid;
    
  }

  // above algorithm returns midi number immediately above frequency
  // now check if previous midi number is closer
  double centsLower = log(m_freqMap[mid-1]/freq)/log(2.0);
  if( fabs( centsLower ) < fabs( cents ) )
    --mid;

  if( diff != NULL ){
    double d = 1200* log(m_freqMap[mid]/freq)/log(2.0);
    *diff = d;
    if(TUNING_DEBUG)cout << "diff " << d << endl;

  }

  return mid;
}

Pitch Tuning::getPitch( MidiNo midiNo, bool preferSharps ){

  pair<PitchMapIterator, PitchMapIterator> range;
  range = m_pitchMap.equal_range( midiNo );

  // first look for spelling with natural
  for( PitchMapIterator pit = range.first;
       pit != range.second; ++pit )
    if( pit->second.getAccidental() == Accidentals::NoAccidental )
      return pit->second;


  
  // then check for sharp/flat depending on preference
  Accidental preference = preferSharps ? Accidentals::Sharp : Accidentals::Flat;

  for( PitchMapIterator pit = range.first;
       pit != range.second; ++pit )
    if( pit->second.getAccidental() == preference  )
      return pit->second;

  // then just return first instance

  return (range.first)->second;

}

Pitch Tuning::getPitch( double frequency, bool preferSharps ){

  return getPitch( getMidiNumber(frequency), preferSharps );

}



/*********************************
 *        Printing Methods       *
 *********************************/



void Tuning::printMaps(){
  // Print out maps
  for( PitchMapIterator p = m_pitchMap.begin();  
       p != m_pitchMap.end(); ++p ){
    int midi = p->first;
    double freq = m_freqMap[midi];
    Pitch pi = p->second;
    cout << midi << "\t" << freq << "\t" << pi << endl;
  }
}



/**
 * Prints to std out for debugging
 */
void Tuning::printTuning() const{

  cout << "Print Tuning\n";
  cout << "Name: '" << m_name << "'\n";

  cout << "Ref note " << m_refPitch.getAsString()
	    << " " << m_refFreq <<endl;

  cout << "Ref freq for C " << m_cRefFreq << endl;

  cout << "Root note " <<  m_rootPitch.first << m_rootPitch.second
	    << endl;
       
  for( SpellingListIterator sit = m_spellings->begin();
       sit != m_spellings->end(); sit++ ){

    cout << "Spelling '" << sit->second
	      << "'\tinterval " << sit->first
	      << endl;
    
  }

  for( unsigned int i=0; i != m_intervals->size(); i++ ){ 
    cout << "Interval '" << i
	      << "'\tinterval " << m_intervals->at(i)
	      << endl;
    
  }
  cout << "Tuning repeats every " << m_intervals->at( m_size ) << " cents\n";


  cout << "Done." << endl;


}

/*********************************
 *        Utility Methods        *
 *********************************/



int Tuning::getInterval( Pitch p1, Pitch p2 ) const{


  // Calculate position within octave
  int step1 = findPitchSpelling( p1.getSpelling() )->first - m_cPosition;
  if( step1 < 0 ) step1 += m_size;


  int step2 = findPitchSpelling( p2.getSpelling() )->first - m_cPosition;
  if( step2 < 0 ) step2 += m_size;

  int stepDiff;

  int octDiff = p2.getOctave() - p1.getOctave();

  cout << "Step1 " << step1 << endl
       << "step2 " << step2 << endl
       << "oct" << octDiff << endl;

  if( octDiff >= 0 ){
    stepDiff = step2 - step1;
    return octDiff * m_size + stepDiff;
  }
  else{
    ++octDiff;
    stepDiff = step1 - step2;
    return octDiff * m_size + (-12-stepDiff);

  }

}


// Finds pitch spelling in spellings list
SpellingListIterator Tuning::findPitchSpelling( PitchSpelling ps ) const{

  SpellingListIterator sit;
  for( sit = m_spellings->begin();
       sit != m_spellings->end();
       sit++ ){
    
    if( sit->second == ps )
      return sit; 
  }
  cout << "findPitchSpelling(...) : pitch spelling not found in list!!!\n";
  return sit;
}







void Tuning::createTuningMaps(){

  m_cPosition = findPitchSpelling( PitchSpelling('C',Accidentals::NoAccidental))->first;

  int refPosition = findPitchSpelling( m_refPitch.getSpelling() )->first;

  double cents = -intervalFromC( refPosition );
  m_cRefFreq = m_refFreq * pow( 2, cents/1200.0 );

  //iterate from midi reference up to upper octave limit filling maps


  // find midi ref pitch in spellings map
  SpellingListIterator sit = findPitchSpelling( m_midiRefPitch.getSpelling() );


  int midiRefPos = findPitchSpelling( m_midiRefPitch.getSpelling() )->first;
  int cPos = findPitchSpelling( PitchSpelling( 'C', Accidentals::NoAccidental ) )->first;

  int cOffset = cPos - midiRefPos;
  if( cOffset > 0 ) cOffset -= 12;

  int refPitchPos = findPitchSpelling( m_refPitch.getSpelling() )->first; 
  double intFromC = -intervalFromC( refPitchPos ); // must be - the result
  double cRefFreq = m_refFreq * pow( 2.0, intFromC/1200.0 );

  if(TUNING_DEBUG)
  cout << "C position\t" << cPos << endl
    
       << "Midi ref " << m_midiRefPitch << endl
       << "\tposition " << midiRefPos << endl
    
       << "ref pitch " << m_refPitch << endl
       << "\tposition " << refPitchPos << endl
       << "\tintervalFromC " << intFromC << endl;
    

  // Iterating values
  int currentOctave = OCT_MIN;
  int midiNote = m_midiRefNo + cOffset - m_size*(m_midiRefPitch.getOctave()- OCT_MIN );  // contains place in spellings
  int spellingPosition = cPos;                                     // contains current midi no


  while( currentOctave <= OCT_MAX ){

    // Get all pitches for current midi note
    pair<SpellingListIterator, SpellingListIterator> range;
    range = m_spellings->equal_range( spellingPosition );

    // Iterate through each pitch
    for( SpellingListIterator sit = range.first;
	 sit != range.second; ++sit ){

      PitchSpelling ps = sit->second;
      int octaveChange = checkNotatedOctave(ps);
      int notatedOctave = currentOctave + octaveChange;
      PitchMapItem item( PitchMapItem(midiNote, Pitch(ps, notatedOctave)) );
      m_pitchMap.insert( item );
    }

    // calculate and insert frequency into freqmap
    intFromC = intervalFromC( spellingPosition );
    double cInOctave = cRefFreq * pow( 2.0, currentOctave - m_refPitch.getOctave() );
    double freq = cInOctave * pow( 2.0, intFromC/1200.0 );
    m_freqMap.insert( FreqMapItem( midiNote, freq ) );


    // Iterate values, wrap spelling position if needs be
    // increment octave on C
    ++midiNote;
    ++spellingPosition;
    if( spellingPosition == m_size ) spellingPosition = 0;
    if( spellingPosition == cPos ) ++currentOctave;

  }
}  



// Returns the interval between C and given interval
// taking root note into account
double Tuning::intervalFromC( int position ) const{

  int rootPosition = findPitchSpelling( m_rootPitch )->first;

  // Cents above root note (interval)
  int relativePosition = position - rootPosition;
  if( relativePosition < 0 ) relativePosition += m_size;
  double cents = (*m_intervals)[relativePosition];

  // cents above root note for reference note ( C )
  int refInterval = m_cPosition - rootPosition;
  if( refInterval < 0 ) refInterval += m_size;
  double refCents = (*m_intervals)[refInterval];

  // relative cents from C to interval
  double relativeCents = cents - refCents;
  if( relativeCents < 0 ) relativeCents += 1200;

  if(TUNING_DEBUG)
  cout << "Tuning::intervalFromC\n"
       << "\tposition in spelling list " << position << endl
       << "\tposition relative to root " << relativePosition << endl
       << "\tcents from root " << cents << endl
       << "\tposition of C relative to root " << refInterval << endl
       << "\troot to C interval " << refCents << endl
       << "\tcents " << relativeCents
       << endl;
 
  return relativeCents;

}




/**
 * This method will determine whether a PitchSpelling
 * should be notated in the sounding octave.
 * Eg. B# with the same frequency as C4 should
 * be notated as B#3. The return value should be
 * added to the sounding octave to obtain the
 * notated octave, thus B# will return -1, Cb will
 * return +1;
 */
int Tuning::checkNotatedOctave( PitchSpelling ps ) const{


  int pos = findPitchSpelling( ps )->first;
  int bPos = findPitchSpelling( PitchSpelling( 'B', Accidentals::NoAccidental ) )->first;
  int cPos = findPitchSpelling( PitchSpelling( 'C', Accidentals::NoAccidental ) )->first;


  /*
    If a spelling with any sharp inflection occurs after B,
    but the uninflected spelling occurs before C, the notated 
    octave should be one more than the sounding octave
    Eg. B#3 == C4
  */
  if( ps.second == Accidentals::QuarterSharp ||
      ps.second == Accidentals::Sharp ||
      ps.second == Accidentals::ThreeQuarterSharp ||
      ps.second == Accidentals::DoubleSharp ){
    
    
    // position of spelling relative to B
    int posFromB = pos - bPos;
    
    // position of uninflected spelling relative to C
    int naturalPos = findPitchSpelling( PitchSpelling( ps.first, Accidentals::NoAccidental ) )->first;
    int naturalFromC = naturalPos - cPos;
    
    // wrap positions to +-6
    if( posFromB > 6 ) posFromB -= 12;
    if( naturalFromC > 6 ) naturalFromC -= 12;
    
    
    if( posFromB > 0 && naturalFromC < 0 )
      return -1;  
    
  }
  /*
    Conversely for any flattened spellings,
    If the spelling is below C, but the uninflected spelling 
    occurs above B, the notated octave is one more than the 
    sounding octave.
    Eg. Cb4 = B3
  */
  else if( ps.second == Accidentals::QuarterFlat ||
	   ps.second == Accidentals::Flat ||
	   ps.second == Accidentals::ThreeQuarterFlat ||
	   ps.second == Accidentals::DoubleFlat ){
    
    // position of spelling relative to C
    int posFromC = pos - cPos;
    
    // position of uninflected spelling relative to B
    int naturalPos = findPitchSpelling( PitchSpelling( ps.first, Accidentals::NoAccidental ) )->first;
    int naturalFromB = naturalPos - bPos;
    
    // wrap positions to +-6
    if( posFromC > 6 ) posFromC -= 12;
    if( naturalFromB > 6 ) naturalFromB -= 12;
    
    if( posFromC < 0 && naturalFromB > 0 )
      return 1;
  }
  // Any natural or NoAccidentals should remain unchanged
  return 0;
  
}


/*********************************
 *        Get & Set Methods      *
 *********************************/

SpellingList *Tuning::getSpellingList() const{ return m_spellings; }
IntervalList *Tuning::getIntervalList() const{ return m_intervals; }

PitchSpelling Tuning::getRootPitch() const { return m_rootPitch; }
Pitch Tuning::getRefPitch() const { return m_refPitch; }
double Tuning::getRefFreq() const{ return m_refFreq; }
const string Tuning::getName() const { return m_name; }
Pitch Tuning::getMidiRefPitch() const { return m_midiRefPitch; }
MidiNo Tuning::getMidiRef() const{ return m_midiRefNo; }


void Tuning::setRootPitch( PitchSpelling pitch ){
  m_rootPitch = pitch;
  createTuningMaps();
}

void Tuning::setRefNote( Pitch pitch, double freq ){
  m_refPitch = pitch;
  m_refFreq = freq;
  createTuningMaps();
}

void Tuning:: setMidiRef( Pitch p, MidiNo m ){
  m_midiRefPitch = p;
  m_midiRefNo = m;
  createTuningMaps();

}

/*********************************
 *        Parse Tuning File      *
 *********************************/


vector<Tuning*> *Tuning::getTunings( string filename ){

  //if already have tunings, return them
  if( m_tunings.size() )
    return &m_tunings;


  //create map from accidental number to accidental string
  map<int, Accidental> accMap;
  accMap.insert( accPair( -4, Accidentals::DoubleFlat ));
  accMap.insert( accPair( -3, Accidentals::ThreeQuarterFlat ));
  accMap.insert( accPair( -2, Accidentals::Flat ));
  accMap.insert( accPair( -1, Accidentals::QuarterFlat ));
  accMap.insert( accPair( 0,  Accidentals::NoAccidental ));
  accMap.insert( accPair( 1,  Accidentals::QuarterSharp ));
  accMap.insert( accPair( 2,  Accidentals::Sharp ));
  accMap.insert( accPair( 3,  Accidentals::ThreeQuarterSharp ));
  accMap.insert( accPair( 4,  Accidentals::DoubleSharp ));
    
  QFile fin( filename );
  
  IntervalList *intervals = new IntervalList;
  intervals->push_back(0);// [0] interval will always be 0, last interval will define octave 
  SpellingList *spellings = new SpellingList;

  if( fin.open( IO_ReadOnly ) ){
    

    bool inTuningDefinition = false;
    int lineNumber = 0;
    QString line = QString::null;
    QTextStream stream( &fin );
    QString tuningName;
      
      while( !stream.atEnd() ){
	line = stream.readLine();
	lineNumber++;
	line = line.stripWhiteSpace();
	
	// ignore comments or blanks
	if( line.at(0) == '#' || line.at(0) == '!')continue;
	else if( line.isEmpty() )continue;
	
	// Tuning declaration
	else if( line.startsWith( QString("Tuning"), false ) ){ 
	  
	  if( inTuningDefinition ){
	    if( TUNING_DEBUG ) cout << "End of tuning " << tuningName << endl;
	    string name = tuningName.ascii();
	    Tuning *newTuning = new Tuning( name, intervals, spellings );
	    m_tunings.push_back( newTuning );
	    intervals = new IntervalList;
	    intervals->push_back(0);      // [0] interval will always be 0, last interval will define octave 
	    spellings = new SpellingList;


	  }

	  // Remove tuning keyword and get name from remainder of line
	  line.remove( 0, 6 );
	  line = line.stripWhiteSpace();


	  tuningName = line;
	  inTuningDefinition = true;

	  if(TUNING_DEBUG)
	    cout << "\nNew Tuning\t '" << tuningName << "'" << endl;


	}
	
	
	// Must be a note line
	else{
	  
	  bool ok;
	  int commaPos;
	  
	  if(TUNING_DEBUG)
	    cout << "note" << endl
		 << "line no." << lineNumber << endl;
	  
	  // is cents or ratio

	  // get cents

	  double cents;
	  commaPos = line.find( QChar(',') );
	  QString intervalString = line;
	  intervalString.remove( commaPos, intervalString.length() - commaPos );
	  intervalString = intervalString.stripWhiteSpace();

	  int dotPos = intervalString.find( QChar('.') );

	  
	  if( dotPos == -1 ){ //interval is a ratio	    
	    
	    if(TUNING_DEBUG)cout << "Interval is a ratio" << endl;

	    int slashPos = intervalString.find( QChar('/') );
	    double ratio=1;
	    if( slashPos == -1 ){ //interval is integer ratio
	     if(TUNING_DEBUG) cout << "Ratio is an integer" << endl;

	      ratio = intervalString.toInt( &ok );
	      if( !ok ){
		cerr << "Syntax Error in tunings file, line " << lineNumber << endl;
		continue;
	      }
	      else{

		//convert ratio to cents	      
		QString numeratorString = intervalString;
		numeratorString.remove( slashPos, numeratorString.length() - slashPos );
		 if(TUNING_DEBUG) cout << "numerator" << numeratorString << endl;
		int numerator = numeratorString.toInt( &ok );
		if( !ok ){
		  cerr << "Syntax Error in tunings file, line " << lineNumber << endl;
		  continue;
		}
      
	      
		QString denominatorString = intervalString;
		denominatorString.remove( 0, slashPos+1 );
		 if(TUNING_DEBUG) cout << "d " << denominatorString << endl;
		int denominator = denominatorString.toInt( &ok );
		if( !ok ){
		  cerr << "Syntax Error in tunings file, line " << lineNumber << endl;
		  continue;
		}	   

		if(TUNING_DEBUG)
		  cout << "Ratio: " << numeratorString
			    << "/" << denominatorString << endl;
		
		ratio = (double)numerator / (double)denominator;
	      

		//calculate cents	    
		cents = 1200.0 * log( ratio )/log(2.0);
		if(TUNING_DEBUG)
		  cout << "cents " << cents << endl;
	      }
	    }
	  }
	  else{ // interval is in cents
	    if(TUNING_DEBUG) cout << "Interval is in cents" << endl;
	    cents = intervalString.toDouble( &ok );
	    if( !ok ){
	      cerr << "Syntax Error in tunings file, line " << lineNumber << endl;
	      continue;
	    }
	     if(TUNING_DEBUG) cout << "Cents: " << cents << endl;
	    
	  }

	  //add intervals to interval list
	  intervals->push_back( cents );

	  //remove interval
	  line.remove( 0, commaPos+1 );
	  
	  
	  // get various spellings
	  while( !line.isEmpty() ){
	    line = line.stripWhiteSpace();
	    
	    commaPos = line.find( QChar(',') );
	    if( commaPos == -1 ){	    
	      if(TUNING_DEBUG)
		cout << "var '" << line << "'" << endl;
	      QString note = line;
	      QString acc = note;
	      acc.remove( 0, 1 ); 
	      note.remove( 1, note.length() -1 ); 
	      if(TUNING_DEBUG) cout << "Acc " << acc << endl;
	      if(TUNING_DEBUG)  cout << "Pitch Class " << note << endl;
	      
	      PitchSpelling p( note.at(0).latin1(), accMap.find(atoi(acc.latin1()))->second );
	      //insert into spelling list
	      spellings->insert( SpellingListItem( intervals->size()-2, p ) );

	      line = QString::null;
	      if(TUNING_DEBUG){
		cout << "Translated variation '" << p << "'" << endl;
		cout << endl;
	      }
	    }
	    else{
	      QString note = line;
	      note.remove( commaPos, note.length() - commaPos );
	      QString acc = note;
	      acc.remove( 0, 1 ); 
	      note.remove( 1, note.length() -1 ); 
	      if(TUNING_DEBUG) cout << "Acc " << acc << endl;
	      if(TUNING_DEBUG) cout << "Pitch Class " << note << endl;
	      PitchSpelling p( note.at(0).latin1(), accMap.find(atoi(acc.latin1()))->second );
	      line.remove( 0, commaPos+1 );

	      //insert into spelling list
	      spellings->insert( SpellingListItem( intervals->size()-2, p ) );

	      if(TUNING_DEBUG)
		cout << "Translated variation '" << p << "'" << endl;
	    }
	    
	    
	  }
	  if(TUNING_DEBUG)
	    cout << endl;	
	}
	
      }
      fin.close();
      if( inTuningDefinition ){
	if( TUNING_DEBUG ) cout << "End of tuning " << tuningName << endl;	
	m_tunings.push_back( new Tuning( tuningName.ascii(), intervals, spellings ) );
      }
      

      return &m_tunings;
  }
  
  cerr << "Cannot find tunings file\n";
  return NULL;   
  
}
