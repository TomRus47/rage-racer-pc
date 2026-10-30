/* Native CD tests keep the optional replacement disabled. */
#include "rage/custom_music.h"
int CustomMusicSelect(int track) { (void)track; return 0; }
int CustomMusicActive(void) { return 0; }
int CustomMusicPlaying(void) { return 0; }
int CustomMusicEnded(void) { return 0; }
void CustomMusicPlay(void) {}
void CustomMusicPause(void) {}
void CustomMusicSetVolume(int level) { (void)level; }
void CustomMusicShutdown(void) {}
