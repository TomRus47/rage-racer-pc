/* The CD request machine runs unchanged around a host replacement. These
 * cases exercise scene-boundary ordering without a disc or audio device. */
#include "game/cd.h"
#include "game/cd_internal.h"
#include "game/scene.h"
#include "game/race.h"
#include "psyq/cd.h"
#include "rage/custom_music.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

Cd g_Cd;
s32 g_CdTrackEnded, g_SceneId;
CdlLOC g_CdTrackLocs[18], g_CdTrackLoopPoint[18];
static int s_custom, s_playing, s_ended, s_haveFile = 1;
static int s_plays, s_pauses, s_discPlaying, s_discEnded, s_discPlays, s_gain;
int CustomMusicSelect(int track) {
    (void)track; s_custom = s_haveFile; s_playing = s_ended = 0; return s_custom;
}
int CustomMusicActive(void) { return s_custom; }
int CustomMusicPlaying(void) { return s_custom && s_playing; }
int CustomMusicEnded(void) { return s_custom && s_playing && s_ended; }
void CustomMusicPlay(void) { s_playing = 1; s_ended = 0; s_plays++; }
void CustomMusicPause(void) { s_playing = 0; s_pauses++; }
void CustomMusicShutdown(void) { s_custom = s_playing = s_ended = 0; }
void CustomMusicSetVolume(int level) { s_gain = level; }
long CdSync(long mode, u_char *result) { (void)mode; (void)result; return CD_SYNC_COMPLETE; }
long CdControl(long command, void *param, u_char *result) {
    (void)param;
    if (result) memset(result, 0, 8);
    if (command == CD_DRIVE_PLAY) { s_discPlaying = 1; s_discEnded = 0; s_discPlays++; }
    if (command == CD_DRIVE_PAUSE) s_discPlaying = 0;
    return 1;
}
int Psyz_CdAudioPlaying(void) { return s_discPlaying; }
int Psyz_CdAudioEnded(void) { return s_discEnded; }
int CdPosToInt(CdlLOC *p) { return p->minute * 4500 + p->second * 75 + p->sector; }
void SsSetSerialVol(u8 channel, short left, short right) { (void)channel; (void)left; (void)right; }
void BuildCdTrackTable(void) {}
void CdMix(u8 *mix) { (void)mix; }
void SetCdVolume(s32 volume) {
    g_Cd.volume = volume;
    g_Cd.mix.ll = g_Cd.mix.rr = (u32)volume * CD_MIX_FIXED_ONE;
    g_Cd.mix.lr = g_Cd.mix.rl = 0;
}
static void Tick(int count) { while (count--) TickCdAudio(); }
static void Select(int track) {
    RequestCdTrack(track);
    Tick(12);
    assert(g_Cd.pendingTrack == -1 && g_Cd.currentTrack == track);
}

int main(void) {
    InitCdAudio();
    SetCdVolume(127);
    g_CdTrackLoopPoint[0].second = 1;
    for (int i=2;i<18;i++)g_CdTrackLoopPoint[i].second = 2;
    /* An EOF left by a previous race must not start the next selected track
     * until the standing-start path issues StartCdAudio. */
    s_discEnded = 1;
    Select(3);
    Tick(50);
    assert(s_plays == 0 && s_discPlays == 0 && !g_Cd.playedSinceSelect);
    StartCdAudio();Tick(1);
    assert(s_plays == 1 && g_Cd.playedSinceSelect);
    s_ended = 1;Tick(1);
    assert(g_Cd.pendingTrack == 3);
    Tick(12);
    assert(s_plays == 2 && s_playing);
    PauseCdAudio();Tick(1);
    s_ended = 1;Tick(30);
    assert(!s_playing && s_plays == 2 && g_Cd.pendingTrack == -1);
    ResumeCdAudio();Tick(1);
    assert(s_plays == 3 && s_playing);
    /* Retry a race directly, without visiting a menu. */
    Select(4);Tick(50);
    assert(!s_playing && !g_Cd.playedSinceSelect && s_plays == 3);
    StartCdAudio();Tick(1);
    assert(s_plays == 4);
    StartCdVolumeFade(4);Tick(4);
    assert(s_gain == 0 && g_Cd.fade == 0);
    /* Stop post-race BGM at the replay/result boundary. Its EOF cannot loop
     * during the result screen or escape into another race's countdown. */
    for (int i=0;i<3;i++) {
        const int tracks[] = {12,13,16};
        Select(tracks[i]);StartCdAudio();Tick(1);
        int plays = s_plays;
        s_ended = 1;
        StopReplayResultMusic();Tick(30);
        assert(!s_playing && s_plays == plays && g_Cd.currentTrack == 0xff);
        Select(5);Tick(30);
        assert(!s_playing && s_plays == plays);
    }
    ResetCdAudioState();
    assert(!CustomMusicActive());
    /* The absent-file path retains native CD-DA ordering and EOF protection. */
    s_haveFile = 0;s_discEnded = 1;
    Select(3);Tick(40);
    assert(s_discPlays == 0);
    StartCdAudio();Tick(6);
    assert(s_discPlaying && s_discPlays == 1);
    Select(12);StartCdAudio();Tick(6);
    int discPlays = s_discPlays;
    s_discEnded = 1;
    StopReplayResultMusic();Tick(30);
    assert(!s_discPlaying && s_discPlays == discPlays);
    assert(s_pauses > 0);
    puts("custom music CD requests: countdown, retry, pause, loop and results passed");
    return 0;
}
