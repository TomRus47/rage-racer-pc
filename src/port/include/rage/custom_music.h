#ifndef RAGE_CUSTOM_MUSIC_H
#define RAGE_CUSTOM_MUSIC_H

/* Optional CD-DA replacements in music/track02..17.ogg or .wav beside the
 * executable. An absent or unreadable replacement leaves CD-DA in control. */
int CustomMusicSelect(int cdTrack);
int CustomMusicActive(void);
int CustomMusicPlaying(void);
int CustomMusicEnded(void);
void CustomMusicPlay(void);
void CustomMusicPause(void);
void CustomMusicSetVolume(int cdMixLevel);
void CustomMusicShutdown(void);

#endif
