

#include <iostream>
#include <algorithms.h>
#include <xmlsc.h>
#include <errorhandler.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMEntity.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMBuilder.hpp>

using namespace std;
using namespace Algorithms;

DOMDocument *m_doc;
ErrorHandler *m_err;
DOMBuilder *m_builder;
DOMImplementation *m_impl;

void mergeParts( DOMElement *part1, DOMElement *part2 );
void alterDivisions( DOMElement *measure, int ratio, int staffNo );
DOMDocument *openDocument( char* filename );
void writeFile( string filename );


/*
  Argument is just a PML file. This will merge 2 parts into one.
*/

int main( int argc, char **argv ){

 
  m_doc = openDocument( argv[1] );

  DOMElement *root = m_doc->getDocumentElement();
  DOMNodeList *parts = root->getElementsByTagName(XS("part"));


  /*

  */
  DOMElement *part1 = (DOMElement*)parts->item(0);
  DOMElement *part2 = (DOMElement*)parts->item(1);

  mergeParts( part1, part2 );


  root->removeChild( part2 );
  DOMElement *partlist = getSingleElement( root, "part-list" );
  partlist->removeChild( partlist->getElementsByTagName(XS("score-part"))->item(1) );


  writeFile("merged.xml");



}
void writeFile( string filename ){

  DOMWriter *writer = m_impl->createDOMWriter();

  m_doc->setEncoding( XS("UTF-8") );
  m_doc->setActualEncoding( XS("UTF-8") );
  writer->setEncoding( XS("UTF-8"));
  writer->setErrorHandler( m_err );
  // use indentation in written file


  writer->setFeature( XS("format-pretty-print"), true);
  XMLFormatTarget *outFile = new LocalFileFormatTarget(filename.c_str());

  writer->writeNode( outFile, *m_doc  );
  outFile->flush();

  writer->release();

  delete outFile;

}


void mergeParts( DOMElement *part1, DOMElement *part2 ){

  DOMElement *partDef1 = m_doc->getElementById(part1->getAttribute(XS("id")));
  DOMElement *partDef2 = m_doc->getElementById(part2->getAttribute(XS("id")));

  DOMNodeList *measures1 = part1->getElementsByTagName(XS("measure"));
  DOMNodeList *measures2 = part2->getElementsByTagName(XS("measure"));
     
  if(measures1->getLength() != measures2->getLength())
    cerr << "parts are of different length!\n";

  //Set part to have two staffs
  DOMElement *printEl = getSingleElement( (DOMElement*)measures1->item(0), "print" );
  DOMElement *staffLayoutEl;
  DOMElement *numberEl;
  DOMElement *staffDistanceEl;
  if( printEl == NULL ){
    cout << "Creating print element\n";
    printEl = m_doc->createElement(XS("print"));
    staffLayoutEl = m_doc->createElement(XS("staff-layout"));
    numberEl = m_doc->createElement(XS("number"));
    staffDistanceEl = m_doc->createElement(XS("staff-distance"));


    ((DOMElement*)measures1->item(0))->insertBefore
      ( printEl, ((DOMElement*)measures1->item(0))->getFirstChild());

    printEl->appendChild(staffLayoutEl);
    staffLayoutEl->appendChild(numberEl);
    staffLayoutEl->appendChild(staffDistanceEl);
  }
  else{
    staffLayoutEl = getSingleElement( printEl, "staff-layout" );
    numberEl = getSingleElement( staffLayoutEl, "number" );
    staffDistanceEl = getSingleElement( staffLayoutEl, "staff-distance" );
  }

  setText( numberEl, "2" );
  setText( staffDistanceEl, "70");

  //-------------------------------------------------------------------------

  DOMElement *attributes1 = getSingleElement( (DOMElement*)measures1->item(0), "attributes" );
  DOMElement *attributes2 = getSingleElement( (DOMElement*)measures2->item(0), "attributes" );

  // get lcd of divisions, and set in part1
  int div1 = atoi( getText( getSingleElement( attributes1, "divisions" )).c_str());
  int div2 = atoi( getText( getSingleElement( attributes2, "divisions" )).c_str());
  int divlcm = lcm( div1, div2);
  cout << "Divs: " << div1 << " & " << div2 << " lcm " << divlcm << endl;
  setText(getSingleElement( attributes1, "divisions" ), itoa(divlcm) );

  //get clefs, number tham and move clef2 into part1
  DOMElement *clef1 = getSingleElement(attributes1,"clef");
  DOMElement *clef2 = getSingleElement(attributes2,"clef");


  clef1->setAttribute(XS("number"),XS(itoa(1).c_str()));
  clef2->setAttribute(XS("number"),XS(itoa(2).c_str()));

  //First, insert staves element
  DOMElement *stavesEl = m_doc->createElement(XS("staves"));
  setText(stavesEl, "2");
  attributes1->appendChild(stavesEl);
  attributes1->appendChild(clef2);


  //get timesig & critchetsperbar

  DOMElement *tsEl = getSingleElement(attributes1,"time");

  int tsn = atoi(getText(getSingleElement(tsEl,"beats")).c_str());
  int tsd = atoi(getText(getSingleElement(tsEl,"beat-type")).c_str());
  cout << "Time sig: " << tsn << "/" << tsd << endl;

  //calculate no of divisions per bar
  int divsperbar = divlcm * tsn / (tsd/4.0);
  cout << "Divs per bar: " << divsperbar << endl;

  //iterate through measures

  for(int i=0; i<measures1->getLength(); i++ ){

    cout << "Altering bars " << i << endl;

    DOMElement *m1 = (DOMElement*)measures1->item(i);
    DOMElement *m2 = (DOMElement*)measures2->item(i);

    //change divisions and staff numbers
    alterDivisions( m1, divlcm/div1, 1 );
    alterDivisions( m2, divlcm/div2, 2 );

    cout << "Adding last backup\n";

    //add backup element
    DOMElement *backupEl = m_doc->createElement(XS("backup"));
    DOMElement *durEl = m_doc->createElement(XS("duration"));
    backupEl->appendChild( durEl );
    setText(durEl, itoa(divsperbar));
    m1->appendChild(backupEl);

    cout << "Copying children over\n";

    //copy children from measure2 into measure1
    DOMNodeList *children = m2->getChildNodes();
    cout << "No children " << children->getLength() << endl;
    for( int c=0; c<children->getLength(); c++ ){
      cout << c << endl;
      cout << "No children " << children->getLength() << endl;
      if( children->item(c)->getNodeType() == DOMNode::ELEMENT_NODE ){

	DOMElement *e = (DOMElement*)children->item(c);

	string tagName = XS(e->getTagName());

	//if( tagName == "note" || tagName == "backup" || tagName == "forward" )
	if( tagName != "attributes" )

	  m1->appendChild( ((DOMElement*)children->item(c))->cloneNode(true) );
      
      }
    }
  }

}
 
