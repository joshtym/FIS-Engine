/*******************************************************************************
 * Class Name: Sound
 * Date Created: February 9, 2012
 * Inheritance: none
 * Description: Handles all sound used within the game. This individual class
 *              only handles playing a single sound at one time. If you click
 *              play while it's still playing, it will stop the previous
 *              execution. However, if you create multiple sound files, SDL 
 *              allows up to 8 files being mixed together at once before
 *              returning the channel full error.
 ******************************************************************************/
#ifndef SOUND_H
#define SOUND_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>

/* Enumerator class for sound */
enum class SoundChannels : int
{
  UNASSIGNED = -1,
  MUSIC1     = 0,
  MUSIC2     = 1,
  WEATHER1   = 2,
  WEATHER2   = 3,
  MENUS      = 4,
  TILES      = 5,
  THINGS     = 6,
  SECTORS    = 7,
  TRIGGERS   = 8
};

class Sound
{
public:
  /* Constructor: Sets up a blank template, nothing will play */
  Sound();

  /* Constructor: Sets up all the pertinent data to create a sound */
  Sound(SoundChannels channel, std::string path, int loop_count = 0);

  /* Destructor function */
  ~Sound();

private:
  /* The channel number the sound will be played on */
  SoundChannels channel;

  /* The time to fade the sound in. If 0, no fade */
  uint32_t fade_time;
  
  /* The number of times to play the sound. It will be loop_count+1 
   * except for -1, which is infinite loop */
  int loop_count;

  /* The raw data of the sound in RAM */
  Mix_Chunk* raw_data;

  /* The volume that the chunk will play at */
  uint8_t volume;
  
  /* --------------------- CONSTANTS --------------------- */
private:
  const static short kINFINITE_LOOP; /* Infinite loop special character */

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/
public:
  /* Cross fade the given channel out with the current class in */
  void crossFade(int channel);
  
  /* Returns the channel that the music is playing on. Useful for calling mix
   * calls to get feedback on the sound */
  SoundChannels getChannel();
  int getChannelInt();
  
  /* Returns the time that the chunk will be faded in or out */
  uint32_t getFadeTime();
  
  /* Returns the loop count, the number of times it will play for */
  int getLoopCount();

  /* Returns the raw chunk data, of the loaded sound. NULL if inactive */
  Mix_Chunk* getRawData();
  
  /* Returns the volume of the sound chunk */
  uint8_t getVolume();
  
  /* Play function. If sound is set, it will play the sound for the given number
   * of loops */
  bool play(bool stop_channel = true);
  
  /* Sets the channel integer to be used for playing the chunk */
  void setChannel(SoundChannels channel);
  
  /* Sets the fade time for the sound chunk sequence (in msec) */
  void setFadeTime(uint32_t time);
  
  /* Set the number of times to play the song for. If 0, defaults to 1 */
  void setLoopCount(int loop_count);

  /* Set the sound to loop infinitely, until stop() is called */
  void setLoopForever();

  /* Trys to set the sound file to the given path */
  bool setSoundFile(std::string path);
  
  /* Sets the volume that the chunk will be played at. */
  void setVolume(uint8_t volume);
  
  /* Stop function. Will stop the sound, if playing */
  bool stop(bool stop_channel = true);
  
  /* Unset the sound file and frees the memory */
  void unsetSoundFile();
};

#endif // SOUND_H