
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "pmlelements.h"
#include "pmldocument.h"
#include "algorithms.h"
#include "pitch.h"
#include "xmlsc.h"


#include <string>
#include <list>
#include <sstream>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace std;
using namespace Algorithms;

#define XS XMLString::transcode


#define ALIGN_TAG "note"


const char* SNote::Onset = "starttime";

const char* PerfPart::Tag = "perfpart";

const char* PNote::Tag = "event";
const char* PNote::TagMidi = "midi";
const char* PNote::TagOnset = "onset";
const char* PNote::TagOffset = "end";
const char* PNote::TagFreq = "freq";

const char* PitchEl::TagNote = "step";
const char* PitchEl::TagAcc = "alter";
const char* PitchEl::TagOct = "octave";

const char* TuningEl::TagIntervalList = "interval_list";
const char* TuningEl::TagInterval = "interval";
const char* TuningEl::TagCents = "cents";
const char* TuningEl::TagRatio = "ratio";
const char* TuningEl::TagSpelling = "spelling";
const char* TuningEl::TagMidiNo = "midi_no_ref";
const char* TuningEl::TagMidiPitch = "midi_pitch_ref";
const char* TuningEl::TagRefPitch = "ref_pitch";
const char* TuningEl::TagRefFreq = "ref_freq";
const char* TuningEl::TagRootPitch = "root_pitch";


DOMElement *PMLBase::getElement(){ return m_element; }


ScorePart::ScorePart( PMLDocument *doc, DOMElement *element ){
  m_element = element; 
  m_doc = doc;
  // Find <score-part> element in part-list and store. This contains
  // Part metadata
  m_definition = m_element->getOwnerDocument()->getElementById( XS(getID().c_str()) );

  // fill vector of notes
  DOMNodeList *measures = m_element->getElementsByTagName(XS("measure"));

  for( unsigned int m=0; m<measures->getLength(); m++ ){

    DOMElement *measure = (DOMElement*)measures->item(m);

    // Iterate through MEASURE contents
    for( DOMNode *measureIt = measure->getFirstChild();
	 measureIt != NULL; 
	 measureIt = measureIt->getNextSibling() ){
      
      if( measureIt->getNodeType() == DOMNode::ELEMENT_NODE ){
	DOMElement *e = (DOMElement*)measureIt;
	
	string tag = XS( e->getTagName() );
	if( tag == "note" ){
	  if( e->getElementsByTagName( XS("rest") )->getLength() > 0 ){
	    //cout << "\trest\n"; //rest not in notes vector;
	  }
	  else{
	    //cout << "Note\n";
	    m_notes.push_back( new SNote(m_doc, e) );
	  }
	}
      }
      
      
    }


  }
  m_notes.sort( SNote::sortByTime );
}

string ScorePart::getName(){
  DOMElement *nameEl = getSingleElement( m_definition, "part-name" );
  return getText( nameEl );
}

int ScorePart::getNoNotes(){ return m_notes.size(); }

/**
 * Not all PML elements will have ID attributes, but
 * implemented in base for ease.
 */
string PMLBase::getID(){ 
  string id = XS( m_element->getAttribute( XS("id") ));
  return id;
}

/**
 * Removes DOM element from document
 */
void PMLBase::removeElement(){ 
  m_element->getParentNode()->removeChild( m_element );
}


/**
 * Returns element containing the performance information
 * for this part. Or NULL if no element exists.
 */
PerfPart *ScorePart::getPerfPart(){

  DOMDocument *doc = m_element->getOwnerDocument();
  DOMElement *root = doc->getDocumentElement();
  DOMElement *perf = (DOMElement*)root->getElementsByTagName(XS("performance"))->item(0);

  DOMNodeList *perfParts = perf->getElementsByTagName(XS( PerfPart::Tag) );

  for( unsigned int i=0; i<perfParts->getLength(); i++ ){
    DOMElement *e = (DOMElement*)perfParts->item(i);
    if( getID() == XS( e->getAttribute(XS("part")) ) ){
      return new PerfPart(m_doc, e);
    }
  }
  cout << "No performance part found\n";
  return new PerfPart(m_doc, this);
}


SNote *ScorePart::getNote( int index ){ 
  ScoreIterator it = m_notes.begin();
  for( int i=0; i<index; i++ )
    ++it;
  return *it; ; 
 
}

