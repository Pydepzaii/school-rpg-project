#include "audio_manager.h"
#include <stdio.h>

static Music musicList[MUSIC_COUNT];
static Sound soundList[SFX_COUNT];
static bool isInitialized = false;
static int currentMusicIndex = -1; 

void Audio_Init() {
    if (isInitialized) return;
    
    // 1. NẠP NHẠC NỀN (MP3) - Đảm bảo file có trong resources
    // Lưu ý: Nếu file chưa có, nó sẽ báo warning nhưng không sập game
    musicList[MUSIC_INTRO]    = LoadMusicStream("resources/intro.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/bgm_library.mp3"); 
    musicList[MUSIC_NHA_AN]   = LoadMusicStream("resources/bgm_canteen.mp3"); 

    // 2. NẠP HIỆU ỨNG (WAV)
    soundList[SFX_STEP]     = LoadSound("resources/sfx_step.wav");
    soundList[SFX_TALK]     = LoadSound("resources/sfx_talk.wav");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sfx_click.wav");

    // Cài đặt Volume mặc định
    for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], 0.5f);
    for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], 1.0f);

    isInitialized = true;
    printf("--- AUDIO MANAGER INITIALIZED ---\n");
}

void Audio_Update() {
    if (!isInitialized) return;
    // Cập nhật stream nhạc nếu đang phát
    if (currentMusicIndex != -1) {
        UpdateMusicStream(musicList[currentMusicIndex]);
    }
}

void Audio_PlayMusic(MusicType type) {
    if (!isInitialized) return;
    if (type >= MUSIC_COUNT) return;

    // Nếu đang phát bài khác thì dừng lại
    if (currentMusicIndex != -1 && currentMusicIndex != type) {
        StopMusicStream(musicList[currentMusicIndex]);
    }

    currentMusicIndex = type;
    PlayMusicStream(musicList[currentMusicIndex]);
}

void Audio_StopMusic(MusicType type) {
    if (!isInitialized) return;
    StopMusicStream(musicList[type]);
}

void Audio_PlaySoundEffect(SoundType type) {
    if (!isInitialized) return;
    if (type >= SFX_COUNT) return;
    PlaySound(soundList[type]);
}

void Audio_Shutdown() {
    if (!isInitialized) return;
    for(int i=0; i<MUSIC_COUNT; i++) UnloadMusicStream(musicList[i]);
    for(int i=0; i<SFX_COUNT; i++) UnloadSound(soundList[i]);
    isInitialized = false;
}