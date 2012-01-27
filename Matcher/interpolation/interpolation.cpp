 


#include <pmldocument.h>
#include <pmlelements.h>
#include <algorithms.h>
#include <iostream>
#include <fstream>
#include <list>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;
using namespace Algorithms;


/*
  Performs interpolation using Scilab.
  Information xferred using files.
  Scilab run in batch mode

  Note: makeLists() is destructive to the 
  ScorePart it receives, so a new one must be 
  generated each call.

  Also note: this is designed to use ids in the form
  noteX and pnoteX for score and perf respect.


  2Do: the input options are really shite
  But... once I've proven that my way is best
  there's no need for the others ;¬)

*/

std::list<SNote*>::iterator fit, bit, oldIt;
Tuning *tuning;
double thresholdAlign;
double thresholdUnalign;
PMLDocument doc;
int partNo;
int staff;
bool useScore=true;
bool evalByVoice=false;
bool useStdDev=false;

// perform evaluation using mxml voice attributes
// Useful for the Chopin stuff
#define UNALIGN 1
#define DBG 1


void unalign( int rTime, list<PNote*> perf );
void unalign();
void scoreUnAlign();
void scoreReAlign();
void realign();
void makeLists();
void removeNoteFromList( SNote &note, list<SNote*>* score );



void usage(){
  cout << "\nUsage:\n"
       << "\tTo just evaluate the correspondences of a pml file\n"
       << "\t\tintermatch ev <pmlfile>\n"
       << "\n"
       << "\n\tFor static thresholds (ns) (no score)\n"
       << "\t\tintermatch ns <pmlfile> <Align threshold>\n"
       << "\t\tintermatch std <pmlfile> <Align threshold (score)>\n"
       << "\n"
       << "\t\tTo unalign only\n"
       << "\t\tintermatch unalign <threshold>\n"
       << "\n"
       << "\t\tintermatch s  <pmlfile> <Align threshold>\n\n"
       << "\t\tintermatch s  <pmlfile> <Unalign threshold> <Align threshold> <staff> \n"
       << "\n"
       << "\t\tVoice independent interpolation\n"
       << "\t\tintermatch vi <pmlfile> <Align threshold>\n\n"
       << "\t\tintermatch vi <pmlfile> <Unalign threshold> <Align threshold> <staff> \n\n";

  exit(1);
}



int main( int argc, char **argv ){
  
  if( argc < 2 ) usage();

  if( !strcmp( argv[1], "vi" ) ){

    if( argc != 6 ) usage();
 
    thresholdUnalign = atof(argv[3]);
    thresholdAlign = atof(argv[4]);
    staff = atoi(argv[5]);

    evalByVoice = true;
    cout << "Analysing staff " << staff << "\n";
    argc--;

  }
  else if( !strcmp( argv[1], "ev" ) ){
  }
  else{
    if( !strcmp( argv[1], "ns" ) ){ 
      useScore=false;
      cout << "Using static based thresholds\n";
    }
    else if( !strcmp( argv[1], "s" ) ){
      cout << "Using dynamic, score-based thresholds\n";
    }
    if( argc == 4 ){
      thresholdUnalign = -1;
      thresholdAlign = atof(argv[3]);
    }
    else if( argc == 5 ){
      thresholdUnalign = atof(argv[3]);
      thresholdAlign = atof(argv[4]);
    }
    else
      usage();

    if( !strcmp( argv[1], "std" ) ){
      cout << "Using std deviation unalign\n";
      useStdDev=true;
      useScore=true;
      thresholdAlign = atof(argv[3]);
      thresholdUnalign = -1;
    }
    if( !strcmp( argv[1], "unalignstd" ) ){
      cout << "Only unalign using score\n";
      useScore=true;
      thresholdAlign = -1;
      thresholdUnalign = atof(argv[3]);
    }


  }


  cout << "Aligning threshold is set to " << thresholdAlign << endl;
  if( thresholdUnalign > 0 )  
    cout << "Unaligning threshold is set to " << thresholdUnalign << endl;

  cout << endl;

  // Open PML file
  doc.openFile( argv[2], false );
  tuning = doc.getTuning();

  if( doc.getNoParts() == 1 )
    partNo=0;
  else{
    printf("Which Part is to be analysed?\n");
    for( int i=0; i<doc.getNoParts(); i++ )
      printf("\t%d\t%s\n", i, doc.getScorePart(i)->getName().c_str() );
    
    do{
      scanf("%d", &partNo );
      cout << partNo << endl;
    }while( 0 > partNo && partNo >= doc.getNoParts() );
  }

  // Make lists of matched notes

  if( !strcmp( argv[1], "ev" ) ){
    makeLists();
    return 0;
  }

  /*
    Unmatch clusters which show high error
    in interpolation
  */

  if( thresholdUnalign > 0 || useStdDev ){
  
    makeLists();
    system("cp matched matched1");
    system("cp unmatched_score unmatched_score1");
    system("cp unmatched_perf unmatched_perf1");
    
    
    // run scilab to get estimated times & tempos for matched notes
    system( "scilab -f /home/dougie/Work/matcher/interpolation/sls -nwni -args 'first'");
    //cerr << "Scilab done.\n";

    /*
      Unalign either using score & tempo or 
      static threshold
     */        

    if( useStdDev )
      unalign();
    else
      scoreUnAlign();

  }

  
  makeLists();
  system("cp matched matched2");
  system("cp unmatched_score unmatched_score2");
  system("cp unmatched_perf unmatched_perf2");
  
  // run scilab to get estimated times & tempos for unmatched notes
  cerr << "Running Scilab Again.\n";
  system( "scilab -f /home/dougie/Work/matcher/interpolation/sls -nwni -args 'second'");
  cerr << "Scilab done.\n";
  
  // iterate through unmatched notes, match matchable perf notes
  // make a final list to check 
  
  if( useScore )
    scoreReAlign();
  else
    realign();

  makeLists();
  system("cp matched matched3");
  system("cp unmatched_score unmatched_score3");
  system("cp unmatched_perf unmatched_perf3 ");

  

  doc.writeFile("interpolated.pml");
  
}

