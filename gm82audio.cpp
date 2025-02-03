#include "stb_vorbis.c"
//bruhhh
#undef L
#undef R
#undef C

#include "intrin.h"
#define CUTE_SOUND_IMPLEMENTATION
#include "cute_sound.h"

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#define GMREAL extern "C" __declspec(dllexport) double __cdecl 
#define GMSTR extern "C" __declspec(dllexport) char* __cdecl

#define __ERROR_NONEXIST    -0x1000001
#define __ERROR_DELETED     -0x1000002
#define __ERROR_FAIL_LOAD   -0x1000003

#define __CHECK_EXISTS(index,sound) \
if (index<0 || index>=SOUND_INDEX) return __ERROR_NONEXIST;\
    sound_struct* sound=SOUNDS[(int)index];\
    if (sound==NULL) return __ERROR_NONEXIST;
        
#define __CHECK_EXISTS_DEL(index,sound) \
    __CHECK_EXISTS(index,sound);\
    if (sound->deleted) return __ERROR_DELETED;


//game maker 8.1 sound memory structures
struct TMemoryStream {
    uint32_t vfp;
    void* memory;
    uint32_t size;
    uint32_t position;
    uint32_t capacity;
};

struct GMSound {
    uint32_t vfp;
    uint32_t kind;
    char* extension;
    char* origname;
    TMemoryStream* memstream;
    uint32_t preload;
    uint32_t effects;
    double volume;
    double pan;
    uint32_t index;
    wchar_t* fname;
};

//game maker 8.1 runner memory locations
static GMSound*** gm_sound_mem = (GMSound***)0x6840c0;
static uint32_t* gm_sound_count = (uint32_t*)0x6840c8;


//audio extension globals
struct sound_struct {
    cs_audio_source_t* source;
    double volume;
    double pan;
	double pitch;
    bool deleted=false;
    sound_struct(cs_audio_source_t* source, double volume = 1, double pan = 0, double pitch = 1): source(source), volume(volume), pan(pan), pitch(pitch){};
};

static double SAMPLE_RATE=44100;
static int SOUND_INDEX=0;
static int BUILTIN_COUNT;
static std::vector<sound_struct*> SOUNDS;
static char ERROR_STR[255];
static bool MUSIC_PAUSED=false;
static cs_audio_source_t* CURRENT_MUSIC_SOURCE=NULL;

GMREAL __gm82audio_load_builtin(double);

