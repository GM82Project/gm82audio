#include "stb_vorbis.c"
//bruhhh
#undef L
#undef R
#undef C

#include "intrin.h"
#define CUTE_SOUND_IMPLEMENTATION
#include "cute_sound.h"

#include <vector>

#define GMREAL extern "C" __declspec(dllexport) double __cdecl 
#define GMSTR extern "C" __declspec(dllexport) char* __cdecl

GMREAL __gm82audio_init(double gm_hwnd);
GMREAL __gm82audio_update(double dt);
GMREAL __gm82audio_end();
GMSTR  __gm82audio_get_error();
GMREAL __gm82audio_loadwav(char* fn);
GMREAL __gm82audio_loadogg(char* fn);
GMREAL __gm82audio_sfx_play(double soundid,double vol,double pan,double pitch,double loops);
GMREAL __gm82audio_music_pause();
GMREAL __gm82audio_music_resume();
GMREAL __gm82audio_music_play(double soundid,double fadeintime,double vol,double pitch,double loops);
GMREAL __gm82audio_music_switch(double soundid,double fadeouttime,double fadeintime,double vol,double pitch,double loops);
GMREAL __gm82audio_music_crossfade(double soundid,double fadetime,double vol,double pitch,double loops);
GMREAL __gm82audio_music_pitch(double pitch);
GMREAL __gm82audio_music_volume(double volume);
GMREAL __gm82audio_music_loop(double loops);
GMREAL __gm82audio_music_get_pos();
GMREAL __gm82audio_music_set_pos(double pos);

static double SAMPLE_RATE=44100;
static int SOUND_INDEX=1;
static std::vector<cs_audio_source_t*> SOUNDS;
static char* ERROR_STR = "";
static bool MUSIC_PAUSED=false;
static cs_audio_source_t* CURRENT_SONG;

GMREAL __gm82audio_init(double gm_hwnd) {
    cs_init((HWND)(int)gm_hwnd,(int)SAMPLE_RATE,1024,NULL);
    cs_spawn_mix_thread();
    cs_mix_thread_sleep_delay(1);
    return 0;
}

GMREAL __gm82audio_update(double dt) {
    cs_update(dt);
    return 0;
}

GMREAL __gm82audio_end() {
    cs_shutdown();
    return 0;
}

GMSTR  __gm82audio_get_error() {
    return ERROR_STR;
}

GMREAL __gm82audio_loadwav(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_wav(fn,&error);
    if (snd==NULL) {
        strcpy(ERROR_STR,cs_error_as_string(error));
        return 0;
    }
    SOUNDS.reserve(((SOUND_INDEX+1)/256+1)*256);
    SOUNDS[SOUND_INDEX]=snd;
    return SOUND_INDEX++;
}

GMREAL __gm82audio_loadogg(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_ogg(fn,&error);
    if (snd==NULL) {
        strcpy(ERROR_STR,cs_error_as_string(error));
        return 0;
    }
    SOUNDS.reserve(((SOUND_INDEX+1)/256+1)*256);
    SOUNDS[SOUND_INDEX]=snd;
    return SOUND_INDEX++;
}

GMREAL __gm82audio_sfx_play(double soundid,double vol,double pan,double pitch,double loops) {
    cs_audio_source_t* snd=SOUNDS[(int)soundid];   
    cs_sound_params_t params=cs_sound_params_default();
    params.volume=vol;
    params.pan=pan;
    params.pitch=pitch;
    params.looped=(loops>=0.5);
    cs_play_sound(snd,params);
    return 0;
}

GMREAL __gm82audio_music_pause() {
    cs_music_pause();
    MUSIC_PAUSED=true;
    return 0;
}

GMREAL __gm82audio_music_resume() {
    cs_music_resume();
    MUSIC_PAUSED=false;
    return 0;
}

GMREAL __gm82audio_music_play(double soundid,double fadeintime,double vol,double pitch,double loops) {
    CURRENT_SONG=SOUNDS[(int)soundid];
    cs_music_play(CURRENT_SONG,(float)fadeintime);    
    __gm82audio_music_pitch(pitch);
    __gm82audio_music_volume(vol);
    __gm82audio_music_loop(loops);
    return 0;
}

GMREAL __gm82audio_music_switch(double soundid,double fadeouttime,double fadeintime,double vol,double pitch,double loops) {
    CURRENT_SONG=SOUNDS[(int)soundid];
    cs_music_switch_to(CURRENT_SONG,(float)fadeouttime,(float)fadeintime);
    __gm82audio_music_pitch(pitch);
    __gm82audio_music_volume(vol);
    __gm82audio_music_loop(loops);
    return 0;
}

GMREAL __gm82audio_music_crossfade(double soundid,double fadetime,double vol,double pitch,double loops) {
    CURRENT_SONG=SOUNDS[(int)soundid];
    cs_music_crossfade(CURRENT_SONG,(float)fadetime);
    __gm82audio_music_pitch(pitch);
    __gm82audio_music_volume(vol);
    __gm82audio_music_loop(loops);
    return 0;
}

GMREAL __gm82audio_music_pitch(double pitch) {
    if (fabs(pitch)<0.0078125f) {
        cs_music_pause();
    } else {
        if (!MUSIC_PAUSED) cs_music_resume();
        cs_music_set_pitch(pitch);
    }
    return 0;
}

GMREAL __gm82audio_music_volume(double volume) {
    cs_music_set_volume((float)volume);    
    return 0;
}

GMREAL __gm82audio_music_loop(double loops) {
    cs_music_set_loop(loops>=0.5);    
    return 0;
}

GMREAL __gm82audio_music_pan(double pan) {
    cs_music_set_pan(pan);    
    return 0;
}

GMREAL __gm82audio_music_get_pos() {
    return (double)(cs_music_get_sample_index()/((double)cs_get_sample_rate(CURRENT_SONG)));
}

GMREAL __gm82audio_music_set_pos(double pos) {
    cs_error_t error=cs_music_set_sample_index((int)(pos*cs_get_sample_rate(CURRENT_SONG)));
    if (error) {
        strcpy(ERROR_STR,cs_error_as_string(error));
        return 1;
    }
    return 0;
}

GMREAL __gm82audio_sound_get_length(double soundid) {
    cs_audio_source_t* snd=SOUNDS[(int)soundid];
    return (double)(cs_get_sample_count(snd)/((double)cs_get_sample_rate(snd)));
}

GMREAL __gm82audio_music_stop(double fadeouttime) {
    cs_music_stop(fadeouttime);
    return 0;
}

GMREAL __gm82audio_stop_all(double musictoo) {
    cs_stop_all_playing_sounds();
    if (musictoo>=0.5) cs_music_stop(0);
    return 0;
}
/*
soubnds from buffers
sfx instances usig std list
instance pause, loop, vol, pitch, pan, get sndid
stop instances
*/