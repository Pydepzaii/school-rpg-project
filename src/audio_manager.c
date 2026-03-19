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
static bool isMusicMuted = false;    // [NEW] Trạng thái tắt nhạc nền
static bool isSFXMuted = false;      // [NEW] Trạng thái tắt tiếng động

void Audio_Init() {
    if (isInitialized) return;
    
    // Load Music
    musicList[MUSIC_INTRO] = LoadMusicStream("resources/intro/intro.mp3");
    musicList[MUSIC_TITLE] = LoadMusicStream("resources/sound/bgm/bgm_title.mp3");
    musicList[MUSIC_TOA_ALPHA] = LoadMusicStream("resources/sound/bgm/bgm_alpha.mp3");
    musicList[MUSIC_NHA_VO] = LoadMusicStream("resources/sound/bgm/bgm_nhavo.mp3");
    musicList[MUSIC_THU_VIEN] = LoadMusicStream("resources/sound/bgm/bgm_thu_vien.mp3"); 
    musicList[MUSIC_NHA_AN] = LoadMusicStream("resources/sound/bgm/bgm_canteen.mp3");
    musicList[MUSIC_LAB] = LoadMusicStream("resources/sound/bgm/bgm_lab.mp3");
    musicList[MUSIC_BETA] = LoadMusicStream("resources/sound/bgm/bgm_beta.mp3");
    musicList[MUSIC_BATTLE] = LoadMusicStream("resources/sound/bgm/bgm_battle.mp3");
    // [THÊM DÒNG NÀY] Load nhạc Phase 2
    musicList[MUSIC_BATTLE_PHASE2] = LoadMusicStream("resources/sound/bgm/bgm_battle_phase2.mp3");
    // Load SFX
    soundList[SFX_STEP] = LoadSound("resources/sound/sfx/sfx_step.wav");
   soundList[SFX_TALK] = LoadSound("resources/sound/sfx/sfx_dialog_sound_tap.mp3");
    soundList[SFX_UI_CLICK] = LoadSound("resources/sound/sfx/sfx_click.ogg");
    soundList[SFX_UI_HOVER] = LoadSound("resources/sound/sfx/sfx_hover.ogg");
    soundList[SFX_DAUGAU_DANHTHUONG] = LoadSound("resources/sound/sfx/daugau_danhthuong_timer0.5-3.mp3");
    soundList[SFX_EXPLOSION_3S] = LoadSound("resources/sound/sfx/explotion_timer_3s.mp3");
    soundList[SFX_KECHIUDON] = LoadSound("resources/sound/sfx/kechiudon_timeming0.5s_to_end.mp3");
    soundList[SFX_CUDAMSAMSET_PREP] = LoadSound("resources/sound/sfx/lightninghit_timeming_0.5_3.mp3");
    soundList[SFX_CUDAMSAMSET_BOOM] = LoadSound("resources/sound/sfx/lightninghit_timing_3_to_end.mp3");
    soundList[SFX_RUNGCHAN_PREP] = LoadSound("resources/sound/sfx/rungchan_timing_0.5_to_3s.mp3");
    soundList[SFX_GIAPGAI] = LoadSound("resources/sound/sfx/giap_gai.mp3");
    soundList[SFX_PHANUNGHOAHOC] = LoadSound("resources/sound/sfx/phan_ung_hoa_hoc_allphase.mp3");
    // [THÊM DÒNG NÀY VÀO CHỖ LOAD SOUND]
    soundList[SFX_TAPTRUNGCAODO] = LoadSound("resources/sound/sfx/taptrungcaodo_allphase.mp3");
    soundList[SFX_CAUTUTRUONG_PREP] = LoadSound("resources/sound/sfx/cau_tu_truong_0.5_to_3s.mp3");
    soundList[SFX_DINHLICUOICUNG_PREP] = LoadSound("resources/sound/sfx/dinhlicuoicung.mp3");
    soundList[SFX_BOSS_LAZER] = LoadSound("resources/sound/sfx/lazer_timing allskill.mp3");
    soundList[SFX_BOSS_TELEGATE] = LoadSound("resources/sound/sfx/telegate_allphase.mp3");
    soundList[SFX_BOSS_PHAOHODEN] = LoadSound("resources/sound/sfx/phao_ho_den_timing0_to_3s.mp3");
    soundList[SFX_BOSS_DOTKICHPHANRA] = LoadSound("resources/sound/sfx/dot_kich_phan_ra_0.5_to_3s.mp3");
    soundList[SFX_BOSS_QUATAIHUYETTHANH] = LoadSound("resources/sound/sfx/qua_tai_huyet_thanh_allphase.mp3");

    soundList[SFX_SOAICA_HUYETTIEN] = LoadSound("resources/sound/sfx/huyet_tien_hit_timing_3s.mp3");
    soundList[SFX_SOAICA_HAOQUANG] = LoadSound("resources/sound/sfx/hao_quang_huyet_sac0.2_to_end.mp3");
    soundList[SFX_SOAICA_DAUAN] = LoadSound("resources/sound/sfx/dau_an_ky_sinh0.2_to_end.mp3");
    soundList[SFX_SOAICA_GIAOKEO] = LoadSound("resources/sound/sfx/giao_keo_ac_quy_0.2_to_end.mp3");
    soundList[SFX_SOAICA_SINGLE_ARROW] = LoadSound("resources/sound/sfx/one_shot0.2.mp3");
    soundList[SFX_PHUNHIDAI_COINSPIN] = LoadSound("resources/sound/sfx/coinspin0.5_to_3.mp3");
    soundList[SFX_PHUNHIDAI_DOTIM_PREP] = LoadSound("resources/sound/sfx/do_tim_con_moi_timing0.5_to_3s.mp3");
    soundList[SFX_PHUNHIDAI_DOTIM_BOOM] = LoadSound("resources/sound/sfx/do_tim_con_moi_timing_3s.mp3");
    soundList[SFX_PHUNHIDAI_TRIETHA] = LoadSound("resources/sound/sfx/triet_ha_con_moi_allphase.mp3");
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
    if (isMuted || isSFXMuted) return;
    
    if (type >= 0 && type < SFX_COUNT) {
        float finalVol = currentSFXVol;

        // [MỚI] HỆ THỐNG KÍCH ÂM LƯỢNG (ĐÃ TỐI ƯU SIÊU GỌN)
        switch (type) {
            case SFX_KECHIUDON:
                finalVol = currentSFXVol * 30.0f; 
                break;
            
            case SFX_DINHLICUOICUNG_PREP:
            case SFX_BOSS_PHAOHODEN:
            case SFX_BOSS_DOTKICHPHANRA:
                finalVol = currentSFXVol * 7.0f; 
                break;
                
            case SFX_CAUTUTRUONG_PREP:
            case SFX_BOSS_LAZER:
            case SFX_BOSS_TELEGATE:
            case SFX_TALK:
                finalVol = currentSFXVol * 4.0f; 
                break;

            // Tất cả các chiêu còn lại dùng chung mức x5.0f
            case SFX_DAUGAU_DANHTHUONG:
            case SFX_EXPLOSION_3S:
            case SFX_CUDAMSAMSET_PREP:
            case SFX_CUDAMSAMSET_BOOM:
            case SFX_RUNGCHAN_PREP:
            case SFX_GIAPGAI:
            case SFX_PHANUNGHOAHOC:
            case SFX_TAPTRUNGCAODO:
            case SFX_BOSS_QUATAIHUYETTHANH:
            case SFX_SOAICA_SINGLE_ARROW:
                finalVol = currentSFXVol * 5.0f; 
                break;
            case SFX_PHUNHIDAI_COINSPIN:
            case SFX_PHUNHIDAI_DOTIM_PREP:
            case SFX_PHUNHIDAI_DOTIM_BOOM:
            case SFX_PHUNHIDAI_TRIETHA:
                finalVol = currentSFXVol * 5.0f; 
                break;
            // ... (các case x5.0f đang có sẵn)
            case SFX_SOAICA_HUYETTIEN:
            case SFX_SOAICA_HAOQUANG:
            case SFX_SOAICA_DAUAN:
            case SFX_SOAICA_GIAOKEO:
                finalVol = currentSFXVol * 5.0f; 
                break;

            default:
                // Các âm thanh UI, Bước chân... giữ nguyên x1.0f
                break;
        }

        // Set âm lượng đã kích rồi mới phát nhạc
        SetSoundVolume(soundList[type], finalVol);
        PlaySound(soundList[type]);
    }
}
void Audio_PlayMusicForMap(int mapID) {
    switch (mapID) {
        case MAP_TOA_ALPHA: Audio_PlayMusic(MUSIC_TOA_ALPHA); break;
        case MAP_NHA_VO:    Audio_PlayMusic(MUSIC_NHA_VO); break;
        case MAP_THU_VIEN:  Audio_PlayMusic(MUSIC_THU_VIEN); break;
        case MAP_NHA_AN:    Audio_PlayMusic(MUSIC_NHA_AN); break;
        case MAP_BETA:    Audio_PlayMusic(MUSIC_BETA); break;
        case MAP_LAB:    Audio_PlayMusic(MUSIC_LAB); break;
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
    // [MODIFIED] Chỉ set volume thật khi không bị Mute
    if (!isMusicMuted && !isMuted) {
        for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], currentMusicVol);
    }
}
float Audio_GetMusicVolume(void) { return currentMusicVol; }

