#include "rage/custom_music.h"
#include "platform_paths.h"
#include "runtime_config.h"

#include <SDL3/SDL.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* The decoder is kept in this translation unit so the same implementation can
 * be used by the standalone port and the source overlay. */
#include "../../external/stb/stb_vorbis.c"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct CustomMusic {
    SDL_AudioStream *stream;
    void *samples;
    void (*freeSamples)(void *);
    int sampleBytes;
    int playing;
    int ended;
    int mixLevel;
    int volumePercent;
} CustomMusic;

static CustomMusic s_music = {.mixLevel = 127};

static void CloseTrack(void) {
    if (s_music.stream) SDL_DestroyAudioStream(s_music.stream);
    if (s_music.samples) s_music.freeSamples(s_music.samples);
    s_music.stream = NULL;
    s_music.samples = NULL;
    s_music.freeSamples = NULL;
    s_music.sampleBytes = 0;
    s_music.playing = 0;
    s_music.ended = 0;
}

static int LoadVorbis(const char *path, SDL_AudioSpec *spec) {
    size_t size = 0;
    unsigned char *file = SDL_LoadFile(path, &size);
    short *samples = NULL;
    int channels = 0, rate = 0, frames;

    if (!file) return 0;
    if (size > INT_MAX) {
        SDL_free(file);
        return 0;
    }
    frames = stb_vorbis_decode_memory(file, (int)size, &channels, &rate, &samples);
    SDL_free(file);
    if (frames <= 0 || channels < 1 || channels > 8 || rate <= 0 ||
        frames > INT_MAX / channels / (int)sizeof(*samples)) {
        free(samples);
        return 0;
    }
    spec->format = SDL_AUDIO_S16;
    spec->channels = channels;
    spec->freq = rate;
    s_music.samples = samples;
    s_music.freeSamples = free;
    s_music.sampleBytes = frames * channels * (int)sizeof(*samples);
    return 1;
}

static int LoadWav(const char *path, SDL_AudioSpec *spec) {
    Uint8 *samples = NULL;
    Uint32 size = 0;
    if (!SDL_LoadWAV(path, spec, &samples, &size)) return 0;
    if (size == 0 || size > INT_MAX) {
        SDL_free(samples);
        return 0;
    }
    s_music.samples = samples;
    s_music.freeSamples = SDL_free;
    s_music.sampleBytes = (int)size;
    return 1;
}

static int QueueTrack(void) {
    return SDL_PutAudioStreamData(s_music.stream, s_music.samples,
                                  s_music.sampleBytes) &&
           SDL_FlushAudioStream(s_music.stream);
}

int CustomMusicSelect(int cdTrack) {
    static const char *extensions[] = {"ogg", "wav"};
    char directory[PATH_MAX], path[PATH_MAX];
    SDL_AudioSpec spec;
    int loaded = 0;

    CloseTrack();
    if (!RuntimeConfigEnabled("audio.custom_music") ||
        cdTrack < 2 || cdTrack > 17 ||
        !PlatformExecutableDirectory(NULL, directory, sizeof(directory)))
        return 0;
    for (unsigned int i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++) {
        int count = snprintf(path, sizeof(path), "%s/music/track%02d.%s",
                             directory, cdTrack, extensions[i]);
        if (count < 0 || (size_t)count >= sizeof(path)) continue;
        loaded = i == 0 ? LoadVorbis(path, &spec) : LoadWav(path, &spec);
        if (loaded) break;
    }
    if (!loaded) return 0;
    s_music.stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!s_music.stream || !SDL_PauseAudioStreamDevice(s_music.stream) ||
        !QueueTrack()) {
        fprintf(stderr, "rage-port: cannot play custom music %s: %s\n",
                path, SDL_GetError());
        CloseTrack();
        return 0;
    }
    s_music.volumePercent = RuntimeConfigInt("audio.custom_music_volume", 100, 0, 100);
    CustomMusicSetVolume(s_music.mixLevel);
    fprintf(stderr, "rage-port: custom music=%s\n", path);
    return 1;
}

int CustomMusicActive(void) { return s_music.stream != NULL; }
int CustomMusicPlaying(void) { return CustomMusicActive() && s_music.playing; }

int CustomMusicEnded(void) {
    if (!CustomMusicPlaying()) return 0;
    if (SDL_GetAudioStreamQueued(s_music.stream) == 0 &&
        SDL_GetAudioStreamAvailable(s_music.stream) == 0)
        s_music.ended = 1;
    return s_music.ended;
}

void CustomMusicPlay(void) {
    if (!CustomMusicActive()) return;
    if (s_music.ended) {
        if (!SDL_ClearAudioStream(s_music.stream) || !QueueTrack()) return;
        s_music.ended = 0;
    }
    if (SDL_ResumeAudioStreamDevice(s_music.stream)) s_music.playing = 1;
}

void CustomMusicPause(void) {
    if (CustomMusicActive()) SDL_PauseAudioStreamDevice(s_music.stream);
    s_music.playing = 0;
}

void CustomMusicSetVolume(int cdMixLevel) {
    if (cdMixLevel < 0) cdMixLevel = 0;
    if (cdMixLevel > 127) cdMixLevel = 127;
    s_music.mixLevel = cdMixLevel;
    if (CustomMusicActive())
        SDL_SetAudioStreamGain(s_music.stream,
            (float)cdMixLevel / 127.0f * (float)s_music.volumePercent / 100.0f);
}

void CustomMusicShutdown(void) { CloseTrack(); }
