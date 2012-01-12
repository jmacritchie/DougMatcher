#ifndef PITCHEL_H
#define PITCHEL_H


#include "pmlbase.h"
#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



XERCES_CPP_NAMESPACE_USE




class PitchEl : public PMLBase{

    public:
        /*PitchEl( DOMDocument *doc, char name, int oct, int acc ); */
        PitchEl( DOMDocument *doc, Pitch p );   // create pitch from frequency (12ET) 
        PitchEl( DOMElement *pitch );

        std::string getAsString();

        Pitch getPitch();

        const static char* TagNote;
        const static char* TagAcc;
        const static char* TagOct;

    protected:

        Accidental getAccidental( int alter );

};

#endif
