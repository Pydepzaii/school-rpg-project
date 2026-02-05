// FILE: src/audio_manager.h
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
    MUSIC_COUNT 
} MusicType;

// Định danh cho Hiệu ứng (Sound)
typedef enum {
    SFX_STEP = 0,
    SFX_TALK,
    SFX_UI_HOVER,
    SFX_UI_CLICK,
    SFX_COUNT 
} SoundType;

// Core Functions
void Audio_Init();                          
void Audio_Update();                        
void Audio_Shutdown();   

// Playback
void Audio_PlayMusic(MusicType type);       
void Audio_StopMusic(MusicType type);       
void Audio_PlaySoundEffect(SoundType type); 
void Audio_PlayMusicForMap(int mapID);      

// --- VOLUME CONTROL SYSTEM ---

// [NEW] 1. Master Volume (Tổng): Ảnh hưởng tất cả
void Audio_SetMasterVolume(float volume); 
float Audio_GetMasterVolume(void); 

// 2. Music Volume (Nhạc nền)
void Audio_SetMusicVolume(float volume);
float Audio_GetMusicVolume(void);

// 3. SFX Volume (Tiếng động)
void Audio_SetSFXVolume(float volume);
float Audio_GetSFXVolume(void);

// 4. Mute (Tắt tiếng)
void Audio_ToggleMute(void);
bool Audio_IsMuted(void);

#endif