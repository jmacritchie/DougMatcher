#ifndef SOUNDFILE_H
#define SOUNDFILE_H

/**
 * Sound file C++ wrapper. Version 0.1, D.McGilvray http://cmt/gla/ac/uk
 * Requires libsndfile. Released under the GNU General Public License.
 */

#include <sndfile.h> //libsndfile1-dev
#include <string>



class SoundFile{

 private:
  SNDFILE* m_sf;
  double *m_data;
  int m_sampleRate;
  int m_channels;
  long m_samples;
  std::string m_filename;

  /// extracts sound data from file
  void getData();

 public:
  SoundFile();
  ~SoundFile();

  bool openFile();
  int sampleRate();
  int channels();
  int length();
  double *data();
  void plot();
  void plot( long start, long end );


  long sampleFromTime( double time );
  double timeFromSample( long sample );

  std::string getFilename(){ return m_filename; }


};

#endif
