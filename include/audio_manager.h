#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "raylib.h"

// Định danh cho Nhạc nền (Music)
typedef enum {
    MUSIC_INTRO = 0,
    MUSIC_TITLE,
    MUSIC_TOA_ALPHA,
    MUSIC_NHA_VO,
    MUSIC_THU_VIEN,
    MUSIC_NHA_AN,
    MUSic_MAP_TRANG,
    MUSIC_MAP_DEN,
    MUSIC_COUNT // Tổng số bài nhạc
} MusicType;

// Định danh cho Hiệu ứng (Sound)
typedef enum {
    SFX_STEP = 0,
    SFX_TALK,
    SFX_UI_HOVER,
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
void Audio_SetMasterVolume(float volume); 
float Audio_GetMasterVolume(void); 
void Audio_PlayMusicForMap(int mapID);                  

#endif