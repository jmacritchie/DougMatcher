/********************************************/
/* Basic Midi File Reader (C) D McGilvray   */
/* Use at your own risk...                  */
/* 10/01/2007                               */
/********************************************/



/*
  HUGE TO DO:

  There deosn't seem to be a need to have delta times 
  between each event. This seems to be a RG thing.
  Cause I'm sure that's illegal.



*/

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <main.h>

#define DEBUG 1
#define DEF_OFF_VEL 127     // default noteoff velocity 

using std::cout;
using std::endl;

// when noteon occurs, mark velocity in channel/pitch array
//  unless velocity exists, in which case, end previous note first
// when noteoff occurs, zero velocity in array



// Velocity array
typedef struct NoteOn{
  int dTime;
  int vel;
};

NoteOn velArray[16][128];



// handle meta events
void handle_meta_event(FILE *input_file);
// reads variable length value
unsigned long readVarLen();
//Gets bytes from file and converts into single decimal value
int getValue( int bytes );
// convert midi ticks to real time
double realTime( long dTime );
int readFile();

void noteOn( long onset, int channel, int pitch, int vel );
void noteOff( long onset, int channel, int pitch, int vel );


int c[8],n=0,d,v;
int trackLength;
char buffer[50];
bool endOfTrack = 0;
int timesig[2];      // numerator, denominator
int metronome;       // midi clock ticks per metronome
int ts32pq;          // number of notated 32nds per quarter note
int key;             // number of sharps/flats (flats are negative)
int mode;            // major=1, minor=0
long accTime;        // accumulated time
int divisions;       // divisions in a quarter note
int quarterNote=500000;// length of quarterNote in micro seconds (default = 120bpm)
int midiChannel;     // midi channel to convert
FILE *inFile;

int readFile( FILE* file, int midiCh ){

  midiChannel = midiCh;

  inFile = file;
  
  int no_tracks;

  memset( velArray, 16*128, sizeof(NoteOn) );

  fgets( buffer, 5, inFile );
  if ( strcpy( buffer, "MThd" )){

    c[0] = getValue(4);
    if(DEBUG) printf("Header Length %d\n", c[0]);

    v = getValue(2);
    no_tracks = getValue(2);
    divisions = getValue(2);
    if(DEBUG) printf("format %d\n",v);
    if(DEBUG) printf("number of tracks %d\n",no_tracks);
    if(DEBUG) printf("Division = %d\n", divisions);

    if (divisions & 0x8000){
      if(DEBUG) printf("Bit 15 set SMPTE Time");
    }
    else{
      if(DEBUG) printf("Ticks per quarter note %d (dec)\n",divisions);
    }

    // Read each track
    for( int track=0; track<no_tracks; track++ ){

      accTime=0;

      //check for track header
      fgets( buffer, 5, inFile );
      if ( strcpy( buffer, "MTrk" )){
	endOfTrack=false;
	if(DEBUG) printf("\n TRACK %d\n", track);

	trackLength = getValue(4);  // read track length
	if(DEBUG) printf("Track Length %d\n", trackLength);

	/*
	  Parsing track events:
	  Get delta time, add to rolling time
	  Get event (if no event suggested by
	  next char, use previous event)
	*/

	char previousEvent=0;

	while( !endOfTrack ){

	  int dTime = readVarLen();
	  accTime += dTime;
	  if(DEBUG) printf("dTime %d\t", accTime );
	  if(DEBUG) printf("Time %lf\t", realTime(accTime) );

	  //get next char to decode event

	  d = fgetc(inFile);

	  // If next byte is not a status byte, use the previous
	  // status byte and rewind one byte in the file
	  if( (d & 0xf0) >= 0x80 ){
	    previousEvent = d;
	  }
	  else{
	    d = previousEvent;
	    fseek( inFile, -1, SEEK_CUR );
	  }

	  if ((d & 0xf0) == 0x90){
	    n=fgetc(inFile);
	    v=fgetc(inFile);
	    if(DEBUG) printf("Note On channel %d Pitch %d Volume %d\n",d&0x0f, n, v);
	    noteOn( accTime, d&0xf, n, v );
	  }
	  else if ((d & 0xf0)== 0x80){
	    n=fgetc(inFile);
	    v=fgetc(inFile);
	    if(DEBUG) printf("Note Off channel %d Pitch %d Volume %d\n",d&0x0f, n, v);
	    noteOff( accTime, d&0xf, n, v );
	  }
	  else if ((d & 0xf0)== 0xa0){
	    n=fgetc(inFile);
	    v=fgetc(inFile);
	    if(DEBUG) printf("Polyphonic Key Pressure Event\n");
	    if(DEBUG) printf("channel %d note %d value %d\n", (d&0xf), n, v);
	  }
	  else if ((d & 0xf0)== 0xb0){
	    n=fgetc(inFile);
	    v=fgetc(inFile);
	    if(DEBUG) printf("Control change ");
	    if(DEBUG) printf("channel %d number %d value %d\n", (d&0xf), n, v);
	  }
	  else if ((d & 0xf0)== 0xc0){
	    n=fgetc(inFile);
	    if(DEBUG) printf("Program change ");
	    if(DEBUG) printf("channel %dvalue %d\n", (d&0xf), n );
	  }
	  else if ((d & 0xf0)== 0xd0){
	    n=fgetc(inFile);
	    if(DEBUG) printf("Channel Pressure (After Touch)");
	    if(DEBUG) printf("channel %dvalue %d\n", (d&0xf), n );
	  }
	  else if (d ==0xff){
	    handle_meta_event(inFile);
	  }
	  else{
	    if(DEBUG) printf("Error: what is this?\n");
	  }
	}
      }
    }
  }
  else 
    if(DEBUG) printf("Not a midi file!\n");
}
  
