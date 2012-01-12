#include "pmlmatcher.h"
#include "matcher.h"
#include "../pmllib/pmldocument.h"
#include "../pmllib/pmlelements.h"
#include "../pmllib/pitch.h"

#include <fstream>
#include <iostream>
#include <curses.h>
#include <math.h>
#include "linux/types.h"

//needed for tempo stuff




using namespace std;

//ofstream fout("matchlog", ios::trunc );

#define LOG cout             // log destination

#define LOGCOMPARE 0      // print out comparison info (requires LOGCorr)
#define LOGGRID 0         // print the the grid
#define LOGCLUST 1        // log clustering
#define LOGCORR true         // log correspondences
#define EPSLOG 1            // log epsilon calculations

#define DYNAMIC_CLUSTERING 1 // use dynamic clustering threshold

/* This values determines the threshold for clustering performance notes.
   The minimum IOI in the current score window is found and converted into
   absolute time using the estimated instantaneous tempo. This interval is 
   divided by the number below to obtain the threshold epsilon. */


ostream& operator<<( ostream &os,XY xy ){
  os << xy.first << "," << xy.second;
}

Corr::Corr( Chord* s, Chord* p, XY xy, Status status ){
  sChord=s;
  pChord=p;
  this->xy=xy;
  this->status = status;
}



bool Chord::containsExact( Note *note ){

  for( ChordIt it = begin();
       it != end(); it++ ){

    if( (*it)->getID() == note->getID() )      
      return true;
  }
  return false;
}


PMLMatcher::PMLMatcher( PMLDocument *doc, int winsize, double threshold, int winShift, double initClust ){
  _winShift = winShift;
  _windowSize = winsize;
  _doc = doc;
  _scorePart = NULL;
  _perfPart = NULL;
  _clusteringThreshold = threshold;  
  _tuning = _doc->getTuning();
  _epsilon = initClust;

}



void PMLMatcher::setPart( ScorePart *part ){
  
  int oldstarttime=0;

  if( _scorePart ) delete _scorePart;
  if( _perfPart ) delete _perfPart;

  _scorePart = part;
  _perfPart = _scorePart->getPerfPart();
  LOG << "Got perf part\n";

  if( !_perfPart->noEvents() )
    LOG << "Perf has no events!\n";

  getScoreChordList();

  clearAlignTags();

}


void PMLMatcher::clearAlignTags(){

  for(PerfIterator pit = _perfPart->getNotes()->begin();
      pit != _perfPart->getNotes()->end();
      pit++ ){

    (*pit)->unalign();

  }


}


// Clustering -------------------------------------------
void PMLMatcher::getScoreChordList(){

  LOG << "getScoreChords\n";

  //clear list
  while( _score.end() != _score.begin() ){
    delete *( --_score.end() ); // delete last element
    _score.erase( --_score.end() );
  }

  _score.push_back( new Chord );

  //Score...  
  list<SNote*> *snotes = _scorePart->getNotes();

  SNote s( **snotes->begin() );
  int lastStartTime = s.getStartTime(); //set to first start time

  for( ScoreIterator sit = snotes->begin();
       sit != snotes->end(); sit++ ){

    SNote note( **sit );

    int startTime = note.getStartTime();

    //if note is coninuation or end of a tie, discard this note
    if( note.isTied() ){
      string tie = note.getTiedType();
      string start="start";
      if( tie != start ){
	LOG << "Note " << note.getID() << " is coninuation of tie\n";
	continue;
      }
    }

    // if note does not start at same time,
    // create new chord list item
    if( startTime != lastStartTime ){
      _score.push_back( new Chord );
       if(LOGCLUST)LOG << startTime << " ----------" << endl;
    }
    if(LOGCLUST)LOG << "Note " << note.getPitchString()
		    << " " << _tuning->getMidiNumber( note.getPitch() )
		    << " " << note.getID() << endl;

    //add note to last chord item
    ChordListIt cl = _score.end();
    cl--;

    (*cl)->push_back( new SNote(note) );

    lastStartTime = startTime;
  }

  LOG << "There are " << _score.size() << " chords in the score\n";

}

