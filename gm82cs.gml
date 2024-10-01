#define audio_init
    ///audio_init(sample_rate)
    //sample_rate: audio mixer sample rate (8000-88200)
    //initialize the audio engine with a fixed sample rate. normally you want to use 44100.
    object_event_add(gm82core_object,ev_step,ev_step_end,"audio_update()")
    global.__audio_last_update=noone
    globalvar __audio_initialized;
    if (!__audio_initialized) __audio_init(argument0)
    else show_error("audio_init was called more than once!",0)
    __audio_initialized=true;


#define audio_update
    ///audio_update()
    //update the audio engine to f.ex. update music volume
    
    var __now=get_timer()
    
    if (global.__audio_last_update==noone)
        global.__audio_last_update=__now-1000/room_speed
    
    __cs_audio_update(__now-global.__audio_last_update)
    
    global.__audio_last_update=__now


#define audio_load_wav
    ///audio_load_wav(filename)
    //filename: full or relative path to a 16-bit wav file
    if (file_exists(argument0))
        snd=__audio_load_wav(argument0)
    if (!snd) show_error("error loading "+argument0+": "+__audio_get_error(),0)
    return snd


#define audio_load_ogg
    ///audio_load_ogg(filename)
    //filename: full or relative path to an ogg file
    if (file_exists(argument0))
        snd=__audio_load_ogg(argument0)
    if (!snd) show_error("error loading "+argument0+": "+__audio_get_error(),0)
    return snd


#define audio_play_sfx
    ///audio_play_sfx(sound,vol,pan,pitch,loops)
    if (argument0) __audio_play_sfx(argument0,argument1,argument2,argument3,argument4)


#define audio_play_music
    ///audio_play_music(sound,vol,pan,pitch)
    if (argument0) __audio_play_music(argument0,argument1,argument2,argument3)
//
//