void handle_meta_event(FILE *inFile)
{
  int hr, mn, se, fr, ff;
  int d=fgetc(inFile);
  switch(d){
  case 0:
    if(DEBUG) printf("Sequence Number\n");
    d=fgetc(inFile);
    if(DEBUG) printf(" 02 identified %d\n", d);
    d=fgetc(inFile);
    if(DEBUG) printf(" Data %d\n", d);
    d=fgetc(inFile);
    if(DEBUG) printf("Data %d\n", d);
    break;
  case 1:
    if(DEBUG) printf("Text Event\n");
    d=fgetc(inFile);
    if(DEBUG) printf("Text length Length %d\n", d);
    fseek(inFile, d, SEEK_CUR);
    break;
  case 2:
    if(DEBUG) printf("Copyright notice\n");
    d=fgetc(inFile);
    if(DEBUG) printf("Text length Length %d\n", d);
    fseek(inFile, d, SEEK_CUR);
    break;
  case 3: // TRACK NAME
    if(DEBUG) printf("Track Name\n");
    d=readVarLen();
    if( d ){
      fgets( buffer, d+1, inFile );
      if(DEBUG) printf("Track Name length %d \"%s\"\n", d, buffer);
    }
    break;
  case 4:
    if(DEBUG) printf("Instrument Name \n");
    d=fgetc(inFile);
    if(DEBUG) printf("Instrument Name Text length Length %d\n", d);
    fseek(inFile, d, SEEK_CUR);
    break;
  case 5:
    if(DEBUG) printf("Lyric \n");
    d=fgetc(inFile);
    if(DEBUG) printf("Lyric Text length Length %d\n", d);
    fseek(inFile, d, SEEK_CUR);
    break;
  case 6:
    if(DEBUG) printf("Marker");
    d=fgetc(inFile);
    char marker[d+1];
    fgets( marker, d+1, inFile );
    if(DEBUG) printf("\t\"%s\"\n", marker);
    break;
  case 7:
    if(DEBUG) printf("Cue Point\n");
    d=fgetc(inFile);
    if(DEBUG) printf("Cue Point Text length Length %d\n", d);
    fseek(inFile, d, SEEK_CUR);
    break;
  case 8:
    if(DEBUG) printf("Midi Channel Prefix\n");
    break;
  case 81:
    if(DEBUG) printf("Set tempo\n");
    d=fgetc(inFile);
    if( d != 3 ) if(DEBUG) printf("Err: tempo should have length of 3");
    quarterNote = getValue(3);
    if(DEBUG) printf("Each quarter note is %d microseconds long\n",quarterNote);
    break;
  case 84:
    if(DEBUG) printf("SMPTE Offset\n");
    d=fgetc(inFile);
    if(DEBUG) printf("05 detected %d\n", d);
    hr=fgetc(inFile);
    if(DEBUG) printf("hr = %d\n", hr);
    mn=fgetc(inFile);
    if(DEBUG) printf("mn =  %d\n", mn);
    se=fgetc(inFile);
    if(DEBUG) printf("se =  %d\n", se);
    fr=fgetc(inFile);
    if(DEBUG) printf("fr =  %d\n", fr);
    ff=fgetc(inFile);
    if(DEBUG) printf("ff =  %d\n", ff);
    break;
  case 88: // TIME SIGNATURE
    d=fgetc(inFile);
    if( d != 4 ) if(DEBUG) printf("Err: time sig should have length of 4\n", d);
    for( n=0; n<4; n++ )
      c[n] = fgetc(inFile);

    timesig[0] = c[0];
    timesig[1] = (int)pow( 2, c[1] );
    metronome = c[2];
    ts32pq = c[3];

    if(DEBUG) printf("Time sig is %d/%d with %d ticks per metronome and %d notated 32nds per q\n",
	   c[0], c[1], c[2], c[3] );

    break;
  case 89: // KEY SIGNATURE
    if(DEBUG) printf("Key Signature\n");
    d=fgetc(inFile);
    d=fgetc(inFile);
    key=fgetc(inFile);
    mode=fgetc(inFile);
    if( key < 0 )
      if(DEBUG) printf("Key sigs has %d flats, ", key*-1 );
      if(DEBUG) printf("Key sigs has %d sharps, ", key);

    if( mode )
      if(DEBUG) printf("and is in minor mode\n");
    else
      if(DEBUG) printf("and is in major mode\n");
    break;
  case 47:
    if(DEBUG) printf("End of track\n");
    readVarLen(); // skip 00 delta time before next track
    endOfTrack = true;
    break;
  default:
    if(DEBUG) printf("Unknown meta event %x\n", d);
  }
}


