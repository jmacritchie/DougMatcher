
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "pmlbase.h"
#include "pmldocument.h"
#include "snote.h"
#include "pnote.h"
#include "pitch.h"
#include "xmlsc.h"
#include "pitchel.h"
#include "tuningel.h"
#include "perfpart.h"

#include <string>
#include <list>
#include <sstream>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace std;
//using namespace Algorithms;

#define XS XMLString::transcode


#define ALIGN_TAG "note"


const char* SNote::Onset = "starttime";

const char* PerfPart::Tag = "perfpart";

const char* PNote::Tag = "event";
const char* PNote::TagMidi = "midi";
const char* PNote::TagMidiVelocity = "velocity";  //BEN - extra added to include midi note velocity please see pnote for more
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



/**
*Returns a pointer to element
**/
DOMElement *PMLBase::getElement(){ return m_element; }



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



//this should possibly be moved as it is not strictly part of pml base.  leave here for now
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
