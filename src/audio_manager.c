#include "audio_manager.h"
#include <stdio.h>

// [STATE] Singleton Management
// Sử dụng static để giấu biến khỏi các file khác, chỉ truy cập qua hàm Audio_*.
static Music musicList[MUSIC_COUNT];
static Sound soundList[SFX_COUNT];
static bool isInitialized = false;
static int currentMusicIndex = -1; 

void Audio_Init() {
    if (isInitialized) return;
    
    // [ASSETS] MUSIC (STREAMING)
    // Dùng cho nhạc nền dài (file nặng). Raylib sẽ stream trực tiếp từ ổ cứng để tiết kiệm RAM.
    // TODO: Nếu thêm bài mới, nhớ cập nhật enum MUSIC_COUNT trong file header.
    musicList[MUSIC_INTRO]    = LoadMusicStream("resources/intro.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/bgm_library.mp3"); 
    musicList[MUSIC_NHA_AN]   = LoadMusicStream("resources/bgm_canteen.mp3"); 

    // [ASSETS] SFX (BUFFERED)
    // Dùng cho hiệu ứng ngắn (bước chân, click). Load toàn bộ vào RAM để phản hồi tức thì (low latency).
    soundList[SFX_STEP]     = LoadSound("resources/sfx_step.wav");
    soundList[SFX_TALK]     = LoadSound("resources/sfx_talk.wav");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sfx_click.wav");

    // [CONFIG] Default Mix
    // Cân bằng âm lượng mặc định ngay khi khởi tạo.
    for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], 0.5f); // Nhạc nền để 50% cho đỡ ồn
    for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], 1.0f);   // SFX để max

    isInitialized = true;
    printf(">> [AUDIO] System Initialized.\n");
}

void Audio_Update() {
    if (!isInitialized) return;
    // [CORE] Music Streaming Loop
    // Hàm này BẮT BUỘC phải gọi trong vòng lặp chính (Game Loop) để nạp tiếp buffer nhạc.
    // Quên gọi hàm này nhạc sẽ bị ngắt quãng hoặc không chạy.
    if (currentMusicIndex != -1) {
        UpdateMusicStream(musicList[currentMusicIndex]);
    }
}

void Audio_PlayMusic(MusicType type) {
    if (!isInitialized || type >= MUSIC_COUNT) return;

    // Logic: Chỉ reset nhạc nếu bài mới khác bài cũ.
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
    if (!isInitialized || type >= SFX_COUNT) return;
    // Sound có thể chồng lên nhau (Multi-instance) nên cứ gọi là phát.
    PlaySound(soundList[type]);
}

void Audio_Shutdown() {
    if (!isInitialized) return;
    // [CLEANUP] Giải phóng tài nguyên để tránh Memory Leak
    for(int i=0; i<MUSIC_COUNT; i++) UnloadMusicStream(musicList[i]);
    for(int i=0; i<SFX_COUNT; i++) UnloadSound(soundList[i]);
    isInitialized = false;
}