#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "raylib.h"

// Định danh cho Nhạc nền (Music)
typedef enum {
    MUSIC_INTRO = 0,
    MUSIC_THU_VIEN,
    MUSIC_NHA_AN,
    MUSIC_COUNT // Tổng số bài nhạc
} MusicType;

// Định danh cho Hiệu ứng (Sound)
typedef enum {
    SFX_STEP = 0,
    SFX_TALK,
    SFX_UI_CLICK,
    SFX_COUNT // Tổng số hiệu ứng
} SoundType;

// Các hàm chức năng
void Audio_Init();                          
void Audio_Update();                        
void Audio_PlayMusic(MusicType type);       
void Audio_StopMusic(MusicType type);       
void Audio_PlaySoundEffect(SoundType type); 
void Audio_Shutdown();                      

#endif