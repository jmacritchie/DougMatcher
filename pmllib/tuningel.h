#ifndef TUNING_EL_H
#define TUNING_EL_H


#include "pmlbase.h"
#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



class TuningEl : public PMLBase{

    public:
        TuningEl( DOMElement *element );

        static const char* TagIntervalList;
        static const char* TagInterval;
        static const char* TagCents;
        static const char* TagRatio;
        static const char* TagSpelling;
        static const char* TagMidiNo;
        static const char* TagMidiPitch;
        static const char* TagRefPitch;
        static const char* TagRefFreq;
        static const char* TagRootPitch;


    protected:

        Tuning *m_tuning;

};



#endif

