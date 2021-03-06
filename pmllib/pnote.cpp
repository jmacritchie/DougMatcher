
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "pnote.h"
#include "pmldocument.h"
#include "pitch.h"
#include "pitchel.h"
#include "xmlsc.h"
#include "note.h"
#include "snote.h"


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


/* PNote */


int PNote::noteNo = 0;
/**
*The Constructor can be called with several different combinations of arguments.
*Crucially it requires the PML document and an onset and offset.
**/
PNote::PNote( PMLDocument *doc, double onset, double offset ){
    init( doc, onset, offset );
}
PNote::PNote( PMLDocument *doc, double onset, double offset, MidiNo midi ){
    init( doc, onset, offset );
    setMidi( midi );
}

//BEN - extra for velocity
PNote::PNote( PMLDocument *doc, double onset, double offset, MidiNo midi, int velocity ){
	init( doc, onset, offset );
	setMidi( midi );
	setMidiVelocity( velocity );
}

PNote::PNote( PMLDocument *doc, double onset, double offset, double freq ){
    init( doc, onset, offset );
    setFreq( freq );
}
/**initialise the pnote - creating an XML - 'Event' in the XML tree for it 
* and adding other info i.e onset/offset/pnote id
* for a pnote, m_element is the pnote 'event' tag
**/
void PNote::init( PMLDocument *doc, double onset, double offset ){
    m_doc = doc;
    m_element = doc->getDocument()->createElement( XS(PNote::Tag) );
    setOnset( onset );
    setOffset( offset );

    noteNo++;

    string id = "pnote" + PMLAlgorithms::itoa( noteNo );
  
    //ensure this id is not used already 
    //if it is then increment noteNo and recheck until an unused id is found
    while( doc->getDocument()->getElementById( XS(id.c_str()) ) != NULL ){
        noteNo++;
        id = "pnote" + PMLAlgorithms::itoa( noteNo );
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

/**
* aligns a pnote with an snote
**/
void PNote::align( SNote *note, bool correct ){

    //checks for an align tag within the pnote 'event' tag (m_element)
    //if one doesn't exist it is added as a child of the 'event' tag
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
    //'align' tag attributes then set as - note="snote id"
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

//BEN - xtra for midivelocity
int PNote::getMidiVelocity(){
	if( hasMidiVelocity() ){
		DOMElement *midivelocity = (DOMElement*)m_element->getElementsByTagName( XS(PNote::TagMidiVelocity) )->item(0);
		string text = getText( midivelocity );
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


//BEN - xtra for midi velocity
void PNote::setMidiVelocity( int velocity ){
	setSubElementText( m_element, XS(PNote::TagMidiVelocity), itoa(velocity) );
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



/**
 * Returns string representation of pitch derived from 
 * one of the following
 *  1. Pitch or
 *  2. Midi or
 *  3. Freq
 *
 * The Representation is in the form of note and accidental - i.e - Bb
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

//BEN - extra for midi velocity
bool PNote::hasMidiVelocity(){
    DOMNodeList *nl = m_element->getElementsByTagName(XS(TagMidiVelocity));
    if( nl->getLength() > 0 ) return true;
    else return false;
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

/**
*returns the aligned noteID for the pnote as a string
*If it is not aligned (there is no alignment tag) then NULL is returned.
**/
string PNote::getAlignedNoteID(){

    DOMNodeList *nl = m_element->getElementsByTagName(XS("align"));

    if( nl->getLength() == 0 )
        return "";

    DOMElement *align = (DOMElement*)nl->item(0);
    return XS( align->getAttribute( XS(ALIGN_TAG) ) );



}