void PMLMatcher::getScoreClusters( int no ){

  // Make the end iterator point to the begining
  // and increment 'no' times from that point

  LOG << "\nGetting " << no << " score clusters\n";LOG.flush();
  _scoreBuffSize = 0;
  _scoreCItEnd = _scoreIt;

  while( no-- && _scoreCItEnd != _score.end() ){
    _scoreCItEnd++;
    _scoreBuffSize++;
  }

  if( _scoreCItEnd == _score.end() )
    LOG << "\n\nReached End of Score!!!!!!!\n";

  ChordListIt end = _scoreCItEnd;
  end--;

  Chord *c1 = *_scoreIt;
  SNote n1(_doc,c1->at(0)->getElement());
  Chord *c2 = *end;
  SNote n2(_doc,c2->at(0)->getElement());
  LOG << "Score Cluster goes from" 
      << n1.getPitchString() << " "
      << n1.getStartTime() 
      << "\tto "
      << n2.getPitchString() << " "
      << n2.getStartTime() << 
      endl;


}


void PMLMatcher::getPerfClusters( int no ){ 
  LOG << "\nGetting perf clusters\n";
  LOG << "Clustering threshold Epsilon is " << _epsilon << " seconds" << endl;
  _perf.push_back( new Chord );
  _perfBuffSize = 1;

  PNote* note = *_perfNotesIt;
  double oldOnset = note->getOnset();

  if(LOGCLUST)
    LOG << "\n\tPNote Onset\tIOI\tMIDI/Frequency\n";
  
  while( _perfNotesIt != _perfNotes->end() && no ){
    note = *_perfNotesIt;
    double newOnset = note->getOnset();
    double diff = newOnset - oldOnset;


    // If diff exceeds threshold, finish chord
    // If total number of chords has been reached, return
    // otherwise, append a new chord to the list
    if( diff > _epsilon ){
      if(LOGCLUST)LOG << "last chord size: " << _perf[_perf.size()-1]->size() << endl;

      if( --no > 0 ){
	_perf.push_back( new Chord );
	_perfBuffSize++;
       }
      else
	return;
    }

    double freq;
    MidiNo midi;
    Pitch p('c',3);
    if( note->hasMidi() ){
      midi = note->getMidi();
      p = _tuning->getPitch(midi);
    }
    else{
      freq = note->getFreq();
      p = _tuning->getPitch( freq );
    }

    cout << p << endl;

    if(LOGCLUST){

      LOG << "\tPNote " << newOnset << "\t" << diff << "\t";
      if( !note->hasMidi() ) LOG << freq << " ";

      LOG  << _tuning->getMidiNumber(p) << " "
	   << note->getID() << " "
	   << p
	   << endl;
    }   
    _perf[_perf.size()-1]->push_back( note );
    
    oldOnset = newOnset;
    _perfNotesIt++;
  }

  //If first time, must set perfIt to begining
  if( _currPos.second == 0 ) _perfIt = _perf.begin();
}

// GRID ------------------------------------------
void PMLMatcher::initGrid(){
  //allocate grid mem
  _gridXSize = _windowSize+2;
  _gridYSize = _windowSize+2;
  _grid = (double*)calloc( sizeof(double), _gridXSize * _gridYSize );
}

void PMLMatcher::populateGrid(){

  LOG << "populateGrid "  << _scoreBuffSize << " x " << _perfBuffSize << endl;

  ChordListIt pit = _perf.end();
  pit--;
  ChordListIt sit;

  // If size of analysis window gets smaller,
  // must ensure border of grid is 0
  memset( _grid, 0, _gridXSize*_gridYSize*sizeof(double) );
  for( int y=_perfBuffSize; y>0; y--, pit-- ){
    sit = _scoreCItEnd;
    sit--;
    for( int x=_scoreBuffSize; x>0; x--, sit-- ){

      //cout << x << " " << y << endl;
      float goodness = compare( *sit, *pit );

      if( goodness+grid(x+1,y+1) > grid(x+1, y) &&
	  goodness+grid(x+1,y+1) > grid(x,y+1)    ){
	grid(x,y) = grid( x+1,y+1 ) + goodness;
      }
      else if( grid(x+1, y) > grid(x,y+1) )
	grid(x,y) = grid( x+1,y   );
      else
	grid(x,y) = grid( x  ,y+1 );
      
    }
  }
  LOG << "Done." << endl;
  if(LOGGRID)printGrid();
}

