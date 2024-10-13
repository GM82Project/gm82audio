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
    if (argument0==-$1000001) {
        show_error("in function "+argument1+": sound "+string(argument2)+" does not exist!",0)
        return 0
    }
    if (argument0==-$1000002) {
        show_error("in function "+argument1+": sound "+string(argument2)+" is deleted!",0)
        return 0
    }
    return 1


#define audio_preload_sound
    ///audio_preload_sound(gmsound)
    //gmsound: built-in sound resource id
    //returns: 0 on success, 1 on error
    //Preloads a builtin game maker sound for use in the audio_ functions.
    //Prevents a lag spike when the sound is used for the first time.
    if (argument0<__gm82audio_get_builtin_count()) {
        if (sound_exists(argument0)) {
            status=__gm82audio_exists(argument0)
            if (status==-$1000001 || status==-$1000002) {
                //make sure the sound is preloaded or exported
                sound_restore(argument0)
                if (!__gm82audio_load_builtin(argument0)) {
                    show_error("error preloading builtin sound: "+__gm82audio_get_error(),0)
                    return 1
                } else {
                    //we don't need it anymore
                    sound_discard(argument0)
                }
            }
        } else {
            show_error("builtin sound index "+string(argument0)+" does not exist",0)
            return 1
        }
    }
    return 0