SNote *ScorePart::getNote( string id ){ 
  DOMDocument *doc = m_element->getOwnerDocument();
  DOMElement *e = doc->getElementById( XS(id.c_str()) );
  if( e ) return new SNote( m_doc, e );
  else return NULL;
}


list<SNote*> *ScorePart::getNotes(){ return &m_notes; }

/// Warning! Only looks for 1 divisions element in the first measure
int ScorePart::getDivisions(){

  DOMElement *m = getSingleElement(m_element, "measure");
  DOMElement *div = getSingleElement(m, "divisions");
  return atoi( getText( div ).c_str() );
}


PerfPart::PerfPart( PMLDocument *doc, ScorePart *part ){
  m_doc = doc;

  DOMDocument *domdoc = part->getElement()->getOwnerDocument();
  m_scorePart = part;
  m_element = domdoc->createElement( XS("perfpart") );

  //create part attribute
  m_element->setAttribute( XS("part"), XS( m_scorePart->getID().c_str() ) );
  
  //append part to performance element
  DOMElement *perf = (DOMElement*)domdoc->getElementsByTagName(XS("performance"))->item(0);
  perf->appendChild( m_element );

  DOMNodeList *nl = m_element->getElementsByTagName(XS(PNote::Tag));

  for( int i=0; i<nl->getLength(); i++ ){
    m_notes.push_back( new PNote( m_doc, (DOMElement*)nl->item(i) ) );

  }

}

int SNote::getVoice(){

  DOMElement *voice = getSingleElement(m_element,"voice");

  if( voice == NULL ) 
    return -1;
  else

  return atoi( getText(voice).c_str() );

}

int SNote::getStaff(){

  DOMElement *staff = getSingleElement(m_element,"staff");

  if( staff == NULL ) 
    return -1;
  else

  return atoi( getText(staff).c_str() );

}


/**
 * Create PerfPart element
 */
PerfPart::PerfPart( PMLDocument *doc, DOMElement *element ){
  m_doc = doc;
  m_element = element;

  DOMDocument *domdoc = m_element->getOwnerDocument();

  const XMLCh *partID = m_element->getAttribute( XS("part") );

  DOMElement *root = m_element->getOwnerDocument()->getDocumentElement();
  DOMNodeList *parts = root->getElementsByTagName(XS("part"));

  for( unsigned int i=0; i<parts->getLength(); i++ ){

    DOMElement *e = (DOMElement*)parts->item(i);

    cout << " perf  " << XS( m_element->getAttribute(XS("id")))
	 << " score " << XS( e->getAttribute(XS("part")) )
	 << endl;

    if( !strcmp(XS( m_element->getAttribute(XS("id")) ),  XS( e->getAttribute(XS("part")) ))   ){
      m_scorePart = new ScorePart( m_doc, e );
    }
  }
  
  DOMNodeList *nl = m_element->getElementsByTagName(XS(PNote::Tag));
  for( int i=0; i<nl->getLength(); i++ ){
    m_notes.push_back( new PNote( m_doc, (DOMElement*)nl->item(i) ) );
    
  }
  
}

ScorePart *PerfPart::getScorePart(){ return m_scorePart; }

PNote* PerfPart::createNote( double start, double end, double avgFreq ){

  stringstream ss;
  string text;

  PNote *note = new PNote( m_doc, start, end, avgFreq );

  //add note in order
  addNote( note );
  return note;


}


int PerfPart::noEvents(){ 
  //return m_element->getElementsByTagName(XS(PNote::Tag))->getLength(); 
  return m_notes.size();
}

PNote* PerfPart::getNote( int i ){
  //if( 0 > index || index >= m_notes.size() ) return NULL;

  PerfIterator it = m_notes.begin();
  while( i-- )
    it++;

  return (*it);

  //DOMNodeList* events = m_element->getElementsByTagName(XS(PNote::Tag));
  //return (DOMElement*)events->item(i);
}