void PMLMatcher::printGrid(){

  //ofstream fout("xout", ios::trunc);

  LOG.precision(4);
  LOG << "Printing grid...\n";
  //  for( int y=0; y<_perfBuffSize; y++ ){
  //  for( int x=0; x<_scoreBuffSize; x++ ){
    for( int y=0; y<_gridYSize; y++ ){
      for( int x=0; x<_gridXSize; x++ ){
	LOG << grid(x,y) << "\t";
      //      LOG << grid(x+1,y+1) << "\t";
    }
    LOG << "\n";
  }
  
}

double& PMLMatcher::grid( int x, int y ){
  return _grid[ y*_gridXSize + x ];
}

// Evaluate -------------------------------------------
void PMLMatcher::match(){

  _currPos = XY(0,0);
  _oldPos = XY(0,0);
  _matches.clear();

  _perfNotes = _perfPart->getNotes();
  _perfNotesIt = _perfNotes->begin();
  _scoreIt = _score.begin();
  _matchIt = _matches.begin();

  initGrid();

  do{ //continue while excerpt size is > 0
    LOG << "\n\n NEW WINDOW___________________________________";
    LOG << "\n\n\nCurrent position is " << _currPos.first << "x" << _currPos.second << endl;
    LOG << "Old position was " << _oldPos.first << "x" << _oldPos.second << endl;

    //Get score clusters, calculate epsilon, then get new perf clusters
    getScoreClusters( _windowSize  );

    if( _currPos != XY(0,0) ) getTempo(_matchIt);

    getPerfClusters( _windowSize );
    _perfIt = _perf.begin();

    LOG << "Got score and perf excerpts: " 
	<< _scoreBuffSize <<  " " 
	<< _perfBuffSize << endl;

    populateGrid();                                

    cout << "Got grid\n"; cout.flush();
    //remove matches from current match position to end
    LOG << "Matches size before " << _matches.size();
    _matches.erase( _matchIt, _matches.end() );
    LOG << " \t and after " << _matches.size() << endl;

    LOG << "Evaluating path\n";
    // This will add matches into match vector
    evaluatePath();                               

    // If we are at end of performance, break the loop
    if( _scoreBuffSize != _windowSize || _perfBuffSize != _windowSize ){
      LOG << "\n\nComparison ended\n"
	  << _matches.size() << " total matches\n"
	  << "Window size  " << _windowSize << endl
	  << "Final grid count " << _scoreBuffSize << "x" << _perfBuffSize << endl;
      break;
    }


    // If failed to get new position, matcher is lost
    // end matching process    
    if( !getNewPosition() ){
      cout << "\nHELP ME I'M MELTING!: Cannot advance matching window.\n"
	   << "You must check the performance at this point.\n"
	   << "You may wish to increase the window size & window shift.\n"
	   << "The window shift must be longer than any consecutive run of\n"
	   << "unmatched correspondences. The window size must be longer\n"
	   << "than the window shift to prevent the matcher losing the plot.\n\n"
	   << "Or, something is just seriously wrong in the performance.\n"
	   << "Or.., something is seriously wrong in the file\n"
	   << "Good luck :)";
      return;
    }

    // must move perfNotesIt back to where perfIt points
    // and reevaluate chords using new tempo
    if( _perfIt == _perf.end() ) LOG << "\n\nEND!!\n";LOG.flush();

    Chord *c = *_perfIt;
    Note *n= c->at(0);
    LOG << "perfIt is at " << n->getID() << endl;LOG.flush();

    //Move to begining of current cluster
    if( _perfNotesIt == _perfNotes->end() ) _perfNotesIt--;
    while( !c->containsExact(*_perfNotesIt) ) _perfNotesIt--; 
    while( c->containsExact(*_perfNotesIt) ) _perfNotesIt--;
    _perfNotesIt++;
    
    n= *_perfNotesIt;
    LOG << "_perfNotesIt is now at " << n->getID() << endl;LOG.flush();

    //erase from perfIt to end of list
    LOG << "Erasing perf clusters\n";LOG.flush();
    _perf.clear();

  }while( _scoreBuffSize == _windowSize && _perfBuffSize == _windowSize );
  
}

