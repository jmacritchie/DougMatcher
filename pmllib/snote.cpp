

/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "snote.h"
#include "pmldocument.h"
#include "pitch.h"
#include "xmlsc.h"
#include "note.h"
#include "pitchel.h"


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





/* SNote */
/**
* SNote constructor
* m_element for this is the 'note' tag
**/
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
