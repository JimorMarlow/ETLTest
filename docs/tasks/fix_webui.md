# Errors in task_webui.md

## Сервер контента - переход в настройки

После нажатия на кнопку [settings] выполняется запуск сервера настроек. Он стартовал успешно.
Нужно сделать вывод сообщения "Loading setup..." и кольцевой индикатор с обновлением страницы через 10 секунд, чтобы после нажатия было время запустить сервер настроек и страница автоматически обновилась

## Ошибка возвращения из server_setup

Нажал на [Save & Reboot] - сервер настроек успешно сохранил настройки, но перезагрузился опять в сервер настроек. Нужно, чтобы после этого запускался сервер контента.

Нажал на [Back] - произошло падение и перезагрузка микроконтроллера


========================
[WebUI] UI settings loaded
[WiFiSetup] Loaded saved settings
[WiFiSetup] Connecting to saved network: FIREFLY
[WiFiSetup] Connecting to FIREFLY
..
[WiFiSetup] Connected
[WiFiSetup] IP address: 192.168.31.221
[WiFiSetup] Connected to saved network
[LightControl] Starting HTTP server...
[LightControl] Setting up HTTP routes...
[LightControl] HTTP server started on port 80
Guru Meditation Error: Core  0 panic'ed (Load access fault). Exception was unhandled.

Core  0 register dump:
MEPC    : 0x42099912  RA      : 0x4200f824  SP      : 0x3fc9e6f0  GP      : 0x3fc8e200
TP      : 0x3fc659cc  T0      : 0x4005890e  T1      : 0x3fc9e34c  T2      : 0x67694c5b
S0/FP   : 0x3fca9b64  S1      : 0x3fca6588  A0      : 0x3fca9b8c  A1      : 0x3fca6588
A2      : 0x00000003  A3      : 0x0000000a  A4      : 0x00000004  A5      : 0x00000001
A6      : 0x3fe00000  A7      : 0x00000009  S2      : 0x00000000  S3      : 0x3fc9e78c
S4      : 0x42011a5e  S5      : 0x00000000  S6      : 0x00000000  S7      : 0x00000000
S8      : 0x00000000  S9      : 0x00000000  S10     : 0x00000000  S11     : 0x00000000
T3      : 0x7520676e  T4      : 0x69747465  T5      : 0x53205d6c  T6      : 0x6f72746e
MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000005  MTVAL   : 0x00000055
MHARTID : 0x00000000

Stack memory:
3fc9e6f0: 0x3fc9ad80 0x3fc91000 0x3fc9ad4c 0xbaad5678 0x3fc9fed4 0x00000000 0x0000000b 0x4038bf92
3fc9e710: 0x0000000b 0x3fc981b8 0x00001800 0x403828aa 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e730: 0x00000000 0x00000000 0x42011a5e 0x4200f80e 0x00001800 0x00000000 0x00000000 0x3c0c93e8
3fc9e750: 0x3fc97064 0x600c2000 0x00000000 0x00000009 0x00000001 0x00000000 0x3fc9e78c 0x3fc97000
3fc9e770: 0x00000001 0x3fca9b64 0x3fc97000 0x420105f8 0x00000000 0x00000000 0x09c9e7ec 0x00000000
3fc9e790: 0x00000000 0x00000000 0x80001005 0x72e02595 0x3fcabfdc 0x3fca9bbc 0x3fca9b64 0x42011b5a
3fc9e7b0: 0x3fc9e808 0x00000008 0x00000000 0x00000003 0x00000000 0x00001000 0x3fca9b8c 0x4209a05c
3fc9e7d0: 0x00000008 0x3fc9e808 0x00001005 0x00000fff 0x00000000 0x3fca9b8c 0x00000000 0x00000000
3fc9e7f0: 0x00000000 0x00000000 0x09ca9b8c 0x72e02595 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e810: 0x00000000 0x3fca9b8c 0x3fca9b64 0x42011d2a 0x00000000 0x3c0f0738 0x00000000 0x000003e8
3fc9e830: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x3fc97000 0x00000bb8 0x00000000
3fc9e850: 0x00000000 0x3fc97000 0x00000000 0x4201d410 0x00000000 0x00000000 0x00000000 0x403898dc
3fc9e870: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5
3fc9e890: 0xa5a5a5a5 0xbaad5678 0x00000160 0xabba1234 0x00000154 0x3fc9e200 0x0002159a 0x3fc92ebc
3fc9e8b0: 0x3fc92ebc 0x3fc9e8a4 0x3fc92eb4 0x00000018 0x3fca895c 0x3fca895c 0x3fc9e8a4 0x00000000
3fc9e8d0: 0x00000001 0x3fc9c894 0x706f6f6c 0x6b736154 0xc0aa9100 0x00505887 0x00000000 0x3fc9e890
3fc9e8f0: 0x00000001 0x00000000 0x3fca6d9c 0x42026096 0x00000009 0x3fc984c4 0x3fc9852c 0x3fc98594
3fc9e910: 0x00000000 0x00000000 0x00000001 0x00000000 0x00000000 0x00000000 0x420a328e 0x00000000
3fc9e930: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e950: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e970: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e990: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e9b0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e9d0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fc9e9f0: 0x00000000 0xe9000000 0xbaad5678 0x00000060 0xabba1234 0x00000054 0x00000000 0x3fc9ea08
3fc9ea10: 0x00000000 0x00000000 0x00000000 0x3fc9ea20 0xffffffff 0x3fc9ea20 0x3fc9ea20 0x00000000
3fc9ea30: 0x3fc9ea34 0xffffffff 0x3fc9ea34 0x3fc9ea34 0x00000001 0x00000001 0x00000000 0x6300ffff
3fc9ea50: 0x00000000 0xb33fffff 0x00000000 0xbaad5678 0x00000160 0xabba1234 0x00000154 0x3fc9eac0
3fc9ea70: 0x3fc9eac0 0x3fc9ebc0 0x3fc9ebbf 0x00000000 0x3fc9ea84 0xffffffff 0x3fc9ea84 0x3fc9ea84
3fc9ea90: 0x00000000 0x3fc9ea98 0xffffffff 0x3fc9ea98 0x3fc9ea98 0x00000000 0x00000100 0x00000001
3fc9eab0: 0x8300ffff 0x00000000 0xb33fffff 0x00000000 0x10002a58 0x26a01401 0x39a30368 0x80b98497
3fc9ead0: 0xfc1fa7f6 0x879e0cfb 0xfdedfefb 0xf1342b99 0x7224e241 0x05111029 0x952c3111 0x3b14fdce