// Decrement match iterator until position is < e,e
// and match is correct/partial
// Returns false if no new position found
bool PMLMatcher::getNewPosition(){
  _matchIt = _matches.end();
  while( _matchIt != _matches.begin() && 
	 _matchIt->xy.first > _currPos.first+_winShift &&    // iterate back until position
	 _matchIt->xy.second > _currPos.second+_winShift ||  // is < e,e and partial/correct
	 (_matchIt->status != Corr::CORRECT && _matchIt->status != Corr::PARTIAL) ) { 
    _matchIt--;
  }

  LOG << "New position is " << _matchIt->xy.first << "x" << _matchIt->xy.second
      << "\tstatus is " << _matchIt->status << endl
      << "CurrentPosition " << _currPos << endl;  
  _oldPos = _currPos;

  // Inmcrement score and performance iterators
  // until they point to new position
  while( _currPos.first != _matchIt->xy.first && _scoreIt != _score.end() ){
    _scoreIt++;
    _currPos.first++;
  }
  while( _currPos.second != _matchIt->xy.second && _perfIt != _perf.end() ){
    _perfIt++;
    _currPos.second++;
  }

  if( _currPos.first == _oldPos.first &&
      _currPos.second == _oldPos.second  ){

    cerr << "\t==============\n"
	 << "\tFATAL ERROR!!!\n"
	 << "\t==============\n"
	 << "\n\tMatcher is lost!!\n";

    return false;

  }

  if( _scoreIt == _score.end() )
    LOG << "\nERROR end of score reached!\n\n";

  Chord *c = *_scoreIt;
  SNote n(_doc,c->at(0)->getElement());
  LOG << "_scoreIt points to" 
      << n.getPitchString() << " "
      << n.getStartTime() << endl;

  return true;

}


void PMLMatcher::evaluateCorr( Corr &corr, bool align ){
  

  LOG << endl;
  switch( corr.status ){
    
  case Corr::CORRECT:
    LOG << "correct";
    break;
  case Corr::WRONG:
    LOG << "wrong";
    break;
  case Corr::PARTIAL:
    LOG << "partial";
    break;
  case Corr::EXTRA:
    LOG << "extra";
    break;
  case Corr::MISSING:
    LOG << "missing";
  }

  LOG << "\tXY " << corr.xy.first << " " << corr.xy.second;

  SNote *s = static_cast<SNote*>(corr.sChord->at(0));

  LOG << "\tTime\t"
      << "\t" << s->getStartTime()
      << "\t" << getAvgTime(corr.pChord)
      << endl;
  
  if( corr.status == Corr::EXTRA ){
    
    for( ChordIt p=corr.pChord->begin(); p!=corr.pChord->end(); p++ ){
      // Get midi number      
      PNote *pnote = static_cast<PNote*>(*p);
      string id = pnote->getID();
      MidiNo pmid = pnote->hasMidi() ? pnote->getMidi() 
	: _tuning->getMidiNumber( pnote->getFreq() );

      LOG << "\t\t" << _tuning->getPitch( pmid )
	  << "\t" << pmid 
	  << "\t" << pnote->getID()
	  << "\t" << pnote->getOnset() 
	  << endl;
    }
  }
  else if( corr.status == Corr::MISSING ){
    for( ChordIt s=corr.sChord->begin(); s!=corr.sChord->end(); s++ ){
      SNote *snote = static_cast<SNote*>(*s);
      string id = snote->getID();
      //Get midi number and print pitch string
      MidiNo smid = _tuning->getMidiNumber( snote->getPitch() );
      LOG << "\t" << snote->getID()
	  << "\t" << snote->getPitchString() 
	  << "\t" << smid 
	  << endl;
    }
  }      
  else{
    // Make a copy of the performance chord. Matched notes are
    // removed so that remaining notes can be printed as extra
    Chord pChord( *(corr.pChord) );

    // For each note in score chord
    for( ChordIt s=corr.sChord->begin(); s!=corr.sChord->end(); s++ ){
      
      SNote *snote = static_cast<SNote*>(*s);
      
      //Get midi number and print pitch string
      MidiNo smid = _tuning->getMidiNumber( snote->getPitch() );
      LOG << "\t" << snote->getID()
	  << "\t" << snote->getPitchString()
	  << "\t" << smid;
      
      // For each note in performance chord
      ChordIt p=pChord.begin(); 
      while( p!=pChord.end() ){
	
	// Get midi number      
	PNote *pnote = static_cast<PNote*>(*p);
	MidiNo pmid = pnote->hasMidi() ? pnote->getMidi() 
	  : _tuning->getMidiNumber( pnote->getFreq() );
	
	// if matches score note
	if( smid == pmid ){
	  _matchedNotes++;
	  // align in PML doc (unless no alignment is true)
	  if( align )
	    pnote->align( snote, true );

	  // print alongside score note
	  LOG << "\t" << pnote->getID(); 
	  pChord.erase( p );
	  break;
	}
	else{
	  p++;
	}
      }
      LOG << endl;
    }

    

  
    //Print remaining performance notes
    for( ChordIt p=pChord.begin(); p!=pChord.end(); p++ ){
      PNote *pnote = static_cast<PNote*>(*p);
      MidiNo pmid = pnote->hasMidi() ? pnote->getMidi() 
	: _tuning->getMidiNumber( pnote->getFreq() );
      LOG << "\t\t\t" << pmid 
	  << "\t" << pnote->getID()
	  << "\t" << _tuning->getPitch( pmid )
	  << endl;
    }
  }
  
}

