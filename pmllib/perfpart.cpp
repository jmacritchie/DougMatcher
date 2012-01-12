
/*****************************************************
 *                                                   *
 * 2DO: removing PNote from a PerfPart does not      *
 *      guarantee removing it from the DOMdocument   *
 *      Should also check addNote                    *
 *                                                   *
 *                                                   *
 *****************************************************/

#include "perfpart.h"
#include "pmldocument.h"
#include "pitch.h"
#include "xmlsc.h"
#include "pmlbase.h"
#include "scorepart.h"
#include "pnote.h"


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


PerfPart::PerfPart( PMLDocument *doc, ScorePart *part ){
    m_doc = doc;

    DOMDocument *domdoc = part->getElement()->getOwnerDocument();
    m_scorePart = part;
    m_element = domdoc->createElement( XS("perfpart") );       //'perfpart' tag is the m_element for this class

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