#define ADDNOTEDEBUG 0
void PerfPart::addNote( PNote *note ){

  if( ADDNOTEDEBUG ) 
    cout << "Add note\n"		       
	 << "Onset " << note->getOnset() << endl;

  DOMNodeList *notes = m_element->getElementsByTagName( XS(PNote::Tag) );

  if( notes->getLength() == 0 ){
    if( ADDNOTEDEBUG ) cout << "Appending only note\n";
    m_element->appendChild( note->getElement() );
    return;
  }
  
  // search for insertion point from end (most likely position)
  PNote refNote( m_doc, (DOMElement*)notes->item(notes->getLength()-1 ) );
  if( ADDNOTEDEBUG ) cout << "Last note at " << refNote.getOnset() << endl;
  if( refNote.getOnset() < note->getOnset() ){
    if( ADDNOTEDEBUG ) cout << "Appending note to end\n";
    m_element->appendChild( note->getElement() );
    return;
  }
  else{
    if( ADDNOTEDEBUG ) cout << "Searching from end\n";
    for( int i=notes->getLength()-1; i>=0; i-- ){
      //cout << "i " << i << endl;
      PNote compNote( m_doc, (DOMElement*)notes->item(i) );
      //cout << compNote.getOnset() << endl;
      if( compNote.getOnset() < note->getOnset() ){
	// insert before the next note
	if( ADDNOTEDEBUG ) cout << "Inserting note after note " << i 
				<< "at " << compNote.getOnset() << endl;
	m_element->insertBefore( note->getElement(), 
				 notes->item(i+1) );
	return;
      }
    }
  }
  if( ADDNOTEDEBUG ) cout << "Inserting note before first note\n";
  m_element->insertBefore( note->getElement(), 
			   notes->item(0) );
  
  
  
}


PerfIterator PerfPart::removeNote( PerfIterator &it ){
  (*it)->removeElement();   // remove element from document
  delete( *it );            // delete PNote
  return m_notes.erase( it ); // erase PNote from note list

}

PerfIterator PerfPart::removeNote( int index ){

  if( 0 > index || index >= m_notes.size() ){
    cout << "PerfParrt::removeNote INDEX! " << index << endl;
    return m_notes.end();
  }

  PerfIterator it = m_notes.begin();
  while( index-- )
    it++;

  //cout << "PerfParrt::removeNote removing " << (*it)->getID() << endl;

  return removeNote( it ); // erase PNote from note list

}


/**
 * Will remove all align elements from notes
 */
void PerfPart::unAlign(){

  for( PerfIterator pit = m_notes.begin();
       pit != m_notes.end(); pit++ ){
    (*pit)->unalign();
  }

}


/* PNote */


int PNote::noteNo = 0;

PNote::PNote( PMLDocument *doc, double onset, double offset ){
  init( doc, onset, offset );
}
PNote::PNote( PMLDocument *doc, double onset, double offset, MidiNo midi ){
  init( doc, onset, offset );
  setMidi( midi );
}

PNote::PNote( PMLDocument *doc, double onset, double offset, double freq ){
  init( doc, onset, offset );
  setFreq( freq );
}
//initialise the pnote - creating an 'Event' for it and adding other info i.e onset/offset/id
void PNote::init( PMLDocument *doc, double onset, double offset ){
  m_doc = doc;
  m_element = doc->getDocument()->createElement( XS(PNote::Tag) );
  setOnset( onset );
  setOffset( offset );

  noteNo++;

  string id = "pnote" + Algorithms::itoa( noteNo );
  
  //ensure this id is not used already
  while( doc->getDocument()->getElementById( XS(id.c_str()) ) != NULL ){
    noteNo++;
    id = "pnote" + Algorithms::itoa( noteNo );
  }
  
  m_element->setAttribute( XS("id"), XS(id.c_str()) );

}


PNote::PNote( PMLDocument *doc, DOMElement *element ){
  m_element = element;
  m_doc = doc;
}

void PNote::unalign(){
  DOMNodeList *nlist = m_element->getElementsByTagName(XS("align"));

  if( nlist->getLength() > 0 )
    m_element->removeChild( nlist->item(0) );
}

void PNote::align( SNote *note, bool correct ){

  DOMNodeList *list = m_element->getElementsByTagName(XS("align"));
  DOMElement *align;

  if( list->getLength() == 0 ){
    DOMDocument *doc = m_element->getOwnerDocument();
    align = doc->createElement(XS("align"));
    m_element->appendChild(align);
  }
  else{
    align = (DOMElement*)list->item(0);
  }

  align->setAttribute( XS(ALIGN_TAG), XS(note->getID().c_str()) );
		       
  if( correct )
    setText( align, "correct" );
  else
    setText( align, "wrong" );

}


