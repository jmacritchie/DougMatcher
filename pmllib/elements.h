


class ScorePart{

  ScorePart( DOMElement *element );

  const std::vector<ScoreNote*> getNotes();

}

class PerfPart{

  /// Creates part if none exists, or 
  PerfPart( int part );
  PerfPart( DOMElement *element );

  std::string getName();

  ScorePart *getScorePart();

  const std::vector<PerfNote*> getNotes( int part );


}


class Pitch{

 public:
  Pitch( char name, int oct, int acc );
  Pitch( DOMElement *pitch );

  std::string getStringFromPitch( bool withOctave=true);
  DOMElement* getElementForPitch( char name, int oct, int acc );

 protected:

  DOMElement *m_element;
  char m_name;
  int m_oct;
  int m_acc;
  
}


const std::vector<PerfNote*> *getNotes( PerfPart *perfpart );

const std::vector<ScoreNote*> *getNotes( ScorePart *scorepart );





class PerfNote{

 public:
  PerfNote( int part, double start, double end );
  PerfNote( int part, double start, double end, double freq );
  PerfNote( DOMElement *element );

  double getStart();
  double getEnd();
  double getAvgFreq();

 protected:
  DOMElement *m_element;
  int m_part;
  double m_start;
  double m_endl;
  double m_freq;

}