void PMLMatcher::alignMatches( bool justPrint ){

  int correct=0, wrong=0, partial=0, missing=0, extra=0;
  _matchedNotes=0;

  for( list<Corr>::iterator mit = _matches.begin(); mit != _matches.end(); mit++ ){
    evaluateCorr( *mit, !justPrint );
    
    switch( mit->status ){
      
    case Corr::CORRECT:
      correct++;
      break;
    case Corr::WRONG:
      wrong++;
      break;
    case Corr::PARTIAL:
      partial++;
      break;
    case Corr::EXTRA:
      extra++;
      break;
    case Corr::MISSING:
      missing++;
    }

  }


  LOG << "Cluster Totals:\n"
      << "\tcorrect " << correct << endl
      << "\twrong " << wrong << endl
      << "\tpartial " << partial << endl
      << "\tmissing " << missing << endl
      << "\textra " << extra << endl;
  
  LOG << "Total matched notes: " << _matchedNotes << endl;
  
}

double PMLMatcher::compare( Chord *score, Chord *perf ){

  if( LOGCOMPARE ){
    Corr corr(score, perf, XY(-1,-1), Corr::NONE);
    evaluateCorr(corr,false);
  }


  // take two vectors,
  double goodness = 0; // measures no of matches
  int s_size = score->size();
  int p_size = perf->size();
  int total = s_size>p_size ? s_size : p_size;

  for( int sit = 0; sit< s_size; sit++ )
    for( int pit = 0; pit< p_size; pit++ ){

      // compare midi numbers in tuning
      // this avoids enharmonic inequalities

      MidiNo sMid, pMid;
      SNote *snote = static_cast<SNote*>(score->at(sit));
      Pitch sp = snote->getPitch();
      sMid = _tuning->getMidiNumber( sp );
      if(LOGCOMPARE)LOG << "S " << sMid;

      // If midi number is explicitly stated in perf data
      // use that before calculating from avg freq
      PNote *pnote = static_cast<PNote*>(perf->at(pit) );
      if( pnote->hasMidi() ){
	pMid = pnote->getMidi();
      }
      else{
	pMid = _tuning->getMidiNumber( pnote->getFreq() );
      }
      
      if(LOGCOMPARE)LOG << " P " << pMid << endl;

      if( sMid == pMid )
	goodness++;
    }
  // divide number of matches by the maximum 
  // number of simultaneous notes
  goodness /= (double)total;

  if(LOGCOMPARE)LOG << "Goodness " << goodness << endl;
  return goodness;

}