double PNote::getOnset(){
  DOMElement *onset = (DOMElement*)m_element->getElementsByTagName( XS(PNote::TagOnset) )->item(0);
  string text = getText( onset );
  return atof( text.c_str() );
}

double PNote::getOffset(){
  DOMElement *offset = (DOMElement*)m_element->getElementsByTagName( XS(PNote::TagOffset) )->item(0);
  string text = getText( offset );
  return atof( text.c_str() );
}

double PNote::getFreq(){
  DOMElement *freq = (DOMElement*)m_element->getElementsByTagName( XS(PNote::TagFreq) )->item(0);
  string text = getText( freq );
  return atof( text.c_str() );
}

MidiNo PNote::getMidi(){
  if( hasMidi() ){
    DOMElement *midi = (DOMElement*)m_element->getElementsByTagName( XS(PNote::TagMidi) )->item(0);
    string text = getText( midi );
    return atoi( text.c_str() );
  }
  else{
    //cerr << "PNote::getMidi element " << getID() << " does not have a midi number!\n";
    return -1;
  }
}

void PNote::setOffset( double offset ){
  setSubElementText( m_element, XS(PNote::TagOffset), ftoa(offset) );
}

void PNote::setFreq( double freq ){
  setSubElementText( m_element, XS(PNote::TagFreq), ftoa(freq) );
}

void PNote::setMidi( MidiNo midi ){
  setSubElementText( m_element, XS(PNote::TagMidi), itoa(midi) );
}

void PNote::setOnset( double onset ){
  setSubElementText( m_element, XS(PNote::TagOnset), ftoa(onset) );
}

/**
 * Set's the minimum integer to be used when
 * creating new notes.
 */
void PNote::setMinID( int i ){
  noteNo = i;
}

void setSubElementText( DOMElement *element, const XMLCh *subElementName, string text ){

  DOMNodeList *list = element->getElementsByTagName( subElementName );

  DOMElement *subEl;

  if( list->getLength() == 0 ){
    DOMDocument *doc = element->getOwnerDocument();
    subEl = doc->createElement( subElementName );
    element->appendChild( subEl );
    DOMText *textNode = doc->createTextNode( XS(text.c_str()) );
    subEl->appendChild( (DOMNode*)textNode );
  }
  else{
    subEl = (DOMElement*)list->item(0);
    setText( subEl, text );
  }

}

/**
 * Returns string representation of pitch derived from 
 * one of the following
 *  1. Pitch or
 *  2. Midi or
 *  3. Freq
 */
std::string PNote::getPitchString(){

  //cout << "PNote::getPitchString\n";

  // if no pitch element exists, create one
  if( m_element->getElementsByTagName(XS("Pitch"))->getLength() != 0 ){


    PitchEl pel( (DOMElement*)m_element->getElementsByTagName(XS("Pitch"))->item(0) );

    return pel.getPitch().getAsString();

  }
  else if( hasMidi() ){
    DOMElement *midi = (DOMElement*)m_element->getElementsByTagName(XS(TagMidi))->item(0);
    return getText( midi );
  }
  else if(m_element->getElementsByTagName(XS(TagFreq))->getLength() != 0  ){

    DOMElement *freqEl = (DOMElement*)m_element->getElementsByTagName(XS("freq"))->item(0);
    string freq = getText( freqEl );

    Pitch p = m_doc->getTuning()->getPitch( atof(freq.c_str()) );
    return p.getAsString();

  }

 }


Pitch PNote::getPitch(){

  PitchEl p( (DOMElement*)m_element->getElementsByTagName(XS("pitch"))->item(0) );
  return p.getPitch();
}

bool PNote::hasMidi(){
  DOMNodeList *nl = m_element->getElementsByTagName(XS(TagMidi));
  if( nl->getLength() > 0 ) return true;
  else return false;
}

bool PNote::hasFreq(){
  DOMNodeList *nl = m_element->getElementsByTagName(XS(TagFreq));
  if( nl->getLength() > 0 ) return true;
  else return false;
}



/* SNote */

SNote::SNote( PMLDocument *doc, DOMElement *element ){
  m_doc = doc;
  m_element = element;
}

