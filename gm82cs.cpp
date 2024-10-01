#include "stb_vorbis.c"
//bruhhh
#undef L
#undef R
#undef C

#include "intrin.h"
#define CUTE_SOUND_IMPLEMENTATION
#include "cute_sound.h"

#include <map>

#define GMREAL extern "C" __declspec(dllexport) double 
#define GMSTR extern "C" __declspec(dllexport) char*

GMREAL __audio_init(double gm_hwnd,double sample_rate);
GMREAL __audio_update(double dt);
GMREAL __audio_end();
GMSTR  __audio_get_error();
GMREAL __audio_load_wav(char* fn);
GMREAL __audio_load_ogg(char* fn);
GMREAL __audio_play_sfx(double soundid,double vol,double pan,double pitch,double loops);
GMREAL __audio_play_music(double soundid,double vol,double pan,double pitch);
GMREAL __audio_music_pitch(double pitch);

static double SAMPLE_RATE;
static int SOUND_INDEX=1;
static std::map<int,cs_audio_source_t*> SOUNDS;
static char* ERROR_STR;
static int MUSIC_SAMPLERATE;

GMREAL __audio_init(double gm_hwnd,double sample_rate) {
    SAMPLE_RATE=sample_rate;
    cs_init((HWND)(int)gm_hwnd,(int)SAMPLE_RATE,1024,NULL);
    cs_spawn_mix_thread();
    cs_mix_thread_sleep_delay(1);
    return 0;
}

GMREAL __audio_update(double dt) {
    cs_update(dt);
    return 0;
}

GMREAL __audio_end() {
    cs_shutdown();
    return 0;
}

GMSTR  __audio_get_error() {
    return ERROR_STR;
}

GMREAL __audio_load_wav(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_wav(fn,&error);
    if (snd==NULL) {
        ERROR_STR = cs_error_as_string(error);
        return 0;
    }
    SOUNDS.insert(std::make_pair(SOUND_INDEX,snd));
    return SOUND_INDEX++;
}

GMREAL __audio_load_ogg(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_ogg(fn,&error);
    if (snd==NULL) {
        return 0;
    }
    SOUNDS.insert(std::make_pair(SOUND_INDEX,snd));
    return SOUND_INDEX++;
}

GMREAL __audio_play_sfx(double soundid,double vol,double pan,double pitch,double loops) {
    //TODO: vol pan loops
    cs_audio_source_t* snd=SOUNDS[(int)soundid];
    //compensate pitch
    int sndrate=cs_get_sample_rate(snd);
    cs_sound_params_t params=cs_sound_params_default();
    params.volume=
    params.pan=
    params.pitch=(sndrate/SAMPLE_RATE)*pitch;
    cs_play_sound(snd,params);
    return 0;
}

GMREAL __audio_play_music(double soundid,double vol,double pan,double pitch) {
    //TODO: vol pan
    cs_audio_source_t* snd=SOUNDS[(int)soundid];
    //compensate pitch
    MUSIC_SAMPLERATE=cs_get_sample_rate(snd);
    cs_music_play(snd,1.0f);
    cs_music_set_pitch(sndrate/SAMPLE_RATE);
    return 0;
}

GMREAL __audio_music_pitch(double pitch) {
    //TODO: check if the division here returns a float
    cs_music_set_pitch(pitch*(MUSIC_SAMPLERATE/SAMPLE_RATE));
}
