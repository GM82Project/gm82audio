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
    if (argument0==-1) {
        show_error("in function "+argument1+": sound "+string(argument2)+" does not exist!",0)
        return 0
    }
    if (argument0==-2) {
        show_error("in function "+argument1+": sound "+string(argument2)+" is deleted!",0)
        return 0
    }
    return 1


#define audio_load
    ///audio_load(filename)
    //filename: full or relative path to either a 16-bit wav or an ogg file
    var __snd;__snd=noone;
    var __erstr;__erstr="in function audio_load: error loading "+argument0+": "
    
    if (file_exists(argument0)) {
        __b=buffer_create()
        buffer_load_part(__b,argument[0],0,4)
        buffer_set_pos(__b,0)
        var __fourcc;__fourcc=buffer_read_data(__b,4)
        buffer_destroy(__b)
        
        if (__fourcc=="RIFF") __snd=__gm82audio_load(argument0,0)
        if (__fourcc=="OggS") __snd=__gm82audio_load(argument0,1)
        
        if (__snd==noone) show_error(__erstr+"unrecognized audio format "+__fourcc,0)
        if (__snd==0) show_error(__erstr+__gm82audio_get_error(),0)
    } else {    
        show_error(__erstr+"file does not exist",0)
    }
    return __snd


#define audio_load_buffer
    ///audio_load_buffer(buffer)
    //buffer: a handle to a gm82net buffer
    var __snd;__snd=noone;
    var __erstr;__erstr="in function audio_load_buffer: "

    if (buffer_exists(argument0)) {
        buffer_set_pos(argument0,0)
        var __fourcc;__fourcc=buffer_read_data(argument0,4)
        var __addr;__addr=buffer_get_address(argument0,0)
        var __size;__size=buffer_get_size(argument0)
        
        if (__fourcc=="RIFF") __snd=__gm82audio_load_mem(__addr,__size,0)
        if (__fourcc=="OggS") __snd=__gm82audio_load_mem(__addr,__size,1)
        
        if (__snd==noone) show_error(__erstr+"unrecognized audio format "+__fourcc,0)
        if (__snd==0) show_error(__erstr+__gm82audio_get_error(),0)
    } else {    
        show_error(__erstr+"buffer does not exist",0)
    }
    return __snd


#define audio_play
    ///audio_play(sound)
    var __call;__call=__gm82audio_sfx_play(argument0,1,0,1,0)
    __gm82audio_check(__call,"audio_play",argument0)
    return __call


#define audio_play_ext
    ///audio_play_ext(sound,vol,pan,pitch,loop)
    var __call;__call=__gm82audio_sfx_play(
        argument0,argument1,argument2,argument3,argument4
    )
    __gm82audio_check(__call,"audio_play_ext",argument0)
    return __call


#define audio_music_play
    ///audio_music_play(sound,[fadeintime])
    var __fade;__fade=0
    
    if (argument_count==0 || argument_count>2) {
        show_error("in function audio_music_play: wrong number of arguments",0)
        exit
    }
    if (argument_count==2) __fade=max(0,argument[1])
    __gm82audio_check(__gm82audio_music_play(
        argument[0],__fade,1,0,1,1
    ),"audio_music_play",argument0)


#define audio_music_play_ext
    ///audio_music_play_ext(sound,fadeintime,vol,pan,pitch,loop)
    __gm82audio_check(__gm82audio_music_play(
        argument0,argument1,argument2,argument3,argument4,argument5>=0.5
    ),"audio_music_play_ext",argument0)


#define audio_music_crossfade
    ///audio_music_crossfade(sound,fadetime)
    __gm82audio_check(__gm82audio_music_crossfade(
        argument0,argument1,1,0,1,1
    ),"audio_music_crossfade",argument0)


#define audio_music_crossfade_ext
    ///audio_music_crossfade_ext(sound,fadetime,vol,pan,pitch,loop)
    __gm82audio_check(__gm82audio_music_crossfade(
        argument0,argument1,argument2,argument3,argument4,argument5>=0.5
    ),"audio_music_crossfade_ext",argument0)


#define audio_music_switch
    ///audio_music_switch(sound,fadeouttime,[fadeintime])
    var __fadein;
    
    if (argument_count<2 || argument_count>3) {
        show_error("in function audio_music_switch: wrong number of arguments",0)
        exit
    }
    
    __fadein=argument[1]
    if (argument_count==3) __fadein=argument[2]
    
    __gm82audio_check(__gm82audio_music_switch(
        argument[0],argument[1],__fadein,1,0,1,1
    ),"audio_music_switch",argument[0])


#define audio_music_switch_ext
    ///audio_music_switch_ext(sound,fadeouttime,fadeintime,vol,pan,pitch,loop)
    __gm82audio_check(__gm82audio_music_switch(
        argument0,argument1,argument2,argument3,argument4,argument5,argument6>=0.5
    ),"audio_music_switch_ext",argument0)


#define audio_music_stop
    ///audio_music_stop([fadeouttime])
    var __fade;__fade=0
    
    if (argument_count>1) {
        show_error("in function audio_music_stop: wrong number of arguments",0)
        exit
    }
    
    if (argument_count==1) __fade=max(0,argument[0])
    __gm82audio_music_stop(__fade)


#define audio_global_stop
    ///audio_global_stop([music too])
    if (argument_count) __gm82audio_stop_all(argument[0])
    else __gm82audio_stop_all(0)

//
//

/*
TODO
- loop points?
- implement renex engine pack file support 
*/