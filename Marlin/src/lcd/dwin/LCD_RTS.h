#ifndef RTS_H
#define RTS_H

#include "string.h"
#include <arduino.h>

#include "i2c_eeprom.h"

#include "../../inc/MarlinConfig.h"

extern int power_off_type_yes;

// =====================================================
// DWIN T5L UART çerçeve protokolü
// =====================================================
constexpr uint8_t  FHONE  = 0x5A;
constexpr uint8_t  FHTWO  = 0xA5;
constexpr uint8_t  FHLENG = 0x06;

constexpr uint8_t  TEXTBYTELEN   = 18;
constexpr uint8_t  MaxFileNumber = 20;
constexpr uint8_t  FileNum       = MaxFileNumber;
constexpr uint8_t  FileNameLen   = TEXTBYTELEN;

constexpr uint16_t RTS_UPDATE_INTERVAL = 1000;  // 1s for responsive temp/progress display (was 2000)
constexpr uint16_t RTS_UPDATE_VALUE    = 2 * RTS_UPDATE_INTERVAL;
constexpr uint8_t  SizeofDatabuf       = 40;

// Register / variable read-write komutları
constexpr uint8_t  RegAddr_W = 0x80;
constexpr uint8_t  RegAddr_R = 0x81;
constexpr uint8_t  VarAddr_W = 0x82;
constexpr uint8_t  VarAddr_R = 0x83;

// DWIN sayfa değişimi: gerçek sayfa = base + page_id
constexpr uint32_t ExchangePageBase = 0x5A010000UL;
constexpr uint16_t ExchangepageAddr = 0x0084;

constexpr uint8_t  FONT_EEPROM = 0;
constexpr uint8_t  FanOn       = 255;
constexpr uint8_t  FanOff      = 0;

// =====================================================
// Aktif legacy VP adresleri (kod tarafından hâlâ kullanılan)
// =====================================================
constexpr uint16_t AutoZero      = 0x1046;  // Autohome trigger VP (move ekranı)
constexpr uint16_t AutolevelVal  = 0x1100;  // BILINEAR mesh value array
constexpr uint16_t AutolevelIcon = 0x108D;  // BLTOUCH probe icon
constexpr uint16_t ExchFlmntIcon = 0x108E;  // Filament değişim progress icon
constexpr uint16_t FilementUnit1 = 0x1054;  // Manual feed/retract uzunluk

// =====================================================
// Cihaz bilgi string'leri (DWIN ekrandaki "About" sayfası)
// =====================================================
#define MACHINE_TYPE      "Sermoon D1"
#define FIRMWARE_VERSION  "MarlinV2 by CTK"
#define HARDWARE_VERSION  "HW 4.3.1"
#define SCREEN_VERSION    "DWIN 1.1.14"
#define PRINT_SIZE        "280*260*310"
#define CORP_WEBSITE_C    "www.cxsw3d.com  "
#define CORP_WEBSITE_E    "www.creality.com"

// =====================================================
// RTS çerçeve struct'ları
// =====================================================
typedef struct DataBuf
{
    unsigned char len;
    unsigned char head[2];
    unsigned char command;
    unsigned long addr;
    unsigned long bytelen;
    unsigned short data[32];
    unsigned char reserv[4];
} DB;

typedef struct CardRecord
{
    int recordcount;
    int Filesum;
    unsigned long addr[FileNum];
    char Cardshowfilename[FileNum][FileNameLen];
    char Cardfilename[FileNum][FileNameLen];
} CRec;

class RTSUI {
  public:
    #if ENABLED(LCD_BED_LEVELING) && EITHER(PROBE_MANUALLY, MESH_BED_LEVELING)
      static bool wait_for_bl_move;
    #else
      static constexpr bool wait_for_bl_move = false;
    #endif

    static int16_t preheat_hotend_temp[2], preheat_bed_temp[2];
    static uint8_t preheat_fan_speed[2];
};

extern RTSUI rtsui;

