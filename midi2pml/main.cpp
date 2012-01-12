#include <pmldocument.h>
#include <scorepart.h>
#include <perfpart.h>
#include <stdio.h>
#include "readmidi.h"

PMLDocument pml;
PerfPart *perf;


int main( int argc, char **argv ){

  int partNo;
  ScorePart *score;

  if( argc != 3 ){

    printf("Usage: midi2pml <midifile> <pmlfile>\n");
    printf("       midi2pml <midifile> <musicxmlfile>\n");
    return 1;
  }

  if( !pml.openFile( argv[2] ) ){
    printf("Cannot open %s\n", argv[2]);
    return 1;
  }

  FILE *inFile = fopen( argv[1], "r" );
  if( !inFile ){
    printf("Cannot open %s\n", argv[1]);
    return 1;
  }


  // List score parts 

  // Ask for score part to link performance to
  int noParts = pml.getNoParts();


  printf("Which score part does the midi data represent?\n");
  for( int i=0; i<pml.getNoParts(); i++ )
    printf("\t%d\t%s\n", i, pml.getScorePart(i)->getName().c_str() );

  scanf("%d", &partNo );
  score = pml.getScorePart(partNo);
  perf = score->getPerfPart();


  // Ask for midi channel to read
  int midiChannel;
  printf("\nIn which midi channel is the data contained?\n");
  scanf("%d", &midiChannel );

  printf("Reading from channel %d\n", midiChannel );

  // Parse
  readFile( inFile, midiChannel );

  pml.writeFile("out.xml");

}


//BEN - extra added for velocity information
void newNote( double onset, double offset, int pitch, int velocity ){
     // add note to pml performance part
     perf->addNote( new PNote( &pml,
			       onset,
			       offset,
			       (MidiNo)pitch,
			       velocity ) );

}
