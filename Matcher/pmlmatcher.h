#ifndef PML_MATCHER_H
#define PML_MATCHER_H

#include "../pmllib/pmlelements.h"
#include "../pmllib/pmldocument.h"

#include "pmlmatcher.h"
#include <string>
#include <vector>


typedef std::vector<std::string> SVec;

class Chord : public std::vector<Note*> {
 public:
  bool containsExact( Note * );
};

//typedef std::vector<Note*> Chord;
typedef std::vector<Note*>::iterator ChordIt;
typedef std::vector<Chord*> ChordList;
typedef std::vector<Chord*>::iterator ChordListIt;
typedef std::pair<int,int> XY;


// returns average time of performance notes
double getAvgTime( Chord *pc );


class Corr{

 public:
  typedef  enum { CORRECT, PARTIAL, WRONG, MISSING, EXTRA, NONE } Status;

  Status status;
  Chord* sChord;
  Chord* pChord;
  XY xy;

  Corr( Chord* s, Chord* p, XY xy, Status status );


};


class PMLMatcher {

 public:
  PMLMatcher( PMLDocument *doc, int winsize, double threshold, int winshift, double initClust );
  PMLMatcher();
  virtual ~PMLMatcher(){}

  void setPart( ScorePart *part );
  void match();                       // match whole performance
  void printMatches();
  void printGrid();
  void printGridCurses();
  void alignMatches( bool justPrint = false ); // aligns matches from currPos onwards

  bool getNewPosition();

  void interpolate();

 protected:
  PMLDocument *_doc;
  Tuning *_tuning;

  ScorePart *_scorePart;
  PerfPart *_perfPart;
  std::list<PNote*> *_perfNotes;
  PerfIterator _perfNotesIt;

  ChordList _score;   // contains entire list
  ChordList _perf;
  ChordListIt _scoreIt; // position of start of window
  ChordListIt _perfIt;  // position of start of window
  ChordListIt _scoreCItEnd; // position of end of window
  ChordListIt _perfCItEnd;  // (this not used)

  int _perfBuffSize, _scoreBuffSize; // size of current window
  ChordListIt _scoreBuffIt; // iterator for evaluation
  ChordListIt _perfBuffIt;  // 

  std::list<Corr> _matches; 
  std::list<Corr>::iterator _matchIt;

  XY _currPos;
  XY _oldPos;

  int _winsize;        // size of window
  int _winShift;       // maximum gap between windows
  int _windowSize;     // maximum size of window
  int _gridXSize;      // size of grid
  int _gridYSize;
  double *_grid;       // grid

  double _epsilon;     // current chord threshold
  double _clusteringThreshold; // Fraction used to calculate epsilon
  int _matchedNotes;

  void getPerfClusters( int no ); // get at most no more
  void getScoreClusters( int no );    // clusters from pos onwards
  void getScoreChordList();
  void clearAlignTags();// called in setPart clears previous alignment

  // GRID STUFF
  double& grid( int x, int y );
  void initGrid();
  void populateGrid();    // must populate grid according to size 

  //Evaluation
  void evaluatePath();
  void evaluateCorr( Corr &corr, bool align);
  double compare( Chord *score, Chord *perf );

  void getTempo(std::list<Corr>::iterator);

  virtual void correct( int x, int y );
  virtual void wrong( int x, int y );
  virtual void partial( int x, int y );
  virtual void extra( int x, int y );
  virtual void missing( int x, int y );
};

#endif 
