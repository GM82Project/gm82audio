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

#define __ERROR_NONEXIST -1
#define __ERROR_DELETED -2

#define __CHECK_EXISTS(index,sound) \
if (index<=0 || index>=SOUND_INDEX) return __ERROR_NONEXIST;\
    Sound sound=SOUNDS[(int)index];\
        
#define __CHECK_EXISTS_DEL(index,sound) \
    __CHECK_EXISTS(index,sound);\
    if (sound->deleted) return __ERROR_DELETED;


struct sound_struct {
    cs_audio_source_t* source;
    bool deleted=false;
    sound_struct(cs_audio_source_t* source): source(source){};
}; typedef sound_struct* Sound;

static double SAMPLE_RATE=44100;
static int SOUND_INDEX=1;
static std::vector<Sound> SOUNDS;
static char* ERROR_STR="";
static bool MUSIC_PAUSED=false;
static cs_audio_source_t* CURRENT_MUSIC_SOURCE=NULL;

GMREAL __gm82audio_init(double gm_hwnd) {
    cs_init((HWND)(int)gm_hwnd,(int)SAMPLE_RATE,1024,NULL);
    cs_spawn_mix_thread();
    cs_mix_thread_sleep_delay(1);
    SOUNDS.push_back(NULL);
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

GMREAL __gm82audio_exists(double index) {
    __CHECK_EXISTS_DEL(index,sound);
    return 1;
}

int __gm82audio_store_sound(cs_audio_source_t* snd) {
    if (snd==NULL) return 0;
    SOUNDS.reserve((((SOUND_INDEX+1)/256)+1)*256);
    SOUNDS.push_back(new sound_struct(snd));
    return SOUND_INDEX++;
}

GMREAL __gm82audio_loadwav(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_wav(fn,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_loadogg(char* fn) {
    cs_error_t error;
    cs_audio_source_t* snd=cs_load_ogg(fn,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_loadwav_mem(double gmbuffer,double length) {
    cs_error_t error;
    const void* data=(void*)(size_t)gmbuffer;
    cs_audio_source_t* snd=cs_read_mem_wav(data,(size_t)length,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_loadogg_mem(double gmbuffer,double length) {
    cs_error_t error;
    const void* data=(void*)(size_t)gmbuffer;
    cs_audio_source_t* snd=cs_read_mem_ogg(data,(size_t)length,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_sfx_play(double soundid,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    cs_audio_source_t* snd=sound->source;   
    cs_sound_params_t params=cs_sound_params_default();
    params.volume=vol;
    params.pan=pan;
    params.pitch=pitch;
    params.looped=(loops>=0.5);
    cs_playing_sound_t inst=cs_play_sound(snd,params);
    return (double)(uint32_t)(inst.id);
}

GMREAL __gm82audio_music_pause(double pause) {
    if (pause>=0.5) {
        cs_music_pause();
        MUSIC_PAUSED=true;
    } else {
        cs_music_resume();
        MUSIC_PAUSED=false;
    }
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

void __gm82audio_music_set_all(double vol,double pan,double pitch,double loops) {
    __gm82audio_music_volume(vol);
    __gm82audio_music_pan(pan);
    __gm82audio_music_pitch(pitch);
    __gm82audio_music_loop(loops);
}

GMREAL __gm82audio_music_play(double soundid,double fadeintime,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    CURRENT_MUSIC_SOURCE=sound->source;
    cs_music_play(CURRENT_MUSIC_SOURCE,(float)fadeintime);    
    __gm82audio_music_set_all(vol,pan,pitch,loops);
    return 0;
}

GMREAL __gm82audio_music_switch(double soundid,double fadeouttime,double fadeintime,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    CURRENT_MUSIC_SOURCE=sound->source;
    cs_music_switch_to(CURRENT_MUSIC_SOURCE,(float)fadeouttime,(float)fadeintime);
    __gm82audio_music_set_all(vol,pan,pitch,loops);
    return 0;
}

GMREAL __gm82audio_music_crossfade(double soundid,double fadetime,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    CURRENT_MUSIC_SOURCE=sound->source;
    cs_music_crossfade(CURRENT_MUSIC_SOURCE,(float)fadetime);
    __gm82audio_music_set_all(vol,pan,pitch,loops);
    return 0;
}

GMREAL __gm82audio_music_get_pos() {
    if (!CURRENT_MUSIC_SOURCE) return 0;
    return (double)(
        cs_music_get_sample_index()/((double)cs_get_sample_rate(CURRENT_MUSIC_SOURCE))
    );
}

GMREAL __gm82audio_music_set_pos(double pos) {
    cs_music_set_sample_index(
        max(0,min(cs_get_sample_count(CURRENT_MUSIC_SOURCE),
            (int)(pos*cs_get_sample_rate(CURRENT_MUSIC_SOURCE))
        ))
    );
    return 0;
}

GMREAL __gm82audio_get_length(double soundid) {
    __CHECK_EXISTS_DEL(soundid,sound);
    return (double)(
        cs_get_sample_count(sound->source)/((double)cs_get_sample_rate(sound->source))
    );
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

GMREAL __gm82audio_global_volume(double vol) {
    if (vol>1) vol=1;
    cs_set_global_volume(vol);
    return 0;
}

GMREAL __gm82audio_sound_pause(double inst,double paused) {
    cs_sound_set_is_paused({(uint64_t)inst},paused>=0.5?true:false);
    return 0;
}

GMREAL __gm82audio_get_pos(double inst) {
    cs_sound_inst_t* instance = s_get_inst({(uint64_t)inst});
    if (instance) return (double)(
        instance->sample_index/((double)cs_get_sample_rate(instance->audio))
    );
    return 0;
}

GMREAL __gm82audio_set_pos(double inst,double pos) {
    cs_sound_inst_t* instance = s_get_inst({(uint64_t)inst});
    if (instance) {
        cs_sound_set_sample_index(
            {(uint64_t)inst},
            max(0,min(cs_get_sample_count(instance->audio),
                (int)(pos*cs_get_sample_rate(instance->audio))
            )
        );
    }
    return 0;
}

GMREAL __gm82audio_unload(double soundid) {
    __CHECK_EXISTS(soundid,sound);
    if (!sound->deleted) {
        cs_free_audio_source(sound->source);
        sound->deleted=true;
    }
    return 0;
}