/* intout contains: 
 *   rtime, 
 *   id, 
 *   midi, 
 *   atime 
 *   current tempo
 */

/**
 * Reads file and searches for notes to realign. 
 * 'intout' created by scilab process contains 
 * information re unaligned score notes: 
 * For each score note, the method searches for
 * suitable pnotes and aligns them to the snote
 * based on a static threshold in seconds
 */
void realign(){

  ScorePart *spart = doc.getScorePart(partNo);
  PerfPart *ppart = spart->getPerfPart();

  list<SNote*> *score = spart->getNotes();
  list<PNote*> *perf = ppart->getNotes();

  ifstream in;
  in.open("intout");

  int rtime, midi, id; 
  double atime, tempo; 

  // duffer & in.get cause scilab outputs floats
  int duffer;
  while( !in.eof() ){
    in >> rtime; in.get();  in >> duffer;
    in >> id;    in.get();  in >> duffer;
    in >> midi;  in.get();  in >> duffer;
    in >> atime;
    in >> tempo;

    string sid = "note";
    sid.append(itoa(id));

    cout << "\nIn " << rtime 
	 << " " << id
	 << " " << midi
	 << " " << atime << endl;

    // incredibly inefficient, but easy
    list<PNote*>::iterator pit = perf->begin();
    while( pit != perf->end() ){
      
      PNote *p = *pit;
      double onset = p->getOnset();

      if( p->getAlignedNoteID() == "" ){
	if( fabs( onset - atime ) < thresholdAlign &&
	    p->getMidi() == midi ){
	  string pid = p->getID();
	  p->align( new SNote(&doc, doc.getElementById(sid)) );
	  cout << "Aligning " << sid 
	       << " to " << p->getID() 
	       << endl;
	  break;
	}
      }
      
      pit++;
    }
    
  }
  
  delete(spart);
  delete(ppart);
  
  
}
 
 
 
/**
 * Reads file and searches for notes to realign. 
 * 'intout' created by scilab process contains 
 * information re unaligned score notes: 
 * For each score note, the method searches for
 * suitable pnotes and aligns them to the snote.
 *
 * The threshold is a value 0-1 representing
 * distance between this note and previous/next
 * note. Times for next/previous notes are based
 * on tempo calculations
 * 
 */
