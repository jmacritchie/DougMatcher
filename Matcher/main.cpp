
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "pmldocument.h"
#include "pmlmatcher.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

void usage(){
  cout << "\nUsage:\n"
       <<"winmatch <PML file> \n"
       <<"winmatch <PML file> <window size> <clustering threshold> <max winshift> \n"
       <<"           <init clust thresh>\n\n"
       <<"window size       :  size of clustering window (10)\n"
       <<"clustering thresh :  fraction of distance between clusters\n"
       <<"                     to include notes eg 0.5 halfway between two clusters\n"
       <<"max window shift  :  max shift of window in clusters between evals (5)\n"
       <<"init clust thresh :  initial threshold for clustering (was 0.05)\n"
       <<"\n"
       <<"A suggested starting point is Window size of 10, window shift 5 & clustering threshold 0.5.\n"
       <<"Detailed instructions can be found in the INSTRUCTIONS file in the source code\n";

  exit(1);
}


#define WINDOWSIZE 15        // Window size
#define MAXWINSHIFT 5        // Maximum gap between windows
#define CLUST_THRESH 0.5      // default threshold for clustering
#define INIT_CLUST 0.05      // default threshold for clustering

int main( int argc, char **argv ){
  
  if (  argc != 2 && argc !=6 ) usage();

  int winsize, win_shift;
  double clust, initClust;
  if( argc == 6 ){
    winsize = atoi(argv[2]);
    clust = atof(argv[3]);
    win_shift = atoi(argv[4]);
    initClust = atof(argv[5]);
  }
  else{
    clust = CLUST_THRESH;
    win_shift = MAXWINSHIFT;
    winsize = WINDOWSIZE;
    initClust = INIT_CLUST;
  }

  cout << "Clustering threshold: " << clust << endl
       << "WIndow size: " << winsize << endl
       << "Win shift: " << win_shift << endl
       << "Initial clustering threshold: " << initClust << endl;

  const char *inFile = argv[1];//argv[1];
    
  PMLDocument doc;
  doc.openFile( inFile,false );
  ScorePart *part;
  int partNo; 

  if( doc.getNoParts() == 1 )
    part = doc.getScorePart(0);
  else{
    printf("Which Part is to be matched?\n");
    for( int i=0; i<doc.getNoParts(); i++ )
      printf("\t%d\t%s\n", i, doc.getScorePart(i)->getName().c_str() );
    
    do{
      scanf("%d", &partNo );
      cout << partNo << endl;
    }while( 0 > partNo && partNo >= doc.getNoParts() );
    
    part = doc.getScorePart(partNo);
  }
  PMLMatcher matcher( &doc, winsize, clust, win_shift, initClust  );
  matcher.setPart( part );
  cout << "Part set\n";  
  matcher.match();  
  // Now just print out correspondences
  matcher.alignMatches(false);  
  cout << "Writing file\n";
  doc.writeFile("matched.xml");
  cout << "done.\n";



  return 0;
}