void PMLMatcher::evaluatePath(){
  
  int x=0, y=0;

  LOG << "Evaluating performance\n";
  _scoreBuffIt = _scoreIt;
  _perfBuffIt = _perf.begin();

  while( x != _scoreBuffSize && y != _perfBuffSize ){

    //if(LOGCLUST)LOG << "Position: " << x << " " << y << "\n";
    double goodness = compare( *_scoreBuffIt, *_perfBuffIt  );
    if(  goodness == 1 ){
      correct(x,y);
      x++; _scoreBuffIt++;
      y++; _perfBuffIt++;      
    }
    else{
      double a = grid(x+2,y+1);   //
      double b = grid(x+2,y+2);   //  H  A
      double c = grid(x+1,y+2);   //
      double h = grid(x+1,y+1);   //  C  B

      //LOG << h << "\t" << a << endl
      //      << c << "\t" << b << endl; 

      if( a==b && a==b && a==c  ){      // WRONG or PARTIAL

	if( h > b )
	  partial( x, y );
	else
	  wrong(x,y);
	x++; _scoreBuffIt++;
	y++; _perfBuffIt++;
      }
      else if( a>h && h==b ) { //WRONG
	wrong(x,y);
	x++; _scoreBuffIt++;
	y++; _perfBuffIt++;
      }
      else if( a==b && h > c ){       //PARTIAL MATCH NOTE: h > c > a == b
	partial(x,y);
	x++; _scoreBuffIt++;
	y++; _perfBuffIt++;

      }
      else if( c==b && h > a ){ //PARTIAL MATCH h > a > c==b
	partial(x,y);
	x++; _scoreBuffIt++;
	y++; _perfBuffIt++;

      }
      else if( (h - b) == goodness ){ //PARTIAL MATCH (must come before a>b & a>c )
	partial(x,y);
	x++; _scoreBuffIt++;
	y++; _perfBuffIt++;
      }
      else if( a>b && a>c ) { //MISSING
	missing(x,y);
	x++; _scoreBuffIt++;
      }
      else if (c>a && c>b) { // EXTRA
	extra(x,y);
	y++; _perfBuffIt++;
      }
      else if( (a == c) && a > b ){ //AMBIGUOUS CASE

	LOG << "AMBIGUOUS ERROR CONDITION, WOW!!!!!!!!!!\n";

	// Ambiguous error can be either extra or 
	// missing
	// Check first in case a partial match is 
	// suitable 
	if( (h - b) == goodness ){
	  partial(x,y);
	  x++; _scoreBuffIt++;
	  y++; _perfBuffIt++;
	}
	else{
	extra(x,y);
	y++; _perfBuffIt++;
      }



      }
      else{
	LOG << "Matcher::findGridPath  Shouldn't get here!\n";
      }
    }   
  }
} 
    


// etc ------------------------------------
void PMLMatcher::correct( int x, int y ){
  Chord *sc( *_scoreBuffIt );
  Chord *pc( *_perfBuffIt );
  _matches.push_back( Corr(sc,pc,XY(x+_currPos.first,
					   y+_currPos.second),Corr::CORRECT) );
  if(LOGCORR)LOG << "\nCorrect\n";
  evaluateCorr( *--_matches.end(), false );
}

void PMLMatcher::partial( int x, int y ){
  Chord *sc( *_scoreBuffIt );
  Chord *pc( *_perfBuffIt );
  _matches.push_back( Corr(sc,pc,XY(x+_currPos.first,
					   y+_currPos.second),Corr::PARTIAL) );
  if(LOGCORR)LOG << "\nPartial\n";
  evaluateCorr( *--_matches.end(), false );
}

void PMLMatcher::wrong( int x, int y ){
  Chord *sc( *_scoreBuffIt );
  Chord *pc( *_perfBuffIt );
  _matches.push_back( Corr(sc,pc,XY(x+_currPos.first,y+_currPos.second),Corr::WRONG) );
  if(LOGCORR)LOG << "\nWrong\n";
}
void PMLMatcher::extra( int x, int y  ){
  if(LOGCORR)LOG << "\nExtra\n";
  Chord *sc( *_scoreBuffIt );
  Chord *pc( *_perfBuffIt );
  _matches.push_back( Corr(sc,pc,XY(x+_currPos.first,y+_currPos.second),Corr::EXTRA) );
}
void PMLMatcher::missing( int x, int y  ){
  if(LOGCORR)LOG << "\nMissing\n";
  Chord *sc( *_scoreBuffIt );
  Chord *pc( *_perfBuffIt );
  _matches.push_back( Corr(sc,pc,XY(x+_currPos.first,y+_currPos.second),Corr::MISSING) );
}


// Tempo -----------------------------