void scoreReAlign(){

  ScorePart *spart = doc.getScorePart(partNo);
  PerfPart *ppart = spart->getPerfPart();

  list<SNote*> *score = spart->getNotes();
  list<PNote*> *perf = ppart->getNotes();

  ifstream in;
  in.open("intout");
  int rTime, midi, id; double eTime, tempo;

  int startTime;
  list<SNote*>::iterator prev, next, sit = score->begin();
  list<SNote*>::iterator bIt = score->begin();
  list<SNote*>::iterator eIt = --score->end();// Dec prevents segfault

  /*
   Scilab outputs integers as 1.000000.
   So after getting an int,
   the in.get() will swallow decimal point
   and duffer will swallow the zeros
  */
  int duffer;

  while( !in.eof() ){
    in >> rTime; in.get(); in >> duffer;if( duffer != 0 )cerr << "HELP!\n"; 
    in >> id;    in.get(); in >> duffer;if( duffer != 0 )cerr << "HELP!\n"; 
    in >> midi;  in.get(); in >> duffer;if( duffer != 0 )cerr << "HELP!\n"; 
    in >> eTime;
    in >> tempo;

    string sid = "note";
    sid.append(itoa(id));

    cout << "In " << rTime 
	 << "\tid " << id
	 << "\tmidi " << midi
	 << "\teTime " << eTime
	 << "\ttempo " << tempo
	 << endl;

    // Find any note at rTime (just to get thresholds)
    do{
      if( (*sit)->getStartTime() < rTime )sit++;
      if( (*sit)->getStartTime() > rTime )sit--;
    }while( (*sit)->getStartTime() != rTime );
    startTime = (*sit)->getStartTime();

    // Find ioi between next and prev notes
    prev = sit, next = sit;
    if( evalByVoice ){
      int voice = (*sit)->getStaff();
      while( prev!=bIt && ((*prev)->getStartTime()==rTime || (*prev)->getStaff()!=voice) )
	prev--;
      while( next!=eIt && ((*next)->getStartTime()==rTime ||  (*next)->getStaff()!=voice) )
	next++;
    }
    else{
      while( prev!=bIt && (*prev)->getStartTime()==rTime )prev--;
      while( next!=eIt && (*next)->getStartTime()==rTime )next++;
    }

    int ioiPrev = startTime - (*prev)->getStartTime();
    int ioiNext = (*next)->getStartTime() - startTime;

    // Calculate thresholds based on eTime & tempo
    double threshPrev = eTime - (thresholdAlign * ioiPrev*tempo);
    double threshNext = eTime + (thresholdAlign * ioiNext*tempo);

    if(DBG) cout << "Prev points to " << (*prev)->getStartTime() << " " 
		 << (*prev)->getID() 
		 << " ioi " << ioiPrev
		 << " thresh " << threshPrev << endl;
    if(DBG) cout << "Next points to " << (*next)->getStartTime() 
		 << " " << (*next)->getID() 
		 << " ioi " << ioiNext
		 << " thresh " << threshNext << endl;
    cout << "  Search for note between " << threshPrev << "\t" << threshNext << endl;

    // Inefficient, but easy
    list<PNote*>::iterator pit = perf->begin();
    while( pit != perf->end() ){
      
      PNote *p = *pit;
      double onset = p->getOnset();

      if( p->getAlignedNoteID() == "" ){

	// If onset is within thresholds & midi note is the same
	// Align perf note to score note
	if( p->getMidi() == midi && threshPrev < onset && onset < threshNext ){ 
	  string pid = p->getID();
	  p->align( new SNote(&doc, doc.getElementById(sid)) );
	  cout << "\t\tAligning " << sid 
	       << " to " << p->getID() 
	       << endl;
	  break;
	}
      }
      
      pit++;
    }
    cout << "Reached end of search\n";
  }
  delete(spart);
  delete(ppart);
}
  
  
/**
 * Unaligns notes based on tempo calculations
 */