ELF file SHA256: 8e4b0c9acaa5b1c5

E (7792) esp_core_dump_flash: Core dump flash config is corrupted! CRC=0x7bd5c66f instead of 0x0
Rebooting...
ESP-ROM:esp32c3-api1-20210207
Build:Feb  7 2021
rst:0x3 (RTC_SW_SYS_RST),boot:0xf (SPI_FAST_FLASH_BOOT)
Saved PC:0x4038202c
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fcd5810,len:0x438
load:0x403cc710,len:0x90c
load:0x403ce710,len:0x2624
entry 0x403cc710
E (191) esp_core_dump_flash: No core dump partition found!
E (191) esp_core_dump_flash: No core dump partition found!

=================================
  ETL Test Project
=================================
Version: 0.2.12

Running ETL tests...

--- Embedded Template Library ---
MEMORY: 275204 diff =     0 | test_empty                - OK
MEMORY: 275204 diff =     0 | test_optional             - OK
MEMORY: 275204 diff =     0 | test_unique               - OK
MEMORY: 275156 diff =    48 | test_shared_weak          - OK
MEMORY: 275204 diff =   -48 | test_queue                - OK
MEMORY: 275204 diff =     0 | test_vector               - OK
MEMORY: 275204 diff =     0 | test_array                - OK
MEMORY: 275204 diff =     0 | test_espnow               - OK
MEMORY: 275204 diff =     0 | test_lookup               - OK
MEMORY: 275156 diff =    48 | test_color_lookup         - OK
MEMORY: 275156 diff =     0 | test_color_spectrum       - OK
MEMORY: 275172 diff =   -16 | test_algorythm            - OK
MEMORY: 272952 diff =  2220 | test_littlefs             - OK
MEMORY: 272952 diff =     0 | test_settings             - OK
MEMORY: 272952 diff =     0 | test_empty                - OK
--------------------------------
MEMORY LEAKS: 0 OK
sizeof: int = 4, float = 4, double = 8
--------------------------------

=================================
  All tests completed
=================================

[LittleFS] etl::little_fs::begin(): OK
[wifi::settings] init_wifi_config()
[wifi::settings] init_wifi_config() result: OK
[wifi::settings] init_ui_config()
[wifi::settings] init_ui_config() result: OK
[WebManager] Starting content server...
[LightWebUIMgr] Creating content server...
[wifi::settings] load_wifi_config()
[wifi::settings] load_wifi_config() loaded from FS
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       = FIREFLY
wifi_password   = 87302998
port            = 80
update_interval = 500
========================
[WiFiSetup] Initializing...
[WebUI] Loading settings...
[wifi::settings] load_wifi_config()
[wifi::settings] load_wifi_config() loaded from FS
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       = FIREFLY
wifi_password   = 87302998
port            = 80
update_interval = 500
========================
[WebUI] WiFi settings loaded
[wifi::settings] load_ui_config()
[wifi::settings] load_ui_config() loaded from FS
=== ui_config_t settings ===
language        = en
dark_theme      = ✅
large_font      = ⬜
use_bold_values = ✅
========================
[WebUI] UI settings loaded
[WiFiSetup] Loaded saved settings
[WiFiSetup] Connecting to saved network: FIREFLY
[WiFiSetup] Connecting to FIREFLY
..
[WiFiSetup] Connected
[WiFiSetup] IP address: 192.168.31.221
[WiFiSetup] Connected to saved network
[LightControl] Starting HTTP server...
[LightControl] Setting up HTTP routes...
[LightControl] HTTP server started on port 80
[LightControl] Initializing mDNS: [LightControl] mDNS: http://espdevice.local
[LightControl] mDNS service added and updated
[WebManager] Content server started

=== WiFi Server Info ===
Server:   Content
Mode:     AP+STA
IP Addr:  192.168.31.221
Hostname: http://espdevice.local
mDNS:     http://espdevice.local
=========================

[WiFiSetup] Restarting HTTP server after STA connection...
[LightControl] Starting HTTP server...
[LightControl] Setting up HTTP routes...
[LightControl] HTTP server started on port 80
[LightControl] mDNS already running: http://espdevice.local
[LightControl] mDNS service added and updated