class RTSSHOW {
  public:
    RTSSHOW();
    int RTS_RecData();
    void RTS_SDCardInit(void);
    void RTS_SDCardUpate(void);
    int RTS_CheckFilement(int);
    void RTS_SndData(void);
    void RTS_SndData(const String &, unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(const char[], unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(char, unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(unsigned char*, unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(int, unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(float, unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(unsigned int,unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(long,unsigned long, unsigned char = VarAddr_W);
    void RTS_SndData(unsigned long,unsigned long, unsigned char = VarAddr_W);
    void RTS_SDcard_Stop();
    void RTS_HandleData();
    void RTS_Init();

    DB recdat;
    DB snddat;
  private:
    unsigned char databuf[SizeofDatabuf];
};

extern RTSSHOW rtscheck;

// =====================================================
// UI sayfa action VP'leri (DWIN ekrandan gelen butonlar)
// =====================================================
enum ReturnKeyAddr {
                      MainPage = 0x2001,
                      SelectFile = 0x2002,
                      PrintOptions = 0x2007,
                      TempMenu = 0x200C,
                      AutoTemp = 0x200E,
                      ManualTemp = 0x2010,
                      SettingsMenu = 0x2011,
                      LevelMode = 0x2012,
                      ChangeFilament = 0x2015,
                      MovePage = 0x2019,
                      LanguageOptions = 0x201D,
                      PwrOffContinue = 0x2025,
                      NoFilamentContinue = 0x2026,
                    };

// =====================================================
// DWIN font tablosundaki status string slot offset'leri
// STATUS_DP_CHAR_VP'ye yazılan değer = language_change + slot
// (language_change 1-9 dil ID'si, slot 0-tabanlı)
// =====================================================
enum DwinStatusSlot : uint8_t {
  DWIN_STATUS_CARD_OUT = 53,
  DWIN_STATUS_READY    = 62,
  DWIN_STATUS_HEATING  = 71,
  DWIN_STATUS_PRINTING = 89,
  DWIN_STATUS_COOLING  = 107,
  DWIN_STATUS_PAUSED   = 125,
};

extern void RTSUpdate();
extern void RTSInit();

extern bool heat_flag;
extern bool home_flag;

extern char waitway;
extern char CardCheckStatus[2];
extern bool InforShowStatus;
extern unsigned char LanguageRecbuf;
extern unsigned char AxisPagenum;
extern bool AutohomeKey;
extern bool TPShowStatus;
extern bool AutoLevelStatus;
extern int Update_Time_Value;
extern bool PoweroffContinue;
extern char FilementStatus[2];
extern char commandbuf[30];
extern int PrintModeTime;
extern float zprobe_zoffset;

extern unsigned char G29_status;
extern char PrinterStatusKey[2];
// PrintStatue[0]: 0 → page 43, 1 → page 44
extern char PrintStatue[2];
extern bool PreheatStatus[];
extern unsigned char language_change;

extern void Set_Language(unsigned char num);
extern void Lcd_Select_Var(unsigned char Num,int BeginAddr,unsigned char TotalNum);


// =====================================================
// 9 dilli UI variable address haritası
// =====================================================

// Boot progress bar
constexpr uint16_t START_PROCESS_ICON_VP    = 0x1000;

// --- Başlık (Title) char VP'leri ---
constexpr uint16_t PRINT_TITLE_CHAR_VP      = 0x1001;  // Yazdır
constexpr uint16_t TEMP_TITLE_CHAR_VP       = 0x1002;  // Sıcaklık
constexpr uint16_t SET_TEMP_TITLE_CHAR_VP   = 0x1003;  // Manuel sıcaklık
constexpr uint16_t AUX_LEVEL_TITLE_CHAR_VP  = 0x1004;  // Manuel tabla
constexpr uint16_t REFUL_TITLE_CHAR_VP      = 0x1005;  // Filament değişim
constexpr uint16_t HEATING_TITLE_CHAR_VP    = 0x1006;  // Isıtılıyor
constexpr uint16_t AUTO_HOME_TITLE_CHAR_VP  = 0x1007;  // Sıfırla
constexpr uint16_t LANGUAGE_TITLE_CHAR_VP   = 0x1008;  // Dil
constexpr uint16_t ABOUT_TITLE_CHAR_VP      = 0x1009;  // Hakkında
constexpr uint16_t SETTINGS_TITLE_CHAR_VP   = 0x100A;  // Ayarlar
constexpr uint16_t FAN_TITLE_CHAR_VP        = 0x100B;  // Fan
constexpr uint16_t LEVEL_MODE_TITLE_CHAR_VP = 0x100C;  // Tabla modu

// --- Sinyal ikonları ---
constexpr uint16_t ASSIST_LEVEL_SIGNAL_VP   = 0x1110;  // Yardımcı tablalama
constexpr uint16_t CHANGE_SIGNAL_VP         = 0x1111;  // Filament değiştirme
constexpr uint16_t AUTO_HOME_SIGNAL_VP      = 0x1112;  // Sıfırlama
constexpr uint16_t AUTO_LEVEL_SIGNAL_VP     = 0x1113;  // Otomatik tablalama
constexpr uint16_t PROCESSING_SIGNAL_VP     = 0x1114;  // Yürütülüyor

// --- GIF animasyonları ---
constexpr uint16_t LEVEL_AUTORUN_VP         = 0x1120;  // Otomatik tablalama animasyonu
constexpr uint16_t HOME_AUTORUN_VP          = 0x1121;  // Otomatik sıfırlama animasyonu
constexpr uint16_t MESSAGE_WARING_VP        = 0x1122;  // Uyarı mesajı
constexpr uint16_t REFUEL_GIF_FILEMENT_VP   = 0x1123;  // Filament değişim animasyonu

// --- Ana menü text ---
constexpr uint16_t PRINT_MAIN_CHAR_VP       = 0x1020;  // Yazdır
constexpr uint16_t TEMP_MAIN_CHAR_VP        = 0x1021;  // Sıcaklık
constexpr uint16_t SETTINGS_MAIN_CHAR_VP    = 0x1022;  // Ayarlar

// --- Yazdırma ekranı ---
constexpr uint16_t BEGIN_PRINT_CHAR_VP      = 0x1023;  // Başla
constexpr uint16_t COMPLETE_PRINT_CHAR_VP   = 0x1024;  // Tamamlandı
constexpr uint16_t STOP_PRINT_CHAR_VP       = 0x1025;  // Durdur
constexpr uint16_t ADJUST_PRINT_CHAR_VP     = 0x1029;  // Ayarla
constexpr uint16_t PAUSE_PRINT_CHAR_VP      = 0x1027;  // Duraklat
constexpr uint16_t CONTINUE_PRINT_CHAR_VP   = 0x1028;  // Devam et
constexpr uint16_t PRINT_PERCENT_DATA_VP    = 0x1408;  // Yüzde

// --- Z offset / Adjust ekranı ---
constexpr uint16_t Z_OFFSET_Z_CHAR_VP       = 0x1030;  // Z offset:
constexpr uint16_t UNIT_Z_CHAR_VP           = 0x1031;  // Birim 0.01mm
constexpr uint16_t FAN_Z_CHAR_VP            = 0x1032;  // Fan
constexpr uint16_t PRINT_SPEED_Z_CHAR_VP    = 0x1033;  // Yazdırma hızı
constexpr uint16_t NOZZLE_TEMP_Z_CHAR_VP    = 0x1034;  // Nozül sıcaklığı
constexpr uint16_t BED_TEMP_Z_CHAR_VP       = 0x1035;  // Yatak sıcaklığı

// --- Sıcaklık ekranı ---
constexpr uint16_t AUTO_TEMP_CHAR_VP        = 0x1040;  // Auto preset
constexpr uint16_t MANUAL_TEMP_CHAR_VP      = 0x1041;  // Manuel sıcaklık
constexpr uint16_t COOLING_TEMP_CHAR_VP     = 0x1042;  // Soğutma
constexpr uint16_t FAN_TEMP_CHAR_VP         = 0x1043;  // Fan

// --- Manuel sıcaklık ekranı ---
constexpr uint16_t NOZZLE_PREHEAT_CHAR_VP   = 0x1050;  // Nozül ön-ısıtma
constexpr uint16_t NOZZLE_TEMP_CHAR_VP      = 0x1051;  // Nozül sıcaklığı
constexpr uint16_t BED_PREHEAT_CHAR_VP      = 0x1052;  // Yatak ön-ısıtma
constexpr uint16_t BED_TEMP_CHAR_VP         = 0x1053;  // Yatak sıcaklığı
constexpr uint16_t COOLING_MANUAL_CHAR_VP   = 0x1054;  // Tek tuş soğutma

// --- Filament değişim ekranı ---
constexpr uint16_t EXTRUDER_REFUL_CHAR_VP   = 0x1060;  // Extruder
constexpr uint16_t UNIT_REFUL_CHAR_VP       = 0x1061;  // Birim:mm
constexpr uint16_t FEED1_REFUL_CHAR_VP      = 0x1062;  // İlerle 1
constexpr uint16_t FEED2_REFUL_CHAR_VP      = 0x1063;  // İlerle 2
constexpr uint16_t RETREAT1_REFUL_CHAR_VP   = 0x1064;  // Geri çek 1
constexpr uint16_t RETREAT2_REFUL_CHAR_VP   = 0x1065;  // Geri çek 2

// --- About ekranı ---
constexpr uint16_t MACHINE_TYPE_ABOUT_CHAR_VP = 0x1080;  // Cihaz modeli
constexpr uint16_t HW_VERSION_ABOUT_CHAR_VP   = 0x1081;  // Donanım sürümü
constexpr uint16_t FW_VERSION_ABOUT_CHAR_VP   = 0x1082;  // Firmware sürümü
constexpr uint16_t DP_VERSION_ABOUT_CHAR_VP   = 0x1083;  // Ekran sürümü
constexpr uint16_t PRINT_SIZE_ABOUT_CHAR_VP   = 0x1084;  // Baskı alanı
constexpr uint16_t WEBSITE_ABOUT_CHAR_VP      = 0x1085;  // Web site

// --- Settings ekranı ---
constexpr uint16_t LEVELING_SET_CHAR_VP     = 0x1090;  // Tabla modu
constexpr uint16_t REFUL_SET_CHAR_VP        = 0x1091;  // Filament değişim
constexpr uint16_t MOVE_SET_CHAR_VP         = 0x1092;  // Eksen hareket
constexpr uint16_t MOTOR_CTRL_SET_CHAR_VP   = 0x1093;  // Motor kontrol
constexpr uint16_t LANGUAGE_SET_CHAR_VP     = 0x1094;  // Dil
constexpr uint16_t ABOUT_SET_CHAR_VP        = 0x1095;  // Hakkında
constexpr uint16_t MOTOR_STATUS_ICON_VP     = 0x1224;  // Motor ikonu

// --- Tabla mod ekranı ---
constexpr uint16_t MEASURE_LEVEL_CHAR_VP    = 0x10A0;  // Platform ölçüm
constexpr uint16_t AUX_LEVEL_CHAR_VP        = 0x10A1;  // Yardımcı tabla
constexpr uint16_t AUTO_LEVEL_CHAR_VP       = 0x10A2;  // Otomatik tabla

// --- Eksen hareket ---
constexpr uint16_t UNIT_MOVE_CHAR_VP        = 0x10B0;  // Birim
constexpr uint16_t X_MOVE_CHAR_VP           = 0x10B1;  // X ekseni
constexpr uint16_t Y_MOVE_CHAR_VP           = 0x10B2;  // Y ekseni
constexpr uint16_t Z_MOVE_CHAR_VP           = 0x10B3;  // Z ekseni

// --- Dialog kutuları ---
constexpr uint16_t MOTOR_OFF_DIALOG_VP      = 0x1100;  // Motor kapat
constexpr uint16_t RESUME_PRINT_DIALOG_VP   = 0x1101;  // Power-loss recovery
constexpr uint16_t REFUEL_DIALOG_VP         = 0x1102;  // Filament uyarısı
constexpr uint16_t HEATING_DIALOG_VP        = 0x1103;  // Isıtılıyor
constexpr uint16_t NO_FILAMENT_DIALOG_VP    = 0x1104;  // Filament bitti
constexpr uint16_t COOLING_DIALOG_VP        = 0x1105;  // Soğutma onay
constexpr uint16_t PAUSE_DIALOG_VP          = 0x1106;  // Duraklat onay
constexpr uint16_t STOP_DIALOG_VP           = 0x1107;  // Durdur onay
constexpr uint16_t CHOOSE_MODE_DIALOG_VP    = 0x1108;  // Mod seçim
constexpr uint16_t CANCEL_DIALOG_VP         = 0x1109;  // İptal
constexpr uint16_t YES_1_DIALOG_VP          = 0x110A;  // Evet-1
constexpr uint16_t YES_2_DIALOG_VP          = 0x110B;  // Evet-2
constexpr uint16_t NO_1_DIALOG_VP           = 0x110C;  // Hayır-1
constexpr uint16_t NO_2_DIALOG_VP           = 0x110D;  // Hayır-2

// --- Dosya seçim status ikonları (1..20) ---
constexpr uint16_t FILE_SELECT_1_ICON_VP    = 0x1200;
constexpr uint16_t FILE_SELECT_2_ICON_VP    = 0x1201;
constexpr uint16_t FILE_SELECT_3_ICON_VP    = 0x1202;
constexpr uint16_t FILE_SELECT_4_ICON_VP    = 0x1203;
constexpr uint16_t FILE_SELECT_5_ICON_VP    = 0x1204;
constexpr uint16_t FILE_SELECT_6_ICON_VP    = 0x1205;
constexpr uint16_t FILE_SELECT_7_ICON_VP    = 0x1206;
constexpr uint16_t FILE_SELECT_8_ICON_VP    = 0x1207;
constexpr uint16_t FILE_SELECT_9_ICON_VP    = 0x1208;
constexpr uint16_t FILE_SELECT_10_ICON_VP   = 0x1209;
constexpr uint16_t FILE_SELECT_11_ICON_VP   = 0x120A;
constexpr uint16_t FILE_SELECT_12_ICON_VP   = 0x120B;
constexpr uint16_t FILE_SELECT_13_ICON_VP   = 0x120C;
constexpr uint16_t FILE_SELECT_14_ICON_VP   = 0x120D;
constexpr uint16_t FILE_SELECT_15_ICON_VP   = 0x120E;
constexpr uint16_t FILE_SELECT_16_ICON_VP   = 0x120F;
constexpr uint16_t FILE_SELECT_17_ICON_VP   = 0x1210;
constexpr uint16_t FILE_SELECT_18_ICON_VP   = 0x1211;
constexpr uint16_t FILE_SELECT_19_ICON_VP   = 0x1212;
constexpr uint16_t FILE_SELECT_20_ICON_VP   = 0x1213;

// --- Diğer status ikonları ---
constexpr uint16_t FAN_SWITCH_ICON_VP       = 0x1220;
constexpr uint16_t AUTOLEVEL_SWITCH_ICON_VP = 0x1221;
constexpr uint16_t PLA_MODE_ICON_VP         = 0x1222;
constexpr uint16_t ABS_MODE_ICON_VP         = 0x1223;

// --- Dil seçim ikonları ---
constexpr uint16_t LANGUAGE_1_ICON_VP       = 0x1225;
constexpr uint16_t LANGUAGE_2_ICON_VP       = 0x1226;
constexpr uint16_t LANGUAGE_3_ICON_VP       = 0x1227;
constexpr uint16_t LANGUAGE_4_ICON_VP       = 0x1228;
constexpr uint16_t LANGUAGE_5_ICON_VP       = 0x1229;
constexpr uint16_t LANGUAGE_6_ICON_VP       = 0x122A;
constexpr uint16_t LANGUAGE_7_ICON_VP       = 0x122B;
constexpr uint16_t LANGUAGE_8_ICON_VP       = 0x122C;
constexpr uint16_t LANGUAGE_9_ICON_VP       = 0x122D;

// --- Status string single-VP (language_change + DwinStatusSlot) ---
constexpr uint16_t STATUS_DP_CHAR_VP        = 0x1300;

// --- Veri (data) variable'ları ---
constexpr uint16_t NOZZLE_PREHEAT_DATA_VP   = 0x1400;  // Nozül ön-ısıtma sıcaklığı
constexpr uint16_t NOZZLE_TEMP_DATA_VP      = 0x1402;  // Nozül anlık sıcaklığı
constexpr uint16_t BED_PREHEAT_DATA_VP      = 0x1404;  // Yatak ön-ısıtma sıcaklığı
constexpr uint16_t BED_TEMP_DATA_VP         = 0x1406;  // Yatak anlık sıcaklığı

constexpr uint16_t PRINT_TIMEHOUR_DATA_VP   = 0x140B;
constexpr uint16_t PRINT_TIMEMIN_DATA_VP    = 0x140E;

// --- Kalan süre (Sermoon D1 2026-05-23: SHOW_REMAINING_TIME) ---
// 0x1410 — kalan süre (dakika). M73 veya elapsed/pct extrapolation'dan.
// DWIN ekran tasariminda bu VP'ye text alani baglanmali.
// Mevcut DWIN firmware'inde kullanici ekraninda bu alan yoksa da yazilir;
// kullanici gormez ama firmware hesaplamasi dogru calisir.
constexpr uint16_t PRINT_REMAIN_MIN_VP       = 0x1410;

constexpr uint16_t PRINT_SPEED_DATA_VP      = 0x1414;
constexpr uint16_t PRINT_SPEED_KEY          = 0x1414;

constexpr uint16_t CHANGE_FILAMENT_DATA_VP  = 0x1418;
constexpr uint16_t CHANGE_FILAMENT_UNIT_KEY = 0x1418;

constexpr uint16_t AUTO_LEVEL_DATA_VP       = 0x1840;

constexpr uint16_t Z_OFFSET_DISPLAY_VP      = 0x1026;  // Adjust ekranı Z offset göstergesi
constexpr uint16_t Z_OFFSET_DATA_VP         = 0x2100;
constexpr uint16_t TEMP_WARNING_DATA_VP     = 0x2120;

constexpr uint16_t X_MOVE_DATA_KEY          = 0x2112;
constexpr uint16_t Y_MOVE_DATA_KEY          = 0x2114;
constexpr uint16_t Z_MOVE_DATA_KEY          = 0x2116;

// --- Dosya adı text slot'ları (her biri 20 byte, file 1..20) ---
constexpr uint16_t FILE_FIRST_TEXT_VP       = 0x1600;

constexpr uint16_t FILE_DISPLAY_1_TEXT_VP   = 0x1600;
constexpr uint16_t FILE_DISPLAY_2_TEXT_VP   = 0x1614;
constexpr uint16_t FILE_DISPLAY_3_TEXT_VP   = 0x1628;
constexpr uint16_t FILE_DISPLAY_4_TEXT_VP   = 0x163C;
constexpr uint16_t FILE_DISPLAY_5_TEXT_VP   = 0x1650;
constexpr uint16_t FILE_DISPLAY_6_TEXT_VP   = 0x1664;
constexpr uint16_t FILE_DISPLAY_7_TEXT_VP   = 0x1678;
constexpr uint16_t FILE_DISPLAY_8_TEXT_VP   = 0x168C;
constexpr uint16_t FILE_DISPLAY_9_TEXT_VP   = 0x16A0;
constexpr uint16_t FILE_DISPLAY_10_TEXT_VP  = 0x16B4;
constexpr uint16_t FILE_DISPLAY_11_TEXT_VP  = 0x16C8;
constexpr uint16_t FILE_DISPLAY_12_TEXT_VP  = 0x16DC;
constexpr uint16_t FILE_DISPLAY_13_TEXT_VP  = 0x16F0;
constexpr uint16_t FILE_DISPLAY_14_TEXT_VP  = 0x1704;
constexpr uint16_t FILE_DISPLAY_15_TEXT_VP  = 0x1718;
constexpr uint16_t FILE_DISPLAY_16_TEXT_VP  = 0x172C;
constexpr uint16_t FILE_DISPLAY_17_TEXT_VP  = 0x1740;
constexpr uint16_t FILE_DISPLAY_18_TEXT_VP  = 0x1754;
constexpr uint16_t FILE_DISPLAY_19_TEXT_VP  = 0x1768;
constexpr uint16_t FILE_DISPLAY_20_TEXT_VP  = 0x177C;

constexpr uint16_t FILE_SELECTED_TEXT_VP    = 0x1790;

constexpr uint16_t MACHINE_TYPE_TEXT_VP     = 0x17B0;
constexpr uint16_t HW_VERSION_TEXT_VP       = 0x17C4;
constexpr uint16_t FW_VERSION_TEXT_VP       = 0x17D8;
constexpr uint16_t DP_VERSION_TEXT_VP       = 0x17EC;
constexpr uint16_t PRINT_SIZE_TEXT_VP       = 0x1800;
constexpr uint16_t WEBSITE_TEXT_VP          = 0x1814;

#endif // RTS_H