unsigned long readVarLen()
{
    register unsigned long value;
    register unsigned char c;

    if ( (value = getc(inFile)) & 0x80 )
    {
       value &= 0x7F;
       do
       {
         value = (value << 7) + ((c = getc(inFile)) & 0x7F);
       } while (c & 0x80);
    }
    cout << "Variable length value: " << value << endl;
    return(value);
}

int getValue( int bytes ){
  int buff[bytes];
  int value=0;
  int n=0;
  while( n<bytes )
    buff[n++] = fgetc(inFile);

  for( n=0; n<bytes; n++ )
    value = value | ( buff[n] << ((bytes-n-1)*8) );

  return value;
}

double realTime( long dTime ){
  return (double)dTime *  0.000001 * (double)quarterNote / (double)divisions;
}


void noteOff( long offset, int channel, int pitch, int vel ){
  
  if( channel == midiChannel ){

    long onset = velArray[channel][pitch].dTime;
    int onvel = velArray[channel][pitch].vel;
    cout << "Adding note to perf part\n";
    // add note to pml performance part
    newNote( realTime( onset ),
	     realTime( offset ),
	     pitch,
	     onvel );  //BEN - extra for velocity
    
    
    // remove entry in velarray
    velArray[channel][pitch].vel = 0;
    cout << "Done.\n";
  }
}

void noteOn( long onset, int channel, int pitch, int vel ){

  // If last note not ended, end note first
  if( velArray[channel][pitch].vel != 0 ){
    noteOff( velArray[channel][pitch].dTime, channel, pitch, 
	     DEF_OFF_VEL );
  }

  velArray[channel][pitch].vel = vel;
  velArray[channel][pitch].dTime = onset;
  
}

