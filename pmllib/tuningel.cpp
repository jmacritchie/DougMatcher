
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "tuningel.h"
#include "pmldocument.h"
#include "pitch.h"
#include "xmlsc.h"
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






/**Tuning Element constructor
* This reads the tunings from the pml file and reads and stores them in the tuning class
* m_element is 'tuning' tag
**/
TuningEl::TuningEl( DOMElement *element ){
    m_element = element;

  // get name of tuning i.e 19ET
    DOMElement *nameEl = (DOMElement*)m_element->getElementsByTagName( XS("name") )->item(0);
    string name = getText( nameEl );

    DOMElement *intervalListEl = (DOMElement*)m_element->getElementsByTagName(XS(TagIntervalList))->item(0);
    DOMNodeList *intervalList = intervalListEl->getElementsByTagName(XS(TagInterval));

    //number of intervals for this tuning
    int size = intervalList->getLength();

    //SpellingList and IntervalList types are typedefs from 'tunings.h'
    SpellingList spellings;
    IntervalList intervals;
    
    //iterate through each 'interval' tag and store information in the Spelling and Interval Lists
    //This information can then be accessed to give tuning information.
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
        
        //places the cents intervals into the IntervalList (vector from tuning.h) 
        intervals.push_back( cents );
  
        //this creates a pitch element and inserts into into the SpellingList (tuning.h)
        //based on the information 
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
