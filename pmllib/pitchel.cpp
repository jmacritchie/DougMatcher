
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "pitchel.h"
#include "pmldocument.h"
//#include "algorithms.h"
#include "pitch.h"
#include "xmlsc.h"
#include "pmlbase.h"


#include <string>
#include <list>
#include <sstream>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace std;
using namespace PMLAlgorithms;

#define XS XMLString::transcode


#define ALIGN_TAG "note"




/**
* PitchEl constructor
* m_element is the 'pitch' tag
**/
PitchEl::PitchEl( DOMElement *element ){
    m_element = element;
}


/**
*returns the accidental
**/
//there are nothing in place here to deal with double sharps/flats etc
//although this may not be important becuase i think 19ET can be written without the need for these
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
/**
* returns the pitch from the xml SNote.using the information in the tags
**/
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
        setText( octave, PMLAlgorithms::itoa( p.getOctave() ) );
        m_element->appendChild( octave );

}
/**
*Returns the pitch spelling as a string
*It does not include octave number
*It also only converts single sharps and flats to their symbol counterparts - not double sharps i.e *
*this is probably not an issue for the pitch spelling definition since no more than this is
*required for 19 and 12 ET - only for actual usage within a score
**/
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
