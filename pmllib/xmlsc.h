
#ifndef XML_SC_H
#define XML_SC_H

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>

XERCES_CPP_NAMESPACE_USE

#define XS XMLString::transcode



void setText( DOMElement *e, std::string s );
std::string getText( DOMElement* e );
DOMElement *getSingleElement( DOMElement *e, std::string name );


#endif