void PMLMatcher::getTempo(list<Corr>::iterator currCorr){

  if(EPSLOG) LOG << "\nTEMPO" << endl;
  list<Corr>::iterator start, end, it;

  Chord *sc = currCorr->sChord;
  int tTime = ((SNote*)sc->at(0))->getStartTime();
  int divs = _scorePart->getDivisions(); //divs per beat
  Chord *c;
  int sTime, lastSTime;
  double lastPTime;    
  double avgbpm=0;
  int count=0; // counts no of bpm estimates

  if(EPSLOG) LOG << "Target time = " << tTime << " divs " << divs << endl;

  lastSTime=tTime;
  lastPTime = getAvgTime( currCorr->pChord );
  start = currCorr; 
  do{
    //Iterate back, skipping extra/missing/wrong correspondences
    do{
      start--;
    }while( start != _matches.begin() && 
	    ( start->status != Corr::CORRECT &&
	      start->status != Corr::PARTIAL ) );
    
    c = start->sChord;
    SNote *sn = (SNote*)c->at(0);
    sTime = sn->getStartTime();
    double pTime = getAvgTime( start->pChord );

    if(EPSLOG) LOG << "s " << sTime << "\tp " << pTime << endl;
    
    int sDiff = abs(lastSTime-sTime);
    double pDiff = fabs(lastPTime-pTime);
    double bpm = ((double)sDiff/(double)divs)/(pDiff/60.0);
    avgbpm += bpm;
    count++;
    
    if(EPSLOG) LOG << "Diff =\t" << sDiff << "\t" << pDiff << "\t " << bpm << " bpm " << endl;
    
    lastPTime = pTime;
    lastSTime = sTime;
  }while( sTime > tTime-divs );

  lastSTime=tTime;
  lastPTime = getAvgTime( currCorr->pChord );
  end = currCorr; 
  do{

    //Iterate forwards, skipping extra/missing/wrong correspondences
    do{
      end++;
    }while( end != _matches.end() && 
	   ( end->status != Corr::CORRECT &&
	     end->status != Corr::PARTIAL ) );
    
    if( end != _matches.end() ){

      double pTime = getAvgTime( end->pChord );

      c = end->sChord;
      SNote *sn = (SNote*)c->at(0);
      sTime = sn->getStartTime();
      
      if(EPSLOG) LOG << "e " << sTime << "\tp = " << pTime << endl;
      
      int sDiff = abs(sTime-lastSTime);
      double pDiff = fabs(pTime-lastPTime);
      double bpm = ((double)sDiff/(double)divs)/(pDiff/60.0);
      avgbpm += bpm;
      count++;

      if(EPSLOG) LOG << "Diff =\t" << sDiff << "\t" << pDiff << "\t " << bpm << " bpm " << endl;
      
      lastSTime = sTime;
      lastPTime = pTime;
    }

  }while( end != _matches.end() && sTime < tTime+divs ); 

  lastSTime=-1;
  int minIOI=INT_MAX;
  for( ChordListIt sit = _scoreIt;  
       sit != _scoreCItEnd; sit++ ){

    Chord *c = *sit;
    SNote *snote = static_cast<SNote*>(c->at(0));
    int sTime = snote->getStartTime();

    if( lastSTime > 0 ){
      LOG << "time " << sTime << " last time " << lastSTime << endl;
      int ioi = sTime-lastSTime;
      //if(EPSLOG) LOG << "IOI " << ioi << endl;
      if( ioi < minIOI ) minIOI  = ioi;
    }
    lastSTime = sTime;
  }
  if(EPSLOG) LOG << "Min IOI " << minIOI << " = " << minIOI/(double)divs << " beats"<< endl;
   
  avgbpm /= count;
  double beatLength =60.0/avgbpm;  
  double minIOIinBeats = (double)minIOI/(double)divs;

  if( DYNAMIC_CLUSTERING )
    _epsilon = beatLength*minIOIinBeats*_clusteringThreshold;

  if(EPSLOG) LOG << "Avg bpm " << avgbpm << endl;
  if(EPSLOG) LOG << "Beat length " << beatLength << endl;
  LOG << "Epsilon: " << _epsilon << " secs" << endl;

}

// gets avg time of a performance chord
double getAvgTime( Chord *pc ){

  double total=0;
  for( ChordIt pcit = pc->begin();
       pcit != pc->end(); pcit++ ){
    PNote *note = static_cast<PNote*>(*pcit);
    total += note->getOnset();
  }
  return total/(double)pc->size();
}




void PMLMatcher::interpolate(){

  ofstream out("matched");
  for( list<Corr>::iterator mit = _matches.begin(); mit != _matches.end(); mit++ ){

    Corr corr = *mit;
    if( corr.status == Corr::CORRECT || corr.status == Corr::PARTIAL ){
      double ptime = getAvgTime(corr.pChord);
      int stime = static_cast<SNote*>(corr.sChord->at(0))->getStartTime();
      
      out << stime << ", " << ptime << endl;
      cout << "alright! " << stime << ", " << ptime << endl;
    }
  }


}
