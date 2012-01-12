
#include "pmldocument.h"


void usage(){
  cout << "Usage:\n"
       << "\ttempo <pmlfile> noteId\n";
}


int main( int argc, char **argv ){
  
  // Check command line and extract arguments.
  if ( argc != 3 ) {
    usage();
    return 1;
  }

  const char *inFile = argv[1];//argv[1];
    
  PMLDocument doc;
  doc.openFile( inFile,false );
 
  double tempo = getTempo( doc, argv[2] );
 
  cout << "Tempo is " << tempo << endl;

}


double getTempo( PMLDocument &doc, string noteID ){


  DOMElement* tNoteEl = (DOMElement*)doc.getElementByID(XS(noteID));
  SNote tNote( tNoteEl );

  ScorePart spart( (DOMElement*)tNote->getParentNode()->getParentNode() );
  PerfPart ppart = spart->getPerfPart();

  std::list<SNote*> noteList = spart.getNotes();

  int tTime = tNote->getStartTime();

  cout << "Target note:\n"
       << "\t" << noteID << endl
       << "\t" << tNote->getPitchString() << endl
       << "\ttime: " << tTime << endl;


  DOMElement *m1 = (DOMElement*)spart.getElement()->getElementsByTagName(XS("measure"))->item(0);

  string tsDen = getText( m1->getElementsByTagName(XS("attributes"))
			->getElementsByTagName(XS("time"))
			->getElementsByTagName(XS("beat-type"))  );

  string divString = getText( m1->getElementsByTagName(XS("attributes"))
			->getElementsByTagName(XS("divisions"))  );
  int divs = atoi(divString);

  cout << "div: " << divs << endl;

  cout << "Range " << tTime-divs << " to " << tTime+divs << endl;


  std::list<SNote*>::iterator start, end, it;

  for( start = noteList->begin(); (*start)->getStartTime < tTime-divs; start++ );
  for( end = start; (*end)->getStartTime < tTime+divs; end++ );


  for( it = start; it != end; it++ ){

    SNote *snote = (*it);
    int stime = snote->getStartTime();

    PNote *pnote = 

  }

}