#define audio_load
    ///audio_load(filename)
    //filename: full or relative path to a sound file
    //returns: sound index
    //Loads a sound file. wav files and ogg vorbis are supported.
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
    //returns: sound index
    //Loads a sound from a buffer. wav files and ogg vorbis are supported.
    //You can delete the buffer afterwards.    
    var __snd;__snd=noone;
    var __erstr;__erstr="in function audio_load_buffer: "

    if (buffer_exists(argument0)) {
        var __fourcc;__fourcc=
            chr(buffer_peek(argument0,0))+
            chr(buffer_peek(argument0,1))+
            chr(buffer_peek(argument0,2))+
            chr(buffer_peek(argument0,3))
        
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
    //sound: sound index to play
    //returns: instance id
    //Plays a sound and returns an instance id.
    if (audio_preload_sound(argument0)) exit
    var __call;__call=__gm82audio_sfx_play(argument0,__gm82audio_get_def_vol(argument0),__gm82audio_get_def_pan(argument0),1,0)
    __gm82audio_check(__call,"audio_play",argument0)
    return __call


#define audio_play_single
    ///audio_play_single(sound)
    //sound: sound index to play
    //returns: instance id
    //Plays a single instance of the sound and returns an instance id.
    audio_stop(argument0)
    return audio_play(argument0)


#define audio_loop
    ///audio_loop(sound)
    //sound: sound index to loop
    //returns: instance id
    //Loops a sound and returns an instance id.
    if (audio_preload_sound(argument0)) exit
    var __call;__call=__gm82audio_sfx_play(argument0,__gm82audio_get_def_vol(argument0),__gm82audio_get_def_pan(argument0),1,1)
    __gm82audio_check(__call,"audio_play",argument0)
    return __call


#define audio_play_ext
    ///audio_play_ext(sound,vol,pan,pitch,loop)
    //sound: sound index to play
    //vol,pan,pitch,loop: sound properties
    //returns: instance id
    //Plays a sound with preset properties and returns an instance id.
    if (audio_preload_sound(argument0)) exit
    var __call;__call=__gm82audio_sfx_play(
        argument0,argument1,argument2,argument3,argument4
    )
    __gm82audio_check(__call,"audio_play_ext",argument0)
    return __call


#define audio_play_single_ext
    ///audio_play_single_ext(sound,vol,pan,pitch,loop)
    //sound: sound index to play
    //vol,pan,pitch,loop: sound properties
    //returns: instance id
    //Plays a single instance of the sound with preset properties and returns an instance id.
    audio_stop(argument0)
    return audio_play_ext(argument0, argument1, argument2, argument3, argument4)


#define audio_music_play
    ///audio_music_play(sound,[fadeintime])
    //sound: sound index to play
    //fadeintime: optional time to fade in in ms
    //Plays a music piece. There can only be one music instance.
    if (audio_preload_sound(argument0)) exit
    var __fade;__fade=0
    
    if (argument_count==0 || argument_count>2) {
        show_error("in function audio_music_play: wrong number of arguments",0)
        exit
    }
    if (argument_count==2) __fade=max(0,argument[1])
    __gm82audio_check(__gm82audio_music_play(
        argument[0],__fade,__gm82audio_get_def_vol(argument0),__gm82audio_get_def_pan(argument0),1,1
    ),"audio_music_play",argument0)


#define audio_music_play_ext
    ///audio_music_play_ext(sound,fadeintime,vol,pan,pitch,loop)
    //sound: sound index to play
    //fadeintime: time to fade in in ms
    //vol,pan,pitch,loop: sound properties    
    //Plays a music piece with preset properties. There can only be one music instance.
    if (audio_preload_sound(argument0)) exit
    __gm82audio_check(__gm82audio_music_play(
        argument0,argument1,argument2,argument3,argument4,argument5>=0.5
    ),"audio_music_play_ext",argument0)


#define audio_music_crossfade
    ///audio_music_crossfade(sound,fadetime)
    //sound: sound index to play
    //fadetime: time to crossfade in in ms
    //Plays a new music piece and crossfades it with the old one.
    if (audio_preload_sound(argument0)) exit
    __gm82audio_check(__gm82audio_music_crossfade(
        argument0,argument1,__gm82audio_get_def_vol(argument0),__gm82audio_get_def_pan(argument0),1,1
    ),"audio_music_crossfade",argument0)


#define audio_music_crossfade_ext
    ///audio_music_crossfade_ext(sound,fadetime,vol,pan,pitch,loop)
    //sound: sound index to play
    //fadetime: time to crossfade in in ms
    //vol,pan,pitch,loop: sound properties   
    //Plays a new music piece with preset properties and crossfades it with the old one.
    if (audio_preload_sound(argument0)) exit
    __gm82audio_check(__gm82audio_music_crossfade(
        argument0,argument1,argument2,argument3,argument4,argument5>=0.5
    ),"audio_music_crossfade_ext",argument0)


#define audio_music_switch
    ///audio_music_switch(sound,fadeouttime,[fadeintime])
    //sound: sound index to play
    //fadeouttime: time to fade out in ms
    //fadeintime: optional, time to fade in in ms
    //Plays a new music piece and fades out and in to it.
    if (audio_preload_sound(argument0)) exit
    var __fadein;
    
    if (argument_count<2 || argument_count>3) {
        show_error("in function audio_music_switch: wrong number of arguments",0)
        exit
    }
    
    __fadein=argument[1]
    if (argument_count==3) __fadein=argument[2]
    
    __gm82audio_check(__gm82audio_music_switch(
        argument[0],argument[1],__fadein,__gm82audio_get_def_vol(argument0),__gm82audio_get_def_pan(argument0),1,1
    ),"audio_music_switch",argument[0])


#define audio_music_switch_ext
    ///audio_music_switch_ext(sound,fadeouttime,fadeintime,vol,pan,pitch,loop)
    //sound: sound index to play
    //fadeouttime: time to fade out in ms
    //fadeintime: time to fade in in ms
    //vol,pan,pitch,loop: sound properties
    //Plays a new music piece with preset properties and fades out and in to it.
    if (audio_preload_sound(argument0)) exit
    __gm82audio_check(__gm82audio_music_switch(
        argument0,argument1,argument2,argument3,argument4,argument5,argument6>=0.5
    ),"audio_music_switch_ext",argument0)


#define audio_music_stop
    ///audio_music_stop([fadeouttime])
    //fadeouttime: optional time to fade the music out
    //Stops the currently playing music piece with a fade out time.
    var __fade;__fade=0
    
    if (argument_count>1) {
        show_error("in function audio_music_stop: wrong number of arguments",0)
        exit
    }
    
    if (argument_count==1) __fade=max(0,argument[0])
    __gm82audio_music_stop(__fade)


#define audio_global_stop
    ///audio_global_stop([music too])
    //music too: stop music as well as sounds
    //Stops all playing sounds.
    if (argument_count) __gm82audio_stop_all(argument[0])
    else __gm82audio_stop_all(0)


#define audio_create_pack
    ///audio_create_pack(sourcedir,filename)
    //sourcedir: directory to load files from
    //filename: filename to save the pack to
    //Creates a sound pack containing supported files from the source directory.
    var __dir,__save,__q,__fn,__size,__mb,__b;
    
    __dir=string_replace_all(filename_dir(argument0),"/","\")
    __save=argument1

    __q=ds_queue_create()

    for (__fn=file_find_first(__dir+"\*.*",0);__fn!="";__fn=file_find_next()) {
        if (string_pos(string_lower(filename_ext(string(__fn))),".wav;.ogg;"))
            ds_queue_enqueue(__q,__dir+"\"+__fn)
    } file_find_close()

    __size=ds_queue_size(__q)

    __mb=buffer_create()
    __b=buffer_create()

    buffer_write_string(__mb,"WASD1.0")
    buffer_write_u32(__mb,__size)

    repeat (__size) {__fn=ds_queue_dequeue(__q)
        buffer_load(__b,__fn)
        buffer_deflate(__b)
        buffer_write_string(__mb,filename_name(__fn))
        buffer_write_u32(__mb,buffer_get_size(__b))
        buffer_copy(__mb,__b)
        buffer_clear(__b)
    }

    buffer_save(__mb,__save)
    buffer_destroy(__mb)
    buffer_destroy(__b)
    ds_queue_destroy(__q)


#define audio_load_pack
    ///audio_load_pack(pack)
    //pack: path to pack file to load
    //returns: map with the sounds added
    //Adds a sound pack for use.
    //Note: Make sure to delete the returned map when you're done.
    var __mb,__retlist,__b,__count,__name,__index,__length,__pos;

    __mb=buffer_create()
    buffer_load(__mb,argument0)

    if (buffer_read_string(__mb)!="WASD1.0") {buffer_destroy(__mb) show_error("Error loading WASD pack: file "+argument0+"is not a valid WASD pack.",0) return noone}

    __retlist=ds_map_create()
    __b=buffer_create()

    __count=buffer_read_u32(__mb)

    repeat (__count) {
        __name=buffer_read_string(__mb)
        __name=filename_remove_ext(__name)
        __length=buffer_read_u32(__mb)
        __pos=buffer_get_pos(__mb)
        buffer_copy_part(__b,__mb,__pos,__length)
        buffer_set_pos(__mb,__pos+__length)
        buffer_inflate(__b)
        __index=audio_load_buffer(__b)
        buffer_clear(__b)
        ds_map_add(__retlist,__name,__index)
    }

    return __retlist


#define audio_unpack_pack
    ///sound_unpack_pack(pack,dir)
    //pack: path to pack file
    //dir: directory to unpack to
    //returns: list of files unpacked
    //unpacks the specified pack file to a directory.
    //Note: Make sure to delete the returned list when you're done with it.
    var __dir,__mb,__retlist,__b,__count,__name,__fname,__length,__pos;

    __dir=string_replace_all(argument1,"/","\")+"\"
    directory_create(__dir)

    __mb=buffer_create()
    buffer_load(__mb,argument0)

    if (buffer_read_string(__mb)!="WASD1.0") {buffer_destroy(__mb) show_error("Error loading WASD pack: file "+argument0+"is not a valid WASD pack.",0) return noone}

    __b=buffer_create()

    __count=buffer_read_u32(__mb)
    
    __retlist=ds_list_create()

    repeat (__count) {
        __name=buffer_read_string(__mb)
        __fname=__dir+"\"+__name
        __length=buffer_read_u32(__mb)
        __pos=buffer_get_pos(__mb)
        buffer_copy_part(__b,__mb,__pos,__length)
        buffer_set_pos(__mb,__pos+__length)
        buffer_inflate(__b)
        buffer_save(__b,__fname)
        buffer_clear(__b)
        ds_list_add(__retlist,__name)
    }
    
    return __retlist


#define audio_set_loop_points
    ///audio_set_loop_points(soundid,a,[b])
    //sound: sound index
    //a: loop start in seconds
    //b (optional): loop end in seconds
    //Sets the loop points to use when playing a sound with looping enabled.
    //When B isn't supplied, it is set to the end of the file.
    if (audio_preload_sound(argument0)) exit
    var __b;
    
    if (argument_count<2 || argument_count>3) {
        show_error("in function audio_set_loop_points: wrong number of arguments",0)
        exit
    }
    
    __b=-1
    if (argument_count==3) __b=argument[2]
    
    __gm82audio_set_loop_points(argument[0],argument[1],__b)


#define audio_load_directory
    ///audio_load_directory(dir)
    //dir: relative or absolute path to search
    //returns: map with loaded sounds
    //Note that this function uses file_find internally. wav and ogg vorbis are supported.
    var __fn,__i,__map,__list,__snd;
    
    __map=ds_map_create()
    
    __list=file_find_list(argument0,"*.*",0,1,1)
    
    __i=0 repeat (ds_list_size(__list)) {
        __fn=ds_list_find_value(__list,__i)
        if (string_pos(string_lower(filename_ext(__fn)),".wav;.ogg;")) {
            __snd=audio_load(__fn)
            if (__snd) ds_map_add(__map,filename_name(__fn),__snd)
        }
    __i+=1}
    ds_list_destroy(__list)
    
    return __map


#define audio_load_included
    ///audio_load_included(filename)
    //filename: name of the included file to load
    //returns: sound index
    //Loads an included file. Make sure the file is set to [ ] do not free memory,
    //[x] overwrite, (x) Don't export. wav and ogg files are supported.
    var __fname,__snd;
    __fname=temp_directory+"\gm82\"+argument0
    export_include_file_location(argument0,__fname)
    __snd=audio_load(__fname)
    file_delete(__fname)
    return __snd


#define audio_get_length
    ///audio_get_length(sound)
    //sound: sound index to get
    //returns: the length of the sound, in seconds
    if (audio_preload_sound(argument0)) return 0
    return __gm82audio_get_length(argument0)


#define audio_exists
    ///audio_exists(sound/inst)
    //sound/inst: sound index to check, or instance
    //returns: various values
    //If a sound id is passed: 1 if it exists, 0 if it doesn't, 0.1 if it was deleted
    //If an instance id is passed: 1 if it's still playing, 0 if it's finished
    if (argument0<__gm82audio_get_builtin_count()) {
        return sound_exists(argument0)
    } else {
        return __gm82audio_exists(argument0)
    }


#define sound_add
    show_error("in function sound_add: please use audio_load instead!",0)
//
//