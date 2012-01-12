#ifndef SCOREPART_H
#define SCOREPART_H

#include "pmldocument.h"
#include "pmlbase.h"
#include "snote.h"
#include "perfpart.h"

#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>

class Perfpart;
//class SNote;


/**
*Scorepart class inherits from the PML base class
**/
class ScorePart : public PMLBase  {

    public:
        ScorePart( PMLDocument * doc, DOMElement *element );

        PerfPart *getPerfPart();

        std::string getName();

        int getNoNotes();
        SNote *getNote( int index );
        SNote *getNote( std::string id );
        std::list<SNote*> *getNotes();
        std::string getAlignedNoteID();
        int getDivisions();

    protected:

        DOMElement *m_definition; // points to score-part element which contains part metadata
        std::list<SNote*> m_notes;
};

#endif