//initialization and system
GMREAL __gm82audio_init(double gm_hwnd) {
    cs_error_t error = cs_init((HWND)(int)gm_hwnd,(int)SAMPLE_RATE,1024,NULL);
    if (error) {
        MessageBoxA(NULL,cs_error_as_string(error),"gm82audio error!",MB_OK|MB_ICONSTOP);
        exit(1);
        return 0;
    }
    cs_spawn_mix_thread();
    cs_mix_thread_sleep_delay(1);
    
    //reserve space for builtin sounds
    BUILTIN_COUNT=*gm_sound_count;
    SOUND_INDEX=BUILTIN_COUNT;
    SOUNDS.reserve(SOUND_INDEX+256);
    
    //preload any builtin sounds
    GMSound* sound;
    for (int i=0;i<SOUND_INDEX;i+=1) {
        SOUNDS.push_back(NULL);
        sound=(*gm_sound_mem)[i];
        if (sound) if (sound->preload) {
            __gm82audio_load_builtin(i);
        }
    }
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
    if (snd==NULL) return __ERROR_FAIL_LOAD;
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

GMREAL __gm82audio_load_builtin(double index) {
    cs_error_t error;
    cs_audio_source_t* snd;
    
    //grab the gm sound struct's memory stream by traversing memory
    GMSound* sound=(*gm_sound_mem)[(int)index];
    TMemoryStream* memstream=sound->memstream;
    
    if (memstream==NULL) {
        //if there is no memory stream in the sound struct, then that means
        //we are looking at an exported "external codec" sound file...
        
        //or an empty sound resource.
        if (sound->fname==NULL) return 0;
        
        void* data = NULL;
        int size;

        //the path to the temp file is stored on this field
        FILE* fp = _wfopen(sound->fname, L"rb");

        //load the file data
        if (fp) {
            fseek(fp, 0, SEEK_END);
            size = (int)ftell(fp);
            fseek(fp, 0, SEEK_SET);
            data = malloc(size);
            fread(data, size, 1, fp);
            fclose(fp);
        }

        if (!data) {
            strcpy(ERROR_STR,"somehow the file data was null? tell renex about this.");
            return NULL;
        }

        if (memcmp("wav",sound->extension,3)) snd=cs_read_mem_wav(data,size,&error);
        if (memcmp("ogg",sound->extension,3)) snd=cs_read_mem_ogg(data,size,&error);
        
        free(data);
    } else {
        //it's a valid builtin sound type loaded in memory, we can just grab it...
        void* buffer=sound->memstream->memory;
        int length=sound->memstream->size;
        if (cs_four_cc("RIFF", buffer)) snd=cs_read_mem_wav(buffer,length,&error);
        if (cs_four_cc("OggS", buffer)) snd=cs_read_mem_ogg(buffer,length,&error);
    }
    
    if (snd==NULL) {
        strcpy(ERROR_STR,cs_error_as_string(error));
        return 0;
    }
    
    sound_struct* existing=SOUNDS[(int)index];
    
    if (existing!=NULL) {       
        if (!existing->deleted) {
            cs_free_audio_source(existing->source);
        }
        free(existing);
    }
    SOUNDS[(int)index]=new sound_struct(snd, (sound->volume-0.3)/0.7, sound->pan/2.0+0.5);
    
    return 1;
}

GMREAL __gm82audio_get_builtin_count() {
    return BUILTIN_COUNT;
}

GMREAL __gm82audio_exists(double index) {
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        return 1;
    } else {
        return cs_sound_is_active({(uint64_t)-index})?1:0;
    }    
}

GMREAL __gm82audio_global_volume(double vol) {
    ///audio_global_volume(volume)
    //volume: gain value
    //Sets the global volume for the audio engine.
    cs_set_global_volume(vol);
    return 0;
}

GMREAL __gm82audio_global_pause() {
    ///audio_global_pause()
    //Pauses execution of the audio engine.
    cs_set_global_pause(true);
    return 0;
}

GMREAL __gm82audio_global_resume() {
    ///audio_global_resume()
    //Resumes execution of the audio engine.
    cs_set_global_pause(false);
    return 0;
}

GMREAL __gm82audio_stop_all(double musictoo) {
    cs_stop_all_playing_sounds();
    if (musictoo>=0.5) cs_music_stop(0);
    return 0;
}

GMREAL __gm82audio_sound_stop(double index) {
    ///audio_stop(sound/inst)
    //sound/inst: sound index or instance to stop
    //Deletes a sound instance, or all instances of a sound.
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        cs_stop_all_instances_of(sound->source);
        return 0;
    }
    cs_sound_stop({(uint64_t)-index});
    return 0;
}

GMREAL __gm82audio_isplaying(double index) {
    ///audio_isplaying(sound/inst)
    //sound/inst: sound index or instance to check
    //Returns the number of instances of a sound, or if the instance is active.
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        return sound->source->playing_count;
    }
    return cs_sound_is_active({(uint64_t)-index});    
}

GMREAL __gm82audio_get_def_vol(double soundid) {
    return SOUNDS[(int)soundid]->volume;
}

GMREAL __gm82audio_get_def_pan(double soundid) {
    return SOUNDS[(int)soundid]->pan;
}

GMREAL __gm82audio_get_def_pitch(double soundid) {
    return SOUNDS[(int)soundid]->pitch;
}


//music
GMREAL __gm82audio_music_volume(double volume) {
    ///audio_music_volume(volume)
    //volume: gain value
    //Changes the volume of the music. Volume above 1 is accepted.
    cs_music_set_volume((float)volume);    
    return 0;
}

GMREAL __gm82audio_music_pitch(double pitch) {
    ///audio_music_pitch(pitch)
    //pitch: pitch shifting factor
    //Changes the pitch of the music. Negative pitch is accepted.
    cs_music_set_pitch(pitch);
    return 0;
}

