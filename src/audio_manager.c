// FILE: src/audio_manager.c
#include "audio_manager.h"
#include <stdio.h>
#include "settings.h"

<<<<<<< Updated upstream
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
    musicList[MUSIC_INTRO]    = LoadMusicStream("resources/intro/intro.mp3");
    musicList[MUSIC_TITLE]    = LoadMusicStream("resources/sound/bgm/bgm_title.mp3");
    musicList[MUSIC_TOA_ALPHA]    = LoadMusicStream("resources/sound/bgm/bgm_alpha.mp3");
     musicList[MUSIC_NHA_VO]    = LoadMusicStream("resources/sound/bgm/bgm_nhavo.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/sound/bgm/bgm_thu_vien.mp3"); 
    musicList[MUSIC_NHA_AN]   = LoadMusicStream("resources/sound/bgm/bgm_canteen.mp3");
    musicList[MUSIC_MAP_DEN]     = LoadMusicStream("resources/sound/bgm/bgm_den.mp3");  
    musicList[MUSic_MAP_TRANG]    = LoadMusicStream("resources/sound/bgm/bgm_trang.mp3"); 

    // [ASSETS] SFX (BUFFERED)
    // Dùng cho hiệu ứng ngắn (bước chân, click). Load toàn bộ vào RAM để phản hồi tức thì (low latency).
    soundList[SFX_STEP]     = LoadSound("resources/sound/sfx/sfx_step.wav");
    soundList[SFX_TALK]     = LoadSound("resources/sound/sfx/sfx_talk.wav");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sound/sfx/sfx_click.ogg");
    soundList[SFX_UI_HOVER] = LoadSound("resources/sound/sfx/sfx_hover.ogg");

    // [CONFIG] Default Mix
    // Cân bằng âm lượng mặc định ngay khi khởi tạo.
    for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], 0.5f); // Nhạc nền để 50% cho đỡ ồn
    for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], 1.0f);   // SFX để max
    SetMasterVolume(currentMasterVolume);
=======
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
    
>>>>>>> Stashed changes
    isInitialized = true;
    printf(">> [AUDIO] System Initialized.\n");
}

void Audio_Update() {
    if (!isInitialized) return;
<<<<<<< Updated upstream
    // [CORE] Music Streaming Loop
    // Hàm này BẮT BUỘC phải gọi trong vòng lặp chính (Game Loop) để nạp tiếp buffer nhạc.
    // Quên gọi hàm này nhạc sẽ bị ngắt quãng hoặc không chạy.
    if (currentMusicIndex != -1) {
        UpdateMusicStream(musicList[currentMusicIndex]);
    }
=======
    if (currentMusicIndex != -1) UpdateMusicStream(musicList[currentMusicIndex]);
>>>>>>> Stashed changes
}

void Audio_PlayMusic(MusicType type) {
    if (!isInitialized || type >= MUSIC_COUNT) return;
<<<<<<< Updated upstream

    // Logic: Chỉ reset nhạc nếu bài mới khác bài cũ.
    if (currentMusicIndex != -1 && currentMusicIndex != type) {
        StopMusicStream(musicList[currentMusicIndex]);
    }

    currentMusicIndex = type;
    PlayMusicStream(musicList[currentMusicIndex]);
=======
    if (currentMusicIndex != -1 && currentMusicIndex != type) StopMusicStream(musicList[currentMusicIndex]);
    currentMusicIndex = type;
    PlayMusicStream(musicList[currentMusicIndex]);
    SetMusicVolume(musicList[currentMusicIndex], currentMusicVol);
>>>>>>> Stashed changes
}

void Audio_StopMusic(MusicType type) {
    if (!isInitialized) return;
    StopMusicStream(musicList[type]);
}
<<<<<<< Updated upstream

void Audio_PlaySoundEffect(SoundType type) {
    if (!isInitialized || type >= SFX_COUNT) return;
    // Sound có thể chồng lên nhau (Multi-instance) nên cứ gọi là phát.
    PlaySound(soundList[type]);
}

// [GIẢI THÍCH]: Hàm tiện ích để map tự gọi nhạc khi chuyển cảnh, đỡ phải nhớ tên bài hát ở file main.
void Audio_PlayMusicForMap(int mapID) {
    switch (mapID) {
        case MAP_TOA_ALPHA:
            Audio_PlayMusic(MUSIC_TOA_ALPHA);
            break;
        case MAP_NHA_VO:
            Audio_PlayMusic(MUSIC_NHA_VO);
            break;
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
=======
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
>>>>>>> Stashed changes
}