int SNote::getMeasure(){

  DOMElement *measure = (DOMElement*)m_element->getParentNode();
  return atoi( 
	      XS( measure->getAttribute(XS("number")) )
	      );
}

  /// Returns time within measure

/// Returns whether note has a chord element
bool SNote::isChord(){
  DOMNodeList *nl = m_element->getElementsByTagName(XS("chord"));
  if( nl->getLength() == 0 ) return false;
  else return true;
}

/**
 * Used to sort a std::vector<SNote*> contents by start time
 * Later sort concurrent notes by pitch.
 */
bool SNote::sortByTime( SNote *s1, SNote *s2 ){

  if( s1->getMeasure() != s2->getMeasure() ){
    return (s1->getMeasure() < s2->getMeasure());
  }
  else {
    return s1->getStartTime() < s2->getStartTime();
  }
}

/** If tied, returns type attribute as string, else returns NULL
 *  This returns tie type, not tied type which is different 
 *  Tie relates to logical tie, tied is notational tie. I think.
 */
bool SNote::isTied(){

  DOMElement *tie = getSingleElement(m_element, "tie");
  if( !tie ) return false;
  else
    return true;
}

string SNote::getTiedType(){
  DOMElement *tie = getSingleElement(m_element, "tie");
  if( tie == NULL ) return "not tied";
  return XS( tie->getAttribute(XS("type")) );
}




std::string SNote::getPitchString( bool onlySharps ){

  PitchEl pitch( (DOMElement*)m_element->getElementsByTagName(XS("pitch"))->item(0) );
  string rVal = pitch.getAsString();

  if( onlySharps ){
    /*
      Must change strings to be all sharps/naturals.
      Later the tuning class can deal with this, but for now,
      the transcribed pitches are either natural or sharp.
    */
    if( rVal == "Ab"  )      rVal = "G#";
    else if( rVal == "Bb"  ) rVal = "A#";
    else if( rVal == "Cb"  ) rVal = "B"; 
    else if( rVal == "Db"  ) rVal = "C#";
    else if( rVal == "Eb"  ) rVal = "D#";
    else if( rVal == "Fb"  ) rVal = "E";
    else if( rVal == "Gb"  ) rVal = "F#";
  }

  return rVal;
}

int SNote::getStartTime(){
  DOMElement *onset = (DOMElement*)m_element->getElementsByTagName(XS(SNote::Onset))->item(0);
  string time = getText(onset);
  return atoi(time.c_str());
}


Pitch SNote::getPitch(){
  PitchEl *p = new PitchEl( (DOMElement*)m_element->getElementsByTagName(XS("pitch"))->item(0) );
  return p->getPitch();
}


void SNote::align( PNote *note, bool correct ){

  DOMNodeList *list = m_element->getElementsByTagName(XS("align"));
  DOMElement *align;

  if( list->getLength() == 0 ){
    DOMDocument *doc = m_element->getOwnerDocument();
    align = doc->createElement(XS("align"));
    m_element->appendChild(align);
  }
  else{
    align = (DOMElement*)list->item(0);
  }


  align->setAttribute( XS("note"), XS(note->getID().c_str()) );
		       
  if( correct )
    setText( align, "correct" );
  else
    setText( align, "wrong" );
}

string SNote::getAlignedNoteID(){

  DOMNodeList *nl = m_element->getElementsByTagName(XS("align"));

  if( nl->getLength() == 0 )
    return "";

  DOMElement *align = (DOMElement*)nl->item(0);
  return XS( align->getAttribute( XS(ALIGN_TAG) ) );


}

string PNote::getAlignedNoteID(){

  DOMNodeList *nl = m_element->getElementsByTagName(XS("align"));

  if( nl->getLength() == 0 )
    return "";

  DOMElement *align = (DOMElement*)nl->item(0);
  return XS( align->getAttribute( XS(ALIGN_TAG) ) );



}


PitchEl::PitchEl( DOMElement *element ){
  m_element = element;
}

Accidental PitchEl::getAccidental( int alter ){
  switch( alter ){
  case 1: return Accidentals::Sharp;
  case -1: return Accidentals::Flat;
  case 0: return Accidentals::NoAccidental;
  default:{
    //cerr << "PitchEl::getAccidental No accidental found for given number!\n";
    return Accidentals::NoAccidental;
  }
  }
}

