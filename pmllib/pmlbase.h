#ifndef PMLBASE_H
#define PMLBASE_H


#include "pmldocument.h"
//#include "pnote.h"
//#include "snote.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



XERCES_CPP_NAMESPACE_USE

class PNote;
class SNote;
class PerfPart;
class ScorePart;
class PitchEl;
class TuningEl;

//there is a better place to move these iterators to.  Keep them here for now.
typedef std::list<PNote*>::iterator PerfIterator;
typedef std::list<SNote*>::iterator ScoreIterator;


void setSubElementText( DOMElement *element, const XMLCh *subElementName, std::string text );

/**
*The base class for PML elements
**/
class PMLBase{
    protected:
        DOMElement* m_element;
        PMLDocument* m_doc;
    public:
        DOMElement* getElement();
        std::string getID();
        void removeElement();
};



#endif