GMREAL __gm82audio_music_pan(double pan) {
    ///audio_music_pan(pan)
    //pan: panning value
    //Changes the horizontal positioning of the music.
    cs_music_set_pan(max(0,min(1,(pan+1)*0.5)));
    return 0;
}

GMREAL __gm82audio_music_loop(double loops) {
    ///audio_music_loop(enabled)
    //enabled: boolean
    //Enables or disables looping the music.
    cs_music_set_loop(loops>=0.5);    
    return 0;
}

GMREAL __gm82audio_music_set_all(double vol,double pan,double pitch,double loops) {
    ///audio_music_set_all(vol,pan,pitch,loops)
    //vol: gain factor
    //pan: panning value
    //pitch: pitch shifting factor
    //loops: boolean
    //Changes all properties of music at once.
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
    return 1;
}

GMREAL __gm82audio_music_switch(double soundid,double fadeouttime,double fadeintime,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    CURRENT_MUSIC_SOURCE=sound->source;
    cs_music_switch_to(CURRENT_MUSIC_SOURCE,(float)fadeouttime,(float)fadeintime);
    __gm82audio_music_set_all(vol,pan,pitch,loops);
    return 1;
}

GMREAL __gm82audio_music_crossfade(double soundid,double fadetime,double vol,double pan,double pitch,double loops) {
    __CHECK_EXISTS_DEL(soundid,sound);
    CURRENT_MUSIC_SOURCE=sound->source;
    cs_music_crossfade(CURRENT_MUSIC_SOURCE,(float)fadetime);
    __gm82audio_music_set_all(vol,pan,pitch,loops);
    return 1;
}

GMREAL __gm82audio_music_pause() {
    ///audio_music_pause()
    //Pauses playback of the music.
    cs_music_pause();
    MUSIC_PAUSED=true;
    return 0;
}

GMREAL __gm82audio_music_resume() {
    ///audio_music_resume()
    //Resumes playback of the music.
    cs_music_resume();
    MUSIC_PAUSED=false;
    return 0;
}

GMREAL __gm82audio_music_get_pos() {
    ///audio_music_get_pos()
    //returns: current music position in seconds
    if (!CURRENT_MUSIC_SOURCE) return 0;
    return (double)(
        cs_music_get_sample_index()/((double)cs_get_sample_rate(CURRENT_MUSIC_SOURCE))
    );
}

GMREAL __gm82audio_music_set_pos(double pos) {
    ///audio_music_set_pos(pos)
    //pos: position in seconds
    //Changes the current position of the music, in seconds.
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
GMREAL __gm82audio_set_sfx_volume(double vol) {
    ///audio_sound_volume(volume)
    //volume: gain factor
    //Sets the volume for sound effect mixing.
    cs_set_playing_sounds_volume(vol);
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
    return -(double)(uint32_t)(inst.id);
}

GMREAL __gm82audio_get_length(double soundid) {
    __CHECK_EXISTS_DEL(soundid,sound);
    return (double)(
        cs_get_sample_count(sound->source)/((double)cs_get_sample_rate(sound->source))
    );
}

GMREAL __gm82audio_set_loop_points(double soundid,double pointa, double pointb) {
    __CHECK_EXISTS_DEL(soundid,sound);
    uint32_t sr=cs_get_sample_rate(sound->source);
    uint32_t sc=cs_get_sample_count(sound->source)-1;
    
    uint32_t _a=(uint32_t)(sr*pointa);
    uint32_t _b=(uint32_t)(sr*pointb);
    
    //i pass -1 when point b isn't set, use end of file
    if (pointb<0) _b=sc;
    
    //range check to avoid errors
    _a=max(0,min(sc-1,_a));
    _b=max(_a+1,min(sc,_b));
    
    cs_set_loop_points(sound->source,_a,_b);
    return 0;
}

GMREAL __gm82audio_delete(double soundid) {
    ///audio_delete(sound)
    //sound: sound to delete
    //Deletes a sound. New sounds don't reuse ids.
    //If an instance of the sound is still playing, it finishes playing.
    __CHECK_EXISTS(soundid,sound);
    if (!sound->deleted) {
        cs_free_audio_source(sound->source);
        sound->deleted=true;
    }
    return 0;
}

GMREAL __gm82audio_set_volume(double index,double vol) {
    ///audio_set_volume(index,volume)
    //index: sound index or audio instance
    //volume: gain factor
    //Changes the volume of a sound source or audio instance. Volume above 1 is accepted.
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        SOUNDS[(int)index]->volume=vol;
        return 0;
    }    
    
    cs_sound_set_volume({(uint64_t)-index},vol);
    return 0;
}

