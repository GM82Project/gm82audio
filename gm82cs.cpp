#include "stb_vorbis.c"
//bruhhh
#undef L
#undef R
#undef C

#include "intrin.h"
#define CUTE_SOUND_IMPLEMENTATION
#include "cute_sound.h"

#include <map>

#define GMREAL extern "C" __declspec(dllexport) double __cdecl
#define GMSTR extern "C" __declspec(dllexport) char* __cdecl

GMREAL audio_init(double sample_rate);
GMREAL audio_update(double dt);
GMREAL audio_end();
GMREAL sound_add(char* fn);
GMREAL music_add(char* fn);
GMREAL sound_play(double soundid);
GMREAL music_play(double soundid);

static double SAMPLE_RATE;
static int SOUND_INDEX=1;
static std::map<int,cs_audio_source_t*> SOUNDS;

GMREAL audio_init(double sample_rate) {
    SAMPLE_RATE=sample_rate;
    cs_init(NULL,(int)SAMPLE_RATE,1024,NULL);
    cs_spawn_mix_thread();
    cs_mix_thread_sleep_delay(1);
    return 0;
}

GMREAL audio_update(double dt) {
    cs_update(dt);
    return 0;
}

GMREAL audio_end() {
    cs_shutdown();
    return 0;
}

GMREAL sound_add(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_wav(fn,&error);
    if (snd==NULL) {
        return 0;
    }
    SOUNDS.insert(std::make_pair(SOUND_INDEX,snd));
    return SOUND_INDEX++;
}

GMREAL music_add(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_ogg(fn,&error);
    if (snd==NULL) {
        return 0;
    }
    SOUNDS.insert(std::make_pair(SOUND_INDEX,snd));
    return SOUND_INDEX++;
}

GMREAL sound_play(double soundid) {
    cs_audio_source_t* snd=SOUNDS[(int)soundid];
    //compensate pitch
    int sndrate=cs_get_sample_rate(snd);
    cs_sound_params_t params=cs_sound_params_default();
    params.pitch=sndrate/SAMPLE_RATE;
    cs_play_sound(snd,params);
    return 0;
}

GMREAL music_play(double soundid) {
    cs_audio_source_t* snd=SOUNDS[(int)soundid];
    //compensate pitch
    int sndrate=cs_get_sample_rate(snd);
    cs_music_play(snd,1.0f);
    cs_music_set_pitch(sndrate/SAMPLE_RATE);
    return 0;
}