void scoreUnAlign(){

  ScorePart *spart = doc.getScorePart(partNo);
  PerfPart *ppart = spart->getPerfPart();

  list<SNote*> *score = spart->getNotes();
  list<PNote*> *perf = ppart->getNotes();
  list<SNote*>::iterator prev, next, sit = score->begin();
  list<SNote*>::iterator bIt = score->begin();
  list<SNote*>::iterator eIt = --score->end();// Dec prevents segfault
  cout << "eIt points to " << (*eIt)->getID() << endl;
  int startTime;
  ifstream in("tempos");

 
  while( !in.eof() ){

    // Get relative time, performed time, estimated time & tempo(secs/div)
    int rTime;
    double eTime, pTime, tempo;
    int duffer; //search 'duffer' in this file for explanation
    in >> rTime; in.get(); in >> duffer; if( duffer != 0 )cerr << "HELP!\n";  
    in >> pTime;   
    in >> eTime;
    in >> tempo;

    // Find note at rTime
    do{
      if( (*sit)->getStartTime() < rTime )sit++;
      if( (*sit)->getStartTime() > rTime )sit--;
    }while( (*sit)->getStartTime() != rTime );
    startTime = (*sit)->getStartTime();

    cout << "\nAnalysing " << startTime << " " << (*sit)->getID() << endl;

    // Find ioi between next and prev notes
    // 2DO! add the option to perform this by voice
    prev = sit, next = sit;

    if( evalByVoice ){
      int voice = (*sit)->getStaff();
      cout << "Note is in voice " << voice << endl;
      int st=(*prev)->getStartTime();
      int stff=(*prev)->getStaff();
      string id = (*prev)->getID();
      cout << "Prev: " << id << " " << stff << " " << st << endl;
      while( prev!=bIt && ((*prev)->getStartTime()==rTime || (*prev)->getStaff()!=voice) )
	prev--;
      while( next!=eIt && ((*next)->getStartTime()==rTime ||  (*next)->getStaff()!=voice) )
	next++;

    }
    else{
      while( prev != bIt &&  (*prev)->getStartTime() == rTime )prev--;
      while( next != eIt &&  (*next)->getStartTime() == rTime )next++;
    }

    if(DBG) cout << "Prev points to " << (*prev)->getStartTime() 
		 << " " << (*prev)->getID() << endl;cout.flush();
    if(DBG) cout << "Next points to " << (*next)->getStartTime() 
		 << " " << (*next)->getID() << endl;

    int ioiPrev = startTime - (*prev)->getStartTime();
    int ioiNext = (*next)->getStartTime() - startTime;

    // Calculate thresholds based on eTime & tempo
    double threshPrev = eTime - (thresholdUnalign * ioiPrev*tempo);
    double threshNext = eTime + (thresholdUnalign * ioiNext*tempo);


    cout << "Performed time " << pTime 
	 << "\tEstimated time " << eTime
	 << "\tdiff " << (eTime - pTime)
	 << endl;

    if( threshPrev < 0 ) threshPrev = 0;

    cout << "Threshold: " << threshPrev << " " << threshNext << endl;
    
    // Find note relating to this score time

    if( pTime < threshPrev || threshNext < pTime ){
      cout << "Unaligning cluster!\n";
      list<PNote*>::iterator pit = perf->begin();
      while( pit != perf->end() ){
	
	PNote *p = *pit;
	string id = p->getAlignedNoteID();
	double pTime = p->getOnset();

	if( !id.empty() ){

	  SNote snote( &doc, doc.getElementById(id) );
	  //cout << "SNote " << snote.getStartTime() << endl;
	  if( !evalByVoice || snote.getStaff() == staff ){
	    if( snote.getStartTime() == rTime ){
	      p->unalign();
	      if(DBG)cout << "Unaligning " << p->getID() << " from " << snote.getID() << endl;
	      if(DBG)cout << snote.getStartTime() << " " << p->getOnset() << endl;
	    }
	  }

	}

      pit++;
      }
    }
    cout << "Cluster ok\n";
  }
  delete(spart);
  delete(ppart);
}





/// Unalign all performance notes which correspond to rTime in score
void unalign( int rTime, list<PNote*> *perf ){
  list<PNote*>::iterator pit = perf->begin();
  while( pit != perf->end() ){
    
    PNote *p = *pit;
    string id = p->getAlignedNoteID();
    if( !id.empty() ){
      SNote snote( &doc, doc.getElementById(id) );
      //cout << "SNote " << snote.getStartTime() << endl;
      if( snote.getStartTime() == rTime ){
	p->unalign();
	cout << "Unaligning " << p->getID() << " from " << snote.getID() << endl;
	cout << snote.getStartTime() << " " << p->getOnset() << endl;
      }
    }
    pit++;
  }
}

#define LOG_LISTS 0

/**
 * Makes lists required to perform interpolation
 *
 *  matched         - contains relative times matched to real times
 *  matches         - ids of all matched pairs
 *
 *  unmatched_perf  - onset id & midi 
 *  extra           - just id
 *
 *  unmatched_score - time id & midi number
 *  missing         - just id
 */
