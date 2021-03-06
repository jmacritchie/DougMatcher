
#include <string>
#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/util/XMLString.hpp>

#include "xmlsc.h"


using std::cout;
using std::cerr;
using std::string;
using std::endl;


XERCES_CPP_NAMESPACE_USE



string getText( DOMElement* e ){

  for( DOMNode *node = e->getFirstChild();
       node != NULL;
       node = node->getNextSibling() ){

    if( node->getNodeType() == DOMNode::TEXT_NODE ){
      const XMLCh *ch = ((DOMText*)node)->getData();
      string s = XS(ch);
      //delete[] ch; memory leak here
      return  s;
    }

  }

  return "";

}

void setText( DOMElement *e, string s ){

  // remove text node, and append new one

  bool hasTextNode = false;

  for( DOMNode *node = e->getFirstChild();
       node != NULL;
       node = node->getNextSibling() ){

    if( node->getNodeType() == DOMNode::TEXT_NODE ){
      e->removeChild( node );
      //delete node;
    }
  }

  DOMDocument *doc = e->getOwnerDocument();
  e->appendChild( doc->createTextNode( XS(s.c_str()) ) );


}

DOMElement *getSingleElement( DOMElement *e, string name ){

  DOMNodeList *nl = e->getElementsByTagName( XS(name.c_str()) );
  if( nl->getLength() > 0 ) return (DOMElement*)nl->item(0);

  //cerr << "getElement: no element found " << name << endl;
  return NULL;
}