Pitch PitchEl::getPitch(){

  // default accidental
  Accidental acc = Accidentals::NoAccidental;
  int oct;

  DOMElement *noteEl = (DOMElement*)m_element->getElementsByTagName( XS(TagNote) )->item(0);
  string note = getText(noteEl);

  
  DOMElement *octEl = (DOMElement*)m_element->getElementsByTagName( XS( TagOct ) )->item(0);
  oct = atoi( getText(octEl).c_str() );
  
  DOMNodeList *nl = m_element->getElementsByTagName( XS( TagAcc ) );
  if( nl->getLength() > 0 ){
    DOMElement *accEl = (DOMElement*)nl->item(0);
    acc = getAccidental( atoi( getText(accEl).c_str() ) );
  }

  return Pitch( note[0], oct, acc );

}

PitchEl::PitchEl( DOMDocument *doc, Pitch p ){

  //reference pitch is C4

  m_element = doc->createElement(XS("pitch"));

  //calculate pitch from frequency
  //require tuning
  //calculate cents from A, then determine pitch

  // tuning system should be used to determine pitch
  // and decide whether two pitches are equal eg. A# Bb
  // for now just assume 12ET

  //cout << "PitchEl::PitchEl" << endl;

  int oct;

  char c = p.getNoteName();
  string s;
  s.append( 1, c );
  Accidental acc = p.getAccidental();

  DOMElement *stepEl = doc->createElement(XS("step"));
  setText( stepEl, s );
  m_element->appendChild( stepEl );

  if( acc != Accidentals::NoAccidental &&
      acc != Accidentals::Natural    ){

    DOMElement *accEl = doc->createElement(XS("alter"));
    m_element->appendChild( accEl );

    if( acc == Accidentals::Sharp )
      setText( accEl, "1" );
    else if( acc == Accidentals::Flat )
      setText( accEl, "-1" );
  }

  DOMElement *octave = doc->createElement(XS("octave"));
  setText( octave, itoa( p.getOctave() ) );
  m_element->appendChild( octave );

}

std::string PitchEl::getAsString(){

  //cout << "PitchEl::getAsString\t";

  DOMElement *step = (DOMElement*)m_element->getElementsByTagName(XS("step"))->item(0);
  string pitchString = getText(step);
  DOMNodeList *nodeList = m_element->getElementsByTagName(XS("alter"));
  if( nodeList->getLength() == 1 ){
    string accString = getText( (DOMElement*)nodeList->item(0) );
    switch( atoi( accString.c_str() ) ){
    case 1:
      accString = "#";
      break;
    case -1:
      accString = "b";
      break;
    }

    pitchString.append(accString);
  }
  return pitchString;
}


PerformanceIterator::PerformanceIterator( ScorePart *score, PerfPart *perf ){

  m_score = score;
  m_perf = perf;

  m_sit = m_score->getNotes()->begin();
  m_pit = m_perf->getNotes()->begin();

  checkStatus();

}

PerformanceIterator& PerformanceIterator::operator++(){


  // Iterate the counter based on current status, then recalculate status

  switch( m_status ){
  case CORRECT:{
    if( m_sit != m_score->getNotes()->end() )
      ++m_sit;
    if( m_pit != m_perf->getNotes()->end() )
      ++m_pit;
    break;
  }
  case WRONG:{
    if( m_sit != m_score->getNotes()->end() )
      ++m_sit;
    if( m_pit != m_perf->getNotes()->end() )
      ++m_pit;
    break;
  }
  case EXTRA:{
    if( m_pit != m_perf->getNotes()->end() )
      ++m_pit;
    break;
  }
  case MISSING:{
    if( m_sit != m_score->getNotes()->end() )
      ++m_sit;
    break;
  }
  default:{
    cout << "error\n";
  }
  }

  checkStatus();

  return *this;

}

