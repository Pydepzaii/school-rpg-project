// FILE: src/audio_manager.c
#include "audio_manager.h"
#include <stdio.h>
#include "settings.h"

// [STATE] Singleton Management
// Sử dụng static để giấu biến khỏi các file khác, chỉ truy cập qua hàm Audio_*.
// [GIẢI THÍCH]: musicList dùng cho nhạc nền (nhẹ RAM, load từ đĩa).
static Music musicList[MUSIC_COUNT];
// [GIẢI THÍCH]: currentMasterVolume lưu âm lượng tổng (0.0 đến 1.0).
static float currentMasterVolume = 1.0f;
// [GIẢI THÍCH]: soundList dùng cho âm thanh ngắn (nặng RAM hơn xíu nhưng phản hồi tức thì).
static Sound soundList[SFX_COUNT];
static bool isInitialized = false;
// [GIẢI THÍCH]: Biến này để nhớ bài nhạc nào đang phát, tránh việc gọi Play lại bài đang chạy gây lỗi reset nhạc.
static int currentMusicIndex = -1; 

void Audio_Init() {
    if (isInitialized) return;
    
    // [ASSETS] MUSIC (STREAMING)
    // Dùng cho nhạc nền dài (file nặng). Raylib sẽ stream trực tiếp từ ổ cứng để tiết kiệm RAM.
    // TODO: Nếu thêm bài mới, nhớ cập nhật enum MUSIC_COUNT trong file header.
    musicList[MUSIC_INTRO]    = LoadMusicStream("resources/intro.mp3");
    musicList[MUSIC_TITLE]    = LoadMusicStream("resources/bgm_title.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/bgm_thu_vien.mp3"); 
    musicList[MUSIC_NHA_AN]   = LoadMusicStream("resources/bgm_canteen.mp3");
    musicList[MUSIC_MAP_DEN]     = LoadMusicStream("resources/bgm_den.mp3");  
    musicList[MUSic_MAP_TRANG]    = LoadMusicStream("resources/bgm_trang.mp3"); 

    // [ASSETS] SFX (BUFFERED)
    // Dùng cho hiệu ứng ngắn (bước chân, click). Load toàn bộ vào RAM để phản hồi tức thì (low latency).
    soundList[SFX_STEP]     = LoadSound("resources/sfx_step.wav");
    soundList[SFX_TALK]     = LoadSound("resources/sfx_talk.wav");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sfx_click.ogg");
    soundList[SFX_UI_HOVER] = LoadSound("resources/sfx_hover.ogg");

    // [CONFIG] Default Mix
    // Cân bằng âm lượng mặc định ngay khi khởi tạo.
    for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], 0.5f); // Nhạc nền để 50% cho đỡ ồn
    for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], 1.0f);   // SFX để max
    SetMasterVolume(currentMasterVolume);
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

// [GIẢI THÍCH]: Hàm tiện ích để map tự gọi nhạc khi chuyển cảnh, đỡ phải nhớ tên bài hát ở file main.
void Audio_PlayMusicForMap(int mapID) {
    switch (mapID) {
        case MAP_THU_VIEN:
            Audio_PlayMusic(MUSIC_THU_VIEN);
            break;
        case MAP_NHA_AN:
            Audio_PlayMusic(MUSIC_NHA_AN);
            break;
        case MAP_DEN:
            Audio_PlayMusic(MUSIC_MAP_DEN);
            break;
        case MAP_TRANG:
            Audio_PlayMusic(MUSic_MAP_TRANG); 
            break;
        case MAP_SAN_TRUONG:
            // [CẢNH BÁO THỪA]: Case này đang trống, nếu map sân trường chưa có nhạc thì code chạy vào đây không làm gì cả.
            // Nếu chưa có nhạc sân trường, có thể tắt nhạc hoặc bật nhạc mặc định
            // Audio_StopMusic(currentMusicIndex); 
            break;
        default:
            break;
    }
}

void Audio_Shutdown() {
    if (!isInitialized) return;
    // [CLEANUP] Giải phóng tài nguyên để tránh Memory Leak
    for(int i=0; i<MUSIC_COUNT; i++) UnloadMusicStream(musicList[i]);
    for(int i=0; i<SFX_COUNT; i++) UnloadSound(soundList[i]);
    isInitialized = false;
}

// [GIẢI THÍCH]: Hàm set volume tổng, có kiểm tra biên (clamp) để không bị âm hoặc quá to (> 1.0).
void Audio_SetMasterVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    currentMasterVolume = volume;
    SetMasterVolume(currentMasterVolume); 
}

float Audio_GetMasterVolume(void) {
    return currentMasterVolume;
}