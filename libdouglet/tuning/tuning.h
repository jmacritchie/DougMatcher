
#ifndef TUNING_H_
#define TUNING_H_


#include <vector>
#include <string>
#include <map>
#include <utility>
#include "pitch.h"

class Pitch;

/**
 * Defines the relative order and equivalencies of pitch spellings
 * Eg. in 12ET <C, 0> <C#, 1> <Bb, 1>
 */
typedef std::pair< int, PitchSpelling > SpellingListItem;
typedef std::multimap< int, PitchSpelling > SpellingList;
typedef std::multimap< int, PitchSpelling >::iterator SpellingListIterator;


/**
 * Lists the intervals in the scale, in order, as cents from the root note.
 * Eg. 12ET would have 13 values where [0]=0, [1]=100, [12]=1200.
 * This allows for scales not based on a perfect octave (any examples?)
 */
typedef std::vector< double > IntervalList;
typedef std::vector< double >::iterator IntervalListIterator;


/**
 * Maps midi numbers to enharmonically equivalent pitches
 */
typedef std::multimap<int, Pitch> PitchMap;
typedef std::pair<int, Pitch> PitchMapItem;
typedef std::multimap<int, Pitch>::iterator PitchMapIterator;
typedef std::multimap<int, Pitch>::const_iterator ConstPitchMapIterator;

/**
 * Maps midi numbers to frequency
 */
typedef std::map<int, double> FreqMap;
typedef std::pair<int, double> FreqMapItem;

/**
 * Defines the tuning system used, and allows conversion
 * between pitch, frequency, and midi notes. Should be extended
 * to accomodate keys for improved automatic pitch spelling.
 */
class Tuning{

 public: 

  /**
   * Construct a tuning with specified name, and interval & 
   * spelling lists. 
   */
  Tuning(std::string name, IntervalList *intervals, 
	 SpellingList *spellings);

  /**
   * Copy the given tuning.
   */
  Tuning( const Tuning *tuning );

  /**
   * Get list of possible tunings from tuning file
   */
  static std::vector<Tuning*>* getTunings( std::string filename );

  /**
   * Set the reference pitch and frequency
   */
  void setRefNote( Pitch, double freq );

  /**
   * Set the root pitch (the pitch from which intervals will be calculated)
   * This is has no effect on equal tempered scales.
   */ 
  void setRootPitch( PitchSpelling root );



  /*********************************
   *        Conversion Methods     *
   *********************************/

  /**
   * Calculates frequency of midi note.
   */
  double getFrequency( MidiNo midiNo );

  /**
   * Get frequency of pitch 
   */
  double getFrequency( Pitch pitch );


  /**
   * Calculates pitch from frequency. If possible, it
   * will return a pitch spelling with no accidental.
   * Failing that it will attempt to return a spelling
   * with a sharp or flat (depending on @preferSharps).
   * Failing that it will return the first possible spelling.
   */
  Pitch getPitch( double frequency, bool preferSharps=true );

  /**
   * Calculates pitch based on midi number as in 'getPitch'
   */
  Pitch getPitch( MidiNo midiNo, bool preferSharps=true );

  /**
   * Returns midi note of pitch
   */
  MidiNo getMidiNumber( Pitch p );

  /**
   * Given frequency, returns the nearest midi note.
   * If diff is given, the method will set diff to be
   * the difference between the supplied frequency and 
   * the exact frequency of the midi note in cents.
   */
  MidiNo getMidiNumber( double freq, double *diff=NULL );



  /**
   * Returns the number of steps between the
   * two pitches in the current tuning.
   */
  int getInterval( Pitch p1, Pitch p2 ) const;




  const std::string getName() const;

  SpellingList *getSpellingList() const;
  IntervalList *getIntervalList() const;

  PitchSpelling getRootPitch() const;

  Pitch getRefPitch() const;
  double getRefFreq() const;

  Pitch getMidiRefPitch() const;
  MidiNo getMidiRef() const;
  void setMidiRef( Pitch p, MidiNo m );

  /// Prints a summary of the tuning to std out
  void printTuning() const;
  void printMaps(); //const (but can't declare this)


 protected:

  std::string m_name;
  /// Root pitch
  PitchSpelling m_rootPitch;
  /// Root pitch
  int m_rootPosition;
  /// Reference pitch
  Pitch m_refPitch;
  /// Position of C in spelling list
  int m_cPosition;
  /// Reference pitch frequency
  double  m_refFreq;
  /// Number of intervals
  int m_size;
  /// Interval of 'octave'
  double octaveCents;
  /// Frequency of C in reference octave. Useful, recalculated each change in tuning.
  double m_cRefFreq;

  /// The midi reference pitch will correspond to the midi reference number. All midi numbers will be generated from this pair.
  Pitch m_midiRefPitch;
  /// See m_midiRefPitch.
  MidiNo m_midiRefNo;


  SpellingList *m_spellings;
  IntervalList *m_intervals;

  /// Maps midi numbers to frequency
  FreqMap m_freqMap;
  /// Maps midi numbers to enharmonically equivalent pitches
  PitchMap m_pitchMap;



  /// Contains tunings
  static std::vector<Tuning*> m_tunings;


  /// Searches in spelling map for pitch spelling 
  SpellingListIterator findPitchSpelling( PitchSpelling ps ) const;


  /// Called whenever tuning is changed.
  void createTuningMaps();

  /**
   * 
   */
  int checkNotatedOctave( PitchSpelling ps ) const;

  /**
   * Calculates interval in cents from C. 
   * Position is the position as found in the spelling map
   * This takes into account the root note and
   * is essential for non-ET tunings
   */
  double intervalFromC( int position ) const;


};







#endif