void alterDivisions( DOMElement *measure, int ratio, int staffNo ){

  cout << "Altering notes\n";
  DOMNodeList *notes = measure->getElementsByTagName(XS("note"));
  for( int i=0; i<notes->getLength(); i++ ){
    DOMElement *note = (DOMElement*)notes->item(i);
    //cout << "note " << i << endl;
    DOMElement *durEl = getSingleElement(note,"duration");
    if( durEl != NULL ){  // grace notes have no duration
      int dur = atoi(getText(durEl).c_str());
      cout << " dur " << dur << endl;
      setText( durEl, itoa(dur*ratio) );
    }
    //set staff no
    DOMElement *staff = getSingleElement(note,"staff");
    if( staff == NULL ){
      staff = m_doc->createElement(XS("staff"));

      DOMElement *before = getSingleElement( note, "beam" );
      if( !before )
	before = getSingleElement( note, "notations" );
     
      if( before )
	note->insertBefore( staff, before );
      else
	note->appendChild(staff);
    }
    setText(staff,itoa(staffNo));

  }
  cout << "Notes & rests altered\n";
  DOMNodeList *backups = measure->getElementsByTagName(XS("backup"));
  for( int i=0; i<backups->getLength(); i++ ){
    DOMElement *durEl = getSingleElement( (DOMElement*)backups->item(i), "duration" );
    int dur = atoi(getText(durEl).c_str());
    setText( durEl, itoa(dur*ratio) );
  }
  cout << "Backups altered\n";
  DOMNodeList *forwards = measure->getElementsByTagName(XS("forward"));
  for( int i=0; i<forwards->getLength(); i++ ){
    DOMElement *durEl = getSingleElement( (DOMElement*)forwards->item(i), "duration" );
    int dur = atoi(getText(durEl).c_str());
    setText( durEl, itoa(dur*ratio) );
  }
  cout << "Forwards altered\n";
}


DOMDocument *openDocument( char* filename ){

  // Initialize the XML4C2 system
  try {
    XMLPlatformUtils::Initialize();
  }
  catch(const XMLException &toCatch) {
    XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
			      << "  Exception message:"
			      << XS(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
    exit(1);
  }

  // get Implementation and DOMBuilder
  const XMLCh *features = XS("XML 2.0");
  m_impl = DOMImplementationRegistry::getDOMImplementation( features );
  m_builder = ((DOMImplementation*)m_impl)->createDOMBuilder(DOMImplementation::MODE_SYNCHRONOUS, 0);


  //create & register error handler
  m_err = new ErrorHandler();
  m_builder->setErrorHandler( m_err );

  m_builder->setFeature( XMLUni::fgDOMValidation, true );
  
  cout << "---- Parsing file " << filename << " ----\n\n";
  try{
    m_doc = m_builder->parseURI( filename );
  }
  catch( const XMLException& e ){ 
    cout << "Message\n";
    cout << XS(e.getMessage()) << endl; 
  }
  catch( const DOMException& e ){ 
    cout << "Message\n";
    cout << XS(e.msg) << endl; 
  }
  catch (...) {
    cout << "Unknown exception\n";
  }
  cout << "---- File parsed ----\n\n";
    
  const string docType = XS( m_doc->getDoctype()->getName() );
  
  if( docType != "score-partwise" ){
    cout << "Not MusicXML file!\n";
    exit(1);
  }

  cout << "Loaded document\n";
  return m_doc;
}
  