// 3. SFX Volume
void Audio_SetSFXVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f; if (volume > 1.0f) volume = 1.0f;
    currentSFXVol = volume;
    // [MODIFIED] Chỉ set volume thật khi không bị Mute
    if (!isSFXMuted && !isMuted) {
        for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], currentSFXVol);
    }
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
// [NEW] 5. Toggle Mute Music
void Audio_ToggleMusicMute(void) {
    isMusicMuted = !isMusicMuted;
    if (isMusicMuted) {
        for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], 0.0f); // Tắt sạch nhạc
    } else {
        if (!isMuted) { // Nếu Master đang không tắt thì mới trả lại tiếng
            for(int i=0; i<MUSIC_COUNT; i++) SetMusicVolume(musicList[i], currentMusicVol);
        }
    }
}
bool Audio_IsMusicMuted(void) { return isMusicMuted; }

// [NEW] 6. Toggle Mute SFX
void Audio_ToggleSFXMute(void) {
    isSFXMuted = !isSFXMuted;
    if (isSFXMuted) {
        for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], 0.0f); // Tắt sạch tiếng động
    } else {
        if (!isMuted) { // Nếu Master đang không tắt thì mới trả lại tiếng
            for(int i=0; i<SFX_COUNT; i++) SetSoundVolume(soundList[i], currentSFXVol);
        }
    }
}
bool Audio_IsSFXMuted(void) { return isSFXMuted; }

void Audio_Shutdown() {
    if (!isInitialized) return;
    for(int i=0; i<MUSIC_COUNT; i++) UnloadMusicStream(musicList[i]);
    for(int i=0; i<SFX_COUNT; i++) UnloadSound(soundList[i]);
    isInitialized = false;
}