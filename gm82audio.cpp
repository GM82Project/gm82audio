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
    sound_struct* sound=SOUNDS[(int)index];\
        
#define __CHECK_EXISTS_DEL(index,sound) \
    __CHECK_EXISTS(index,sound);\
    if (sound->deleted) return __ERROR_DELETED;


struct sound_struct {
    cs_audio_source_t* source;
    bool deleted=false;
    sound_struct(cs_audio_source_t* source): source(source){};
};


static double SAMPLE_RATE=44100;
static int SOUND_INDEX=1;
static std::vector<sound_struct*> SOUNDS;
static char* ERROR_STR="";
static bool MUSIC_PAUSED=false;
static cs_audio_source_t* CURRENT_MUSIC_SOURCE=NULL;


//initialization and system
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

int __gm82audio_store_sound(cs_audio_source_t* snd) {
    if (snd==NULL) return 0;
    SOUNDS.reserve((((SOUND_INDEX+1)/256)+1)*256);
    SOUNDS.push_back(new sound_struct(snd));
    return SOUND_INDEX++;
}


//general functions
GMREAL __gm82audio_load(char* fn,double type) {
    cs_error_t error;
    cs_audio_source_t* snd;
    if (type>=0.5) snd=cs_load_ogg(fn,&error);
    else snd=cs_load_wav(fn,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_load_mem(double gmbuffer,double length,double type) {
    cs_error_t error;
    cs_audio_source_t* snd;
    if (type>=0.5) snd=cs_read_mem_ogg((void*)(size_t)gmbuffer,(size_t)length,&error);
    else snd=cs_read_mem_wav((void*)(size_t)gmbuffer,(size_t)length,&error);
    if (snd==NULL) strcpy(ERROR_STR,cs_error_as_string(error));
    return __gm82audio_store_sound(snd);
}

GMREAL __gm82audio_exists(double index) {
    __CHECK_EXISTS_DEL(index,sound);
    return 1;
}

GMREAL __gm82audio_global_volume(double vol) {
    if (vol>1) vol=1;
    cs_set_global_volume(vol);
    return 0;
}

GMREAL __gm82audio_global_pause() {
    cs_set_global_pause(true);
    return 0;
}

GMREAL __gm82audio_global_resume() {
    cs_set_global_pause(false);
    return 0;
}

GMREAL __gm82audio_get_length(double soundid) {
    __CHECK_EXISTS_DEL(soundid,sound);
    return (double)(
        cs_get_sample_count(sound->source)/((double)cs_get_sample_rate(sound->source))
    );
}

GMREAL __gm82audio_stop_all(double musictoo) {
    cs_stop_all_playing_sounds();
    if (musictoo>=0.5) cs_music_stop(0);
    return 0;
}

GMREAL __gm82audio_delete(double soundid) {
    __CHECK_EXISTS(soundid,sound);
    if (!sound->deleted) {
        cs_free_audio_source(sound->source);
        sound->deleted=true;
    }
    return 0;
}


//music
GMREAL __gm82audio_music_volume(double volume) {
    cs_music_set_volume((float)volume);    
    return 0;
}

GMREAL __gm82audio_music_pitch(double pitch) {
    cs_music_set_pitch(pitch);
    return 0;
}

GMREAL __gm82audio_music_pan(double pan) {
    cs_music_set_pan(max(0,min(1,(pan+1)*0.5)));
    return 0;
}

GMREAL __gm82audio_music_loop(double loops) {
    cs_music_set_loop(loops>=0.5);    
    return 0;
}

GMREAL __gm82audio_music_set_all(double vol,double pan,double pitch,double loops) {
    cs_music_set_volume((float)vol); 
    cs_music_set_pitch(pitch);
    cs_music_set_pan(max(0,min(1,(pan+1)*0.5)));
    cs_music_set_loop(loops>=0.5);
    return 0;
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

GMREAL __gm82audio_music_stop(double fadeouttime) {
    cs_music_stop(fadeouttime);
    return 0;
}


//sounds
GMREAL __gm82audio_set_volume(double inst,double vol) {
    cs_sound_set_volume({(uint64_t)inst},vol);
    return 0;
}

GMREAL __gm82audio_set_pitch(double inst,double pitch) {
    cs_sound_set_pitch({(uint64_t)inst},pitch);
    return 0;
}

GMREAL __gm82audio_set_pan(double inst,double pan) {
    cs_sound_set_pan({(uint64_t)inst},max(0,min(1,(pan+1)/2)));
    return 0;
}

GMREAL __gm82audio_set_loop(double inst,double loops) {
    cs_sound_set_is_looped({(uint64_t)inst},loops>=0.5);
    return 0;
}

GMREAL __gm82audio_set_all(double inst,double vol,double pan,double pitch,double loops) {
    cs_playing_sound_t instance={(uint64_t)inst};
    cs_sound_set_volume(instance,vol);
    cs_sound_set_pan(instance,(pan+1)*0.5);
    cs_sound_set_pitch(instance,pitch);
    cs_sound_set_is_looped(instance,loops>=0.5);
    return 0;
}

GMREAL __gm82audio_sfx_play(double soundid,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    cs_audio_source_t* snd=sound->source;   
    cs_sound_params_t params=cs_sound_params_default();
    params.volume=vol;
    params.pan=(pan+1)*0.5;
    params.pitch=pitch;
    params.looped=(loops>=0.5);
    cs_playing_sound_t inst=cs_play_sound(snd,params);
    if (pitch<0) cs_sound_set_sample_index(inst,cs_get_sample_count(snd));
    return (double)(uint32_t)(inst.id);
}

GMREAL __gm82audio_sound_pause(double inst) {
    cs_sound_set_is_paused({(uint64_t)inst},true);
    return 0;
}

GMREAL __gm82audio_sound_resume(double inst) {
    cs_sound_set_is_paused({(uint64_t)inst},false);
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
            ))
        );
    }
    return 0;
}

GMREAL __gm82audio_sound_stop(double inst) {
    cs_sound_stop({(uint64_t)inst});
    return 0;
}

GMREAL __gm82audio_sound_stop_instances(double soundid) {
    __CHECK_EXISTS_DEL(soundid,sound);
    cs_stop_all_instances_of(sound->source);
    return 0;
}
