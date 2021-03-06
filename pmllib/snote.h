#ifndef SNOTE_H
#define SNOTE_H

#include "note.h"
#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"
#include "pnote.h"


#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>

class PNote;

/* Need to add duration nethods */
class SNote : public Note{

    public:

        SNote( PMLDocument *doc, DOMElement* element );

        int getMeasure();
        int getStartTime();
        ScorePart *getPart();


        Pitch getPitch();
        std::string getPitchString( bool onlySharps=false );

        void align( PNote *note, bool correct );
        std::string getAlignedNoteID();

        int getVoice();
        int getStaff();

        bool isChord();
        bool isTied();
        std::string getTiedType();
  /// Used to sort a std::vector<SNote*> contents by start time
        static bool sortByTime( SNote* s1, SNote* s2 );

        static const char* Onset;

};


#endif
