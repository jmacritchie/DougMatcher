#ifndef PERFORMANCE_ITERATOR_H
#define PERFORMANCE_ITERATOR_H

#include "pitch.h"
#include "tuning.h"
#include "pmldocument.h"
#include "pmlbase.h"

#include <iostream>
#include <list>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>



typedef std::list<PNote*>::iterator PerfIterator;
//typedef std::list<SNote*>::iterator ScoreIterator;


class PerformanceIterator{

 public:
  enum Status { CORRECT, WRONG, EXTRA, MISSING, ERROR };

  PerformanceIterator( ScorePart *score, PerfPart *perf );
  PerformanceIterator( PerformanceIterator &original );

  PerformanceIterator& operator=( PerformanceIterator &original );
  PerformanceIterator& operator++();
  PerformanceIterator& operator--();

  bool hasNext();

  SNote* getScoreNote();
  PNote* getPerfNote();

  ScoreIterator getScoreIterator(){ return m_sit; }
  PerfIterator getPerfIterator(){ return m_pit; }

  ScorePart *getScorePart(){ return m_score; } 
  PerfPart *getPerfPart(){ return m_perf; }


  void removePNote();


  std::string getStatusString();
  Status getStatus();

  void printStatus();


 protected:

  void checkStatus();

  Status m_status;

  ScorePart *m_score;
  PerfPart *m_perf;

  ScoreIterator m_sit;
  PerfIterator m_pit;

};


#endif