GMREAL __gm82audio_set_pitch(double index,double pitch) {
    ///audio_set_pitch(index,pitch)
    //inst: sound index or audio instance
    //pitch: pitch shifting factor
    //Changes the pitch of a sound source or an audio instance. Negative pitch is accepted.
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        SOUNDS[(int)index]->pitch=pitch;
        return 0;
    }
    
    cs_sound_set_pitch({(uint64_t)-index},pitch);
    return 0;
}

GMREAL __gm82audio_set_pan(double index,double pan) {
    ///audio_set_pan(index,pan)
    //inst: sound index or audio instance
    //pan: panning value
    //Changes the horizontal positioning of a sound source or an audio instance.
    if (index>=0) {
        __CHECK_EXISTS_DEL(index,sound);
        SOUNDS[(int)index]->pan=pan;
        return 0;
    }
    
    cs_sound_set_pan({(uint64_t)-index},max(0,min(1,(pan+1)/2)));
    return 0;
}

//instances
GMREAL __gm82audio_set_loop(double inst,double loops) {
    ///audio_set_loop(inst,enabled)
    //inst: audio instance
    //enabled: boolean
    //Enables or disables looping a sound instance.
    if (inst>=0) return 0;
    cs_sound_set_is_looped({(uint64_t)-inst},loops>=0.5);
    return 0;
}

GMREAL __gm82audio_set_all(double inst,double vol,double pan,double pitch,double loops) {
    ///audio_set_all(inst,vol,pan,pitch,loops)
    //inst: audio instance
    //vol: gain factor
    //pan: panning value
    //pitch: pitch shifting factor
    //loops: boolean
    //Changes all properties of a sound instance at once.    
    if (inst>=0) return 0;
    cs_playing_sound_t instance={(uint64_t)-inst};
    cs_sound_set_volume(instance,vol);
    cs_sound_set_pan(instance,(pan+1)*0.5);
    cs_sound_set_pitch(instance,pitch);
    cs_sound_set_is_looped(instance,loops>=0.5);
    return 0;
}

GMREAL __gm82audio_sound_pause(double inst) {
    ///audio_pause(inst)
    //inst: sound instance
    //Pauses playback of a sound instance.
    if (inst>=0) return 0;
    cs_sound_set_is_paused({(uint64_t)-inst},true);
    return 0;
}

GMREAL __gm82audio_sound_resume(double inst) {
    ///audio_resume(inst)
    //inst: sound instance
    //Resumes playback of a sound instance.
    if (inst>=0) return 0;
    cs_sound_set_is_paused({(uint64_t)-inst},false);
    return 0;
}

GMREAL __gm82audio_get_pos(double inst) {
    ///audio_get_pos(inst)
    //inst: sound instance
    //returns: the current position of the sound instance, in seconds
    if (inst>=0) return 0;
    cs_sound_inst_t* instance = s_get_inst({(uint64_t)-inst});
    if (instance) return (double)(
        instance->sample_index/((double)cs_get_sample_rate(instance->audio))
    );
    return 0;
}

GMREAL __gm82audio_set_pos(double inst,double pos) {
    ///audio_set_pos(inst,pos)
    //inst: sound instance
    //pos: position in seconds
    //Changes the current position of the sound instance, in seconds.
    if (inst>=0) return 0;
    cs_sound_inst_t* instance = s_get_inst({(uint64_t)-inst});
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

GMREAL __gm82audio_instance_count() {
    int count=0;
    for (int i=0;i<SOUND_INDEX;i++) {
        auto sound=SOUNDS[i];
        if (sound) count+=sound->source->playing_count;
    }
    
    return count;
}