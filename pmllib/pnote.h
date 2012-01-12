#ifndef PNOTE_H
#define PNOTE_H

#include "note.h"
#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"
#include "snote.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



XERCES_CPP_NAMESPACE_USE

class SNote;

class PNote : public Note{

  /// used to create id attributes
    static int noteNo;
    void init( PMLDocument *doc, double onset, double offset );

    public:

        PNote( PMLDocument *doc, double onset, double offset );
        PNote( PMLDocument *doc, double onset, double offset, MidiNo midi );
	PNote( PMLDocument *doc, double onset, double offset, MidiNo midi, int velocity ); //BEN - extra constructor added to accomodate velocity
        PNote( PMLDocument *doc, double onset, double offset, double freq );
        PNote( PMLDocument *doc, DOMElement* );

        static const char* Tag;
        static const char* TagMidi;
	static const char* TagMidiVelocity;
        static const char* TagOnset;
        static const char* TagOffset;
        static const char* TagFreq;

        void setOnset( double onset );
        void setOffset( double offset );
        void setFreq( double freq );
        void setMidi( MidiNo midi );
	void setMidiVelocity( int velocity );  //extra for midi velocity

        double getOnset();
        double getOffset();
        double getFreq();
        MidiNo getMidi();
	int getMidiVelocity();  //BEN - extra for velocity

  /// Returns true if average frequency is specified in element
        bool hasFreq();
  /// Returns true if midi number is specified in element
        bool hasMidi();

	//BEN - extra for velocity data
    /// Returns true if midi velocity is specified in element	
	bool hasMidiVelocity();

        std::string getPitchString();
        Pitch getPitch();

        void align( SNote *note, bool correct=true );
        void unalign();
        std::string getAlignedNoteID();

  /// Sets min id to be used for new pnotes
        void setMinID( int i );

};

#endif
