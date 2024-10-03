#define __gm82audio_init
    object_event_add(gm82core_object,ev_step,ev_step_end,"__gm82audio_step()")
    __gm82audio_init(window_handle())
    __gm82audio_initialized=true;    
    global.__gm82audio_last_update=get_timer()


#define __gm82audio_step
    var __now;__now=get_timer()
    __gm82audio_update((__now-global.__gm82audio_last_update)/1000)
    global.__gm82audio_last_update=__now


#define __gm82audio_check
    if (argument0==-1) {show_error("in function "+argument1+": sound "+string(argument2)+" does not exist!",0) return 0}
    if (argument0==-2) {show_error("in function "+argument1+": sound "+string(argument2)+" is deleted!",0) return 0}
    return 1


#define audio_load
    ///audio_load(filename)
    //filename: full or relative path to either a 16-bit wav file or an ogg file
    var snd;snd=noone;
    
    if (file_exists(argument0)) {
        ext=string_lower(filename_ext(argument0))
        
        if (ext==".wav") snd=__gm82audio_loadwav(argument0)
        if (ext==".ogg") snd=__gm82audio_loadogg(argument0)
        
        if (snd==noone)
            show_error("in function audio_load: error loading "+argument0+": unrecognized extension "+ext,0)
        if (snd==0)
            show_error("in function audio_load: error loading "+argument0+": "+__gm82audio_get_error(),0)
        
        return snd
    } else {    
        show_error("in function audio_load: error loading "+argument0+": file does not exist",0)
        return 0
    }


#define audio_sound_play
    ///audio_sound_play(sound)
    __gm82audio_check(__gm82audio_sfx_play(argument0,1,0.5,1,0),"audio_sound_play",argument0)


#define audio_sound_play_ext
    ///audio_sound_play_ext(sound,vol,pan,pitch,loop)
    __gm82audio_check(__gm82audio_sfx_play(
        argument0,
        median(0,argument1,1),
        median(0,argument2/2+0.5,1),
        median(-2,argument3,2),
        argument4
    ),"audio_sound_play_ext",argument0)


#define audio_music_play
    ///audio_music_play(sound,[fadeintime])
    var __fade;__fade=0
    
    if (argument_count==0 || argument_count>2) {
        show_error("in function audio_music_play: wrong number of arguments",0)
        exit
    }
    if (argument_count==2) __fade=max(0,argument[1])
    __gm82audio_check(__gm82audio_music_play(argument[0],__fade,1,1,1),"audio_music_play",argument0)


#define audio_music_play_ext
    ///audio_music_play_ext(sound,fadeintime,vol,pitch,loop)
    __gm82audio_check(__gm82audio_music_play(
        argument0,
        argument1,
        median(0,argument2,1),
        median(-2,argument3,2),
        argument4>=0.5
    ),"audio_music_play_ext",argument0)


#define audio_music_crossfade
    ///audio_music_crossfade(sound,fadetime)
    __gm82audio_check(__gm82audio_music_crossfade(argument0,argument1,1,1,1),"audio_music_crossfade",argument0)


#define audio_music_crossfade_ext
    ///audio_music_crossfade_ext(sound,fadetime,vol,pitch,loop)
    __gm82audio_check(__gm82audio_music_crossfade(
        argument0,
        argument1,
        median(0,argument2,1),
        median(-2,argument3,2),
        argument4>=0.5
    ),"audio_music_crossfade_ext",argument0)


#define audio_music_switch
    ///audio_music_switch(sound,fadeouttime,fadeintime)
    __gm82audio_check(__gm82audio_music_switch(argument0,argument1,argument2,1,1,1),"audio_music_switch",argument0)


#define audio_music_switch_ext
    ///audio_music_switch_ext(sound,fadeouttime,fadeintime,vol,pitch,loop)
    __gm82audio_check(__gm82audio_music_switch(
        argument0,
        argument1,
        argument2,
        median(0,argument3,1),
        median(-2,argument4,2),
        argument5>=0.5
    ),"audio_music_switch_ext",argument0)


#define audio_music_pitch
    ///audio_music_pitch(pitch)
    __gm82audio_music_pitch(median(-2,argument0,2))


#define audio_music_pan
    ///audio_music_pan(pan)
    __gm82audio_music_pan(median(0,argument0/2+0.5,1))


#define audio_music_set_pos
    ///audio_music_set_pos(pos)
    if (__gm82audio_music_set_pos(argument0))
        show_error("in function audio_music_set_pos: "+__gm82audio_get_error(),0)


#define audio_music_volume
    ///audio_music_volume(volume)
    __gm82audio_music_volume(median(0,argument0,1))


#define audio_music_set_loop
    ///audio_music_set_loop(loops)
    __gm82audio_music_loop(argument0)


#define audio_music_stop
    ///audio_music_stop([fadeouttime])
    var __fade;__fade=0
    
    if (argument_count>1) {
        show_error("in function audio_music_stop: wrong number of arguments",0)
        exit
    }
    if (argument_count==1) __fade=max(0,argument[0])
    __gm82audio_music_stop(__fade)


#define audio_stop_all
    ///audio_stop_all([musictoo])
    if (argument_count) __gm82audio_stop_all(argument[0])
    else __gm82audio_stop_all(0)
//
//