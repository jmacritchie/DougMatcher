

#include <string>


class DOMDocument;


class Performance{

  Performance( DOMDocument *doc );
  std::string operator[]( int i );

}



Performance::Performance( PerfPart *perf ){


  m_perfPart = perf;





}






class Score{

  Score( DOMDocument *doc );
  std::string operator[]( int i );

}



