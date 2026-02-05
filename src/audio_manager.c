// FILE: src/audio_manager.c
#include "audio_manager.h"
#include <stdio.h>
#include "settings.h"

static Music musicList[MUSIC_COUNT];
static Sound soundList[SFX_COUNT];
static bool isInitialized = false;
static int currentMusicIndex = -1; 

// [STATE] Biến lưu Volume
static float currentMasterVol = 1.0f; // [NEW] Biến tổng
static float currentMusicVol = 0.5f; 
static float currentSFXVol = 1.0f;   
static bool isMuted = false;         

void Audio_Init() {
    if (isInitialized) return;
    
    // Load Music
    musicList[MUSIC_INTRO] = LoadMusicStream("resources/intro/intro.mp3");
    musicList[MUSIC_TITLE] = LoadMusicStream("resources/sound/bgm/bgm_title.mp3");
    musicList[MUSIC_TOA_ALPHA] = LoadMusicStream("resources/sound/bgm/bgm_alpha.mp3");
    musicList[MUSIC_NHA_VO] = LoadMusicStream("resources/sound/bgm/bgm_nhavo.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/sound/bgm/bgm_thu_vien.mp3"); 
    musicList[MUSIC_NHA_AN] = LoadMusicStream("resources/sound/bgm/bgm_canteen.mp3");
    musicList[MUSIC_MAP_DEN] = LoadMusicStream("resources/sound/bgm/bgm_den.mp3");  
    musicList[MUSic_MAP_TRANG]= LoadMusicStream("resources/sound/bgm/bgm_trang.mp3"); 

    // Load SFX
    soundList[SFX_STEP] = LoadSound("resources/sound/sfx/sfx_step.wav");
    soundList[SFX_TALK] = LoadSound("resources/sound/sfx/sfx_talk.wav");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sound/sfx/sfx_click.ogg");
    soundList[SFX_UI_HOVER] = LoadSound("resources/sound/sfx/sfx_hover.ogg");

    // Apply Volume mặc định
    Audio_SetMasterVolume(currentMasterVol); // Set tổng trước
    Audio_SetMusicVolume(currentMusicVol);
    Audio_SetSFXVolume(currentSFXVol);
    
    isInitialized = true;
    printf(">> [AUDIO] System Initialized.\n");
}

void Audio_Update() {
    if (!isInitialized) return;
    if (currentMusicIndex != -1) UpdateMusicStream(musicList[currentMusicIndex]);
}

void Audio_PlayMusic(MusicType type) {
    if (!isInitialized || type >= MUSIC_COUNT) return;
    if (currentMusicIndex != -1 && currentMusicIndex != type) StopMusicStream(musicList[currentMusicIndex]);
    currentMusicIndex = type;
    PlayMusicStream(musicList[currentMusicIndex]);
    SetMusicVolume(musicList[currentMusicIndex], currentMusicVol);
}

void Audio_StopMusic(MusicType type) {
    if (!isInitialized) return;
    StopMusicStream(musicList[type]);
}
void Audio_PlaySoundEffect(SoundType type) {
    if (!isInitialized || type >= SFX_COUNT) return;
    PlaySound(soundList[type]);
}
void Audio_PlayMusicForMap(int mapID) {
    switch (mapID) {
        case MAP_TOA_ALPHA: Audio_PlayMusic(MUSIC_TOA_ALPHA); break;
        case MAP_NHA_VO:    Audio_PlayMusic(MUSIC_NHA_VO); break;
        case MAP_THU_VIEN:  Audio_PlayMusic(MUSIC_THU_VIEN); break;
        case MAP_NHA_AN:    Audio_PlayMusic(MUSIC_NHA_AN); break;
        case MAP_DEN:       Audio_PlayMusic(MUSIC_MAP_DEN); break;
        case MAP_TRANG:     Audio_PlayMusic(MUSic_MAP_TRANG); break;
        default: break;
    }
}

// --- LOGIC VOLUME CONTROL ---

// [NEW] 1. Master Volume
void Audio_SetMasterVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f; 
    if (volume > 1.0f) volume = 1.0f;
    
    currentMasterVol = volume;
    
    // Nếu đang không Mute thì cập nhật ngay
    if (!isMuted) {
        SetMasterVolume(currentMasterVol);
    }
}

float Audio_GetMasterVolume(void) {
    return currentMasterVol;
}

// 2. Music Volume
void Audio_SetMusicVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f; if (volume > 1.0f) volume = 1.0f;
    currentMusicVol = volume;
    for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], currentMusicVol);
}
float Audio_GetMusicVolume(void) { return currentMusicVol; }

// 3. SFX Volume
void Audio_SetSFXVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f; if (volume > 1.0f) volume = 1.0f;
    currentSFXVol = volume;
    for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], currentSFXVol);
}
float Audio_GetSFXVolume(void) { return currentSFXVol; }

// 4. Toggle Mute
void Audio_ToggleMute(void) {
    isMuted = !isMuted;
    
    if (isMuted) {
        SetMasterVolume(0.0f); // Tắt sạch
    } else {
        // [QUAN TRỌNG]: Khi mở lại, trả về đúng mức Master Volume người dùng đã chỉnh
        SetMasterVolume(currentMasterVol); 
    }
}
bool Audio_IsMuted(void) { return isMuted; }

void Audio_Shutdown() {
    if (!isInitialized) return;
    for(int i=0; i<MUSIC_COUNT; i++) UnloadMusicStream(musicList[i]);
    for(int i=0; i<SFX_COUNT; i++) UnloadSound(soundList[i]);
    isInitialized = false;
}