void makeLists(){

  ofstream m( "matched", ios::trunc );
  ofstream us( "unmatched_score", ios::trunc );
  ofstream up( "unmatched_perf", ios::trunc );
  ofstream corr( "matches", ios::trunc );

  ofstream extra( "extra", ios::trunc );
  ofstream missing( "missing", ios::trunc );

  ScorePart *spart = doc.getScorePart(partNo);
  PerfPart *ppart = spart->getPerfPart();
  std::list<SNote*>* score = spart->getNotes();
  std::list<PNote*>*  perf = ppart->getNotes();
  oldIt = score->begin();

  // iterate through perf notes
  for( list<PNote*>::iterator pit = perf->begin();
       pit != perf->end(); pit++ ){

    // If note has a match print relative & absolute times to file
    // remove score note from list

    PNote * pnote(*pit);
    
    string alignID( pnote->getAlignedNoteID() );
    if( alignID != "" ){
      DOMElement *e = ppart->getElement()->getOwnerDocument()->getElementById( XS(alignID.c_str()) );
      SNote snote(&doc, e);
      if( !evalByVoice || (snote.getStaff() == staff) ){
	m << snote.getStartTime() << ", "
	  << pnote->getOnset() << ", "
	  << endl;
	removeNoteFromList( snote, score );
      }

      // Print ids of matched notes to matches file      
      // spaces and the s required here for evaluation
      corr << pnote->getID() << " s" << alignID << " " << endl; 
    }

    // If note doesnt have a match, print details to 
    // file of unmatched perf notes unless
    else {
      if(LOG_LISTS)cout << "No match\n";cout.flush();
      up << pnote->getOnset() << ", "
	 << pnote->getID().erase( 0,5 ) << ", "
	 << pnote->getMidi()
	 << endl;

      extra << pnote->getID().erase( 0,5 ) << endl;
    }
  }

  // Print remaining score notes in list to the file
  // Skip notes which are continuations of ties
  for( list<SNote*>::iterator sit = score->begin();
       sit != score->end(); sit++ ){
    SNote *snote = *sit;

    string alignID = snote->getID().erase( 0,4 );

    if( evalByVoice && snote->getStaff() != staff )
      continue;
    
    if( snote->isTied() )
      if( snote->getTiedType() != "start" )
	continue;
    
    
    
    us << snote->getStartTime() << ", "
       << alignID << ", "
       << tuning->getMidiNumber( snote->getPitch() )
       << endl;

    missing << alignID << endl;
  }
  m.close();
  us.close();
  up.close();
  corr.close();
  delete(spart);
  delete(ppart);

  cout << "Lists made\n";
}

/*
  We'll be incrementing through performance, can do some groovy 
  search alternating between a backwards and forwards iterator 
  from the last found position. Or could just search from begining 
  or last position every time...
*/
void removeNoteFromList( SNote &note, std::list<SNote*>* score ){
  string id = note.getID();
  if( oldIt == score->end() ) oldIt--;
  fit = oldIt; 
  bit = oldIt;

  if(LOG_LISTS)cout << "id " << id << endl;
  
  SNote *sn= *oldIt;
  if(LOG_LISTS)cout << "old it at " << sn->getID() << endl;


  while( fit != score->end() || bit != score->begin() ){

    if( fit != score->end() ){
      SNote *s(*fit);
      if( s->getID() == id ){
	if(LOG_LISTS)cout << "fit " << s->getID() << endl;
	if(LOG_LISTS)cout << "Match found " << id << " " << s->getID() << endl;
	oldIt = score->erase( fit );
	return;
      }
      fit++;
    }

    if( bit != score->begin() ){
      bit--;
      SNote *s(*bit);
      if(LOG_LISTS)cout << "bit " << s->getID() << endl;
      if( s->getID() == id ){
	if(LOG_LISTS)cout << "Match found " << id << " " << s->getID() << endl;
	oldIt = score->erase(bit);
	if(LOG_LISTS)cout << "Erased\n";cout.flush();
	return;
      }
    }
  }
  if(LOG_LISTS)cout << "No match found!\n";
}


// Old unalign - based on scilab stdev discrimination
void unalign(){
  
  cout << "Using stddev unalign method\n";

  ScorePart *spart = doc.getScorePart(partNo);
  PerfPart *ppart = spart->getPerfPart();
  list<PNote*>* perf = ppart->getNotes();

  // get times to unalign   
  ifstream in("remove");
  cout << "Opening remove file ... " << endl;
  while( !in.eof() ){
    //find notes with start time & unalign
    int rtime;
    char c;
    in >> rtime;
    in >> c;
    cout << "Time " << rtime << endl;
    // incredibly inefficient, but easy
    list<PNote*>::iterator pit = perf->begin();
    while( pit != perf->end() ){

      PNote *p = *pit;
      string id = p->getAlignedNoteID();
      if( !id.empty() ){
	SNote snote( &doc, doc.getElementById(id) );
	//cout << "SNote " << snote.getStartTime() << endl;
	if( snote.getStartTime() == rtime ){
	  p->unalign();
	  cout << "Unaligning " << p->getID() << " from " << snote.getID() << endl;
	  cout << snote.getStartTime() << " " << p->getOnset() << endl;
	}
      }
      pit++;
    }


  }
  in.close();
  delete(spart);
  delete(ppart);
  
}
  


