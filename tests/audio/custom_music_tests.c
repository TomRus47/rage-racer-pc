#include "rage/custom_music.h"
#include "platform_paths.h"
#include "runtime_config.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#ifndef CUSTOM_MUSIC_FIXTURES
#define CUSTOM_MUSIC_FIXTURES "tests/audio/fixtures"
#endif
static char s_directory[4096];
static int s_enabled = 1;
int PlatformExecutableDirectory(const char *argv0, char *path, size_t size) {
    (void)argv0;
    return SDL_strlcpy(path, s_directory, size) < size;
}
int RuntimeConfigEnabled(const char *key) {
    return !strcmp(key, "audio.custom_music") && s_enabled;
}
int RuntimeConfigInt(const char *key, int fallback, int minimum, int maximum) {
    (void)key; (void)minimum; (void)maximum;
    return fallback;
}
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"line %d: %s (%s)\n",__LINE__,#c,SDL_GetError()); return 1; } } while (0)

static int Copy(const char *from, const char *to) {
    size_t size;
    void *data = SDL_LoadFile(from, &size);
    int ok = data && SDL_SaveFile(to, data, size);
    SDL_free(data);
    return ok;
}

static void Write16(FILE *f, unsigned value) {
    fputc(value & 255, f); fputc((value >> 8) & 255, f);
}
static void Write32(FILE *f, unsigned value) {
    Write16(f, value); Write16(f, value >> 16);
}
static int Wav(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fwrite("RIFF",1,4,f);Write32(f,36+2205*2);
    fwrite("WAVEfmt ",1,8,f);Write32(f,16);Write16(f,1);Write16(f,1);
    Write32(f,22050);Write32(f,44100);Write16(f,2);Write16(f,16);
    fwrite("data",1,4,f);Write32(f,2205*2);
    for (int i=0;i<2205;i++)Write16(f,(i&1)?1000:64536);
    return fclose(f)==0;
}

int main(void) {
    char music[4096], ogg[4096], wav[4096];
    CHECK(SDL_SetHint(SDL_HINT_AUDIO_DRIVER,"dummy"));
    CHECK(SDL_Init(SDL_INIT_AUDIO));
    {
        char *current = SDL_GetCurrentDirectory();
        CHECK(current);
        SDL_strlcpy(s_directory, current, sizeof(s_directory));
        SDL_free(current);
    }
    SDL_snprintf(music,sizeof(music),"%s/custom-music-test",s_directory);
    CHECK(SDL_CreateDirectory(music));
    SDL_strlcpy(s_directory,music,sizeof(s_directory));
    SDL_snprintf(music,sizeof(music),"%s/music",s_directory);
    CHECK(SDL_CreateDirectory(music));
    SDL_snprintf(ogg,sizeof(ogg),"%s/track02.ogg",music);
    SDL_snprintf(wav,sizeof(wav),"%s/track03.wav",music);
    CHECK(Copy(CUSTOM_MUSIC_FIXTURES "/custom-tone.ogg",ogg));
    CHECK(Wav(wav));
    CHECK(CustomMusicSelect(2));
    CHECK(CustomMusicActive() && !CustomMusicPlaying() && !CustomMusicEnded());
    CustomMusicPlay();
    CHECK(CustomMusicPlaying());
    CustomMusicPause();
    SDL_Delay(120);
    CHECK(!CustomMusicEnded());
    CustomMusicPlay();
    for (int i=0;i<100 && !CustomMusicEnded();i++)SDL_Delay(10);
    CHECK(CustomMusicEnded());
    CustomMusicPlay();
    CHECK(CustomMusicPlaying() && !CustomMusicEnded());
    CHECK(CustomMusicSelect(3));
    CHECK(!CustomMusicPlaying());
    CustomMusicPlay();
    CHECK(CustomMusicPlaying());
    CustomMusicSetVolume(0);CustomMusicSetVolume(127);
    CustomMusicSetVolume(-100);CustomMusicSetVolume(500);
    CHECK(!CustomMusicSelect(4) && !CustomMusicActive());
    CHECK(!CustomMusicSelect(1) && !CustomMusicActive());
    CHECK(!CustomMusicSelect(18) && !CustomMusicActive());
    /* A broken preferred Ogg falls back to a valid WAV in the same slot. */
    SDL_snprintf(ogg,sizeof(ogg),"%s/track03.ogg",music);
    CHECK(SDL_SaveFile(ogg,"not vorbis",10));
    CHECK(CustomMusicSelect(3));
    CustomMusicShutdown();
    CHECK(!CustomMusicActive());
    s_enabled=0;
    CHECK(!CustomMusicSelect(3));
    SDL_Quit();
    puts("custom music: WAV/Vorbis, fallback, pause, EOF and replay passed");
    return 0;
}
