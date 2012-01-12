#ifndef NOTE_H
#define NOTE_H

#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"
#include "pmlbase.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



class Note : public PMLBase{
    public:
        virtual Pitch getPitch(){}; // {} avoids typeinfo error
        virtual std::string getPitchString(){};
};

#endif
