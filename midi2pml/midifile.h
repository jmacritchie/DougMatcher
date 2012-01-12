



/**

   Idea:
   Create a class hierarchy which represents the data in a midi file.
   Top class is MidiDocument which contains metadata, initial settings,
   a vector/array of tracks, and can perform conversion between relative
   and absolute time.

   Tracks hold a vector of events. 
   Events can be added/removed to/from tracks.
   Tracks can calculate their length in bytes (needed for writing files).

   Hierarchy can be created by parsing a midi file. And written to a midi file.

   Midi time is stored as bar,beat,division.
   Midi Time can be converted to absolute time (secs), and deltaTime 
    (duration which requires context time to ascertain tempo etc.)

  Each structure is descendant of MidiMessage which encapsulates a time 
  and a string of bytes.

  Each structure must be able to translate it's contents into a byte string,
  thus obtaining the data for a midi file is simply doc->getMessage();

 */




// Get variable length value from current position in file
int getVarLengthValue();

// Get /bytes/ number of bytes from current position in file 
// and convert into value
int getValue( size_t bytes );


class Track{

  string m_name;
  std::vector<MidiMessage> m_items;


}



class MidiTime{

  int m_bar;
  int m_beat;
  int m_divisions;

  MidiTime( int bar, beat, divisions );

  MidiTime operator-( MidiTime &time2 );
  MidiTime operator+( MidiTime &time2 );
  MidiTime operator=( MidiTime &time2 );
  MidiTime operator!=( MidiTime &time2 );

  double getRealTime();
  double getDeltaTime();
}


class MidiMessage{
  MidiTime m_time;
  char *m_message;
}

class MidiEvent : MidiMessage{
  int m_channel;
}

class NoteOn : MidiEvent{
  MidiNo m_pitch;
  char m_velocity;

  NoteOff getNoteOff();

}

class NoteOff : MidiEvent{
  MidiNo m_pitch;
  char m_velocity;
}

class Aftertouch : MidiEvent{
  MidiNo m_pitch;
}
class Controller : MidiEvent{
  char controller;
  char value;
}
class ProgramChange : MidiEvent{
  char value;
}
class ChannelAftertouch : MidiEvent{
  char value;
}
class PitchBend : MidiEvent{
  short value;
}

/* Midi Event types

   Note Off           0x8  (note off) note number  velocity 
   Note Aftertouch    0xA  note number  aftertouch value
   Controller         0xB  controller number  controller value
   Program Change     0xC  program number  not used
   Channel Aftertouch 0xD  aftertouch value  not used
   Pitch Bend         0xE  pitch value (LSB)  pitch value (MSB)
*/



class MetaEvent : MidiMessage{
}