PerformanceIterator& PerformanceIterator::operator--(){


  // check previous snote and pnote

 
  // decrement both 
  // if both have unequal matches: status = error
  // if only perf has match: status = missing, increment perf
  // if only score has match: status = extra, inccrement score
  // if they match: correct/wrong

  --m_pit;
  --m_sit;

  SNote *snote = *m_sit;
  PNote *pnote = *m_pit;

  string snoteAlign =  snote->getAlignedNoteID();
  string pnoteAlign =  pnote->getAlignedNoteID();


  if( snoteAlign == "" ){

    if( pnoteAlign == "" ){
      cout << "Fatal Error ! extra and missing together during decrement\n"
	   << "score id=" << snote->getID() << "\tperf id=" << pnote->getID() 
	   << endl;
      m_status = ERROR;
      return *this;
    }
    else{
      ++m_pit;
      m_status = MISSING;
      return *this;
    }
  }
  else if( pnoteAlign == "" ){
    ++m_sit;
    m_status = EXTRA;
      return *this;
  }
  else if( (pnote->getID() == snoteAlign) && (snote->getID() == pnoteAlign) ){

    if( snote->getPitchString(true) == pnote->getPitchString() ){
      m_status = CORRECT;
      return *this;
    }
    else{
      m_status = WRONG;
      return *this;
    }
  }
  else{
    cout << "Something really wrong here PerformanceIterator::operator--\n"
	   << snote->getID() << " matched to " << snote->getAlignedNoteID() << "\t"
	   << pnote->getID() << " matched to " << pnote->getAlignedNoteID() 
	   << endl;
  }



  return *this;

}

void PerformanceIterator::checkStatus(){

  SNote *snote = *m_sit;

  PNote *pnote = *m_pit;

  // Matcher rules state that in a contiguous group of results,
  // missing and extra notes cannot occur together (as they will
  // be resolved into wrong notes)

  /*  
  cout << "\nChecking status...\n";
  cout << "Score note: " << snote->getPitchString() << endl 
       << "\tid '" << snote->getID() << "'" << endl
       << "\taligned to '" << snote->getAlignedNoteID() << "'" << endl
       << endl
       << "Perf note: " << pnote->getPitchString() << endl 
       << "\tid '" << pnote->getID() << "'" << endl
       << "\taligned to '" << pnote->getAlignedNoteID() << "'" << endl;

  cout << endl;
  */

  string snoteID, snoteAligned, sPitch;
  string pnoteID, pnoteAligned, pPitch;
  if( m_sit == m_score->getNotes()->end() ) {
    snoteID = "end";
    snoteAligned = "end";
    pPitch = "end";
  }
  else {
    snoteID = snote->getID();
    snoteAligned = snote->getAlignedNoteID();
    sPitch = snote->getPitchString();
  }

  if( m_pit == m_perf->getNotes()->end() ) {
    pnoteID = "end";
    pnoteAligned = "end";
    pPitch = "end";
  }
  else {
    pnoteID = pnote->getID();
    pnoteAligned = pnote->getAlignedNoteID();
    pPitch = pnote->getPitchString();
  }



  if( snoteAligned == pnoteID ){
    
    if( snoteAligned == "" ){
      cout << "PerformanceIterator: FATAL ERROR! Missing and extra notes together!!\n"
	   << "score id=" << snoteID << "\tperf id=" << pnoteID 
	   << endl;
      m_status = ERROR;
    }
    else if( sPitch == pPitch ){
      m_status = CORRECT;
    }
    else{
      m_status = WRONG;
    }
    
  }
  else if( ( pnoteAligned == "") &&
	   ( snoteAligned != "") ){ // score is matched, so performance note must be extra
    m_status = EXTRA;
  }

  else if( ( snoteAligned == "") && 
	   ( pnoteAligned != "")    ){ // perf is matched, so score note must be extra
    m_status = MISSING;
  }
  else{
    cout << "PerformanceIterator: FATAL ERROR! element mismatch!!\n"
	   << snoteID << " matched to " << snoteAligned << "\t"
	   << pnoteID << " matched to " << pnoteAligned 
	   << endl;

    m_status = ERROR;
  }

  //cout << "Status " << getStatusString() << endl;

}


SNote* PerformanceIterator::getScoreNote(){
  return *m_sit;
}

PNote* PerformanceIterator::getPerfNote(){
  return *m_pit;
}



PerformanceIterator::Status PerformanceIterator::getStatus(){
  return m_status;
}

string PerformanceIterator::getStatusString(){

  switch( m_status ){
  case CORRECT:{
    return "Correct";
  }
  case WRONG:{
    return "Wrong";
  }
  case EXTRA:{
    return "Extra";
  }
  case MISSING:{
    return "Missing";
  }
  default:{
    return "Error";
  }
  }


}


PerformanceIterator::PerformanceIterator( PerformanceIterator &original ){


  m_status = original.getStatus();

  m_score = original.getScorePart();
  m_perf = original.getPerfPart();

  m_sit = original.getScoreIterator();
  m_pit = original.getPerfIterator();

}


PerformanceIterator& PerformanceIterator::operator=( PerformanceIterator &original ){


  m_status = original.getStatus();

  m_score = original.getScorePart();
  m_perf = original.getPerfPart();

  m_sit = original.getScoreIterator();
  m_pit = original.getPerfIterator();
  return *this;

}


bool PerformanceIterator::hasNext(){

  // Returns false if iterator isa at the end of either the score or the 
  // performance. (At this point, all successive notes will be extra or missing)
  PerfIterator pit = m_pit;
  ScoreIterator sit = m_sit;


  return !( ( ++pit == m_perf->getNotes()->end() ) ||
	    ( ++sit == m_score->getNotes()->end() )   ) ;
}

void PerformanceIterator::printStatus(){

  cout << "S: " << (*m_sit)->getPitchString() << " " << (*m_sit)->getID() 
       << "\tP: " << (*m_pit)->getPitchString() << " " << (*m_pit)->getID() 
       << "\tstatus: " << m_status 
       << endl;



}

void PerformanceIterator::removePNote(){

  m_pit = m_perf->removeNote( m_pit );
  checkStatus();
}

TuningEl::TuningEl( DOMElement *element ){
  m_element = element;

  // get name
  DOMElement *nameEl = (DOMElement*)m_element->getElementsByTagName( XS("name") )->item(0);
  string name = getText( nameEl );

  DOMElement *intervalListEl = (DOMElement*)m_element->getElementsByTagName(XS(TagIntervalList))->item(0);
  DOMNodeList *intervalList = intervalListEl->getElementsByTagName(XS(TagInterval));

  int size = intervalList->getLength();

  SpellingList spellings;
  IntervalList intervals;

  for( int i=0; i<size; i++ ){

    DOMElement *interval = (DOMElement*)intervalList->item(i);
    string centsText = getText( (DOMElement*)interval->getElementsByTagName(XS(TagInterval)) );

    //check if interval is cents or ratio and add to intervals
    DOMNodeList *nl = interval->getElementsByTagName( XS(TagCents) );

    double cents;

    if( nl->getLength() > 0 ){
      DOMElement *e = (DOMElement*)nl->item(0);
      cents = atof( getText( e ).c_str() );
    }
    else{
      // 2DO: convert ratio

    }

    intervals.push_back( cents );

    DOMNodeList *spellingElements = interval->getElementsByTagName(XS(TagSpelling));
    for( int s = 0; s < spellingElements->getLength(); s++ ){
      PitchEl pEl( (DOMElement*)spellingElements->item(s) );
      spellings.insert( SpellingListItem( i, pEl.getPitch().getSpelling() ) );
    }

  }

  m_tuning = new Tuning( name, &intervals, &spellings );


  // Reference pitches
  PitchEl refPitch( (DOMElement*)m_element->getElementsByTagName( XS(TagRefPitch) )->item(0) );
  PitchEl midiPitch( (DOMElement*)m_element->getElementsByTagName( XS(TagMidiPitch) )->item(0) );
  PitchEl rootPitch( (DOMElement*)m_element->getElementsByTagName( XS(TagRootPitch) )->item(0) );

  // Reference Frequency
  DOMElement *refFreq = (DOMElement*)m_element->getElementsByTagName(XS(TagRefFreq))->item(0);

  //Reference midi number
  DOMElement *midiNo = (DOMElement*)m_element->getElementsByTagName(XS(TagMidiNo))->item(0);
  MidiNo midiRefNo = atoi( getText(midiNo).c_str() );

  m_tuning->setRefNote( refPitch.getPitch(), atof(getText(refFreq).c_str()) );
  m_tuning->setMidiRef( midiPitch.getPitch(), midiRefNo );
  m_tuning->setRootPitch( rootPitch.getPitch().getSpelling() );

  m_tuning->printTuning();

}
