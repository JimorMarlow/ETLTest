# Errors in task_webui.md

## Сервер контента - визуальные проблемы

Светлая тема, при нажатии на кнопку [power] видны черные края квадрата вокруг кноки включения питания. В темной теме не видно.

## Ошибка возвращения из server_setup

Не дожидаясь окончания сканирования сети изменил настройки интерфейса и нажал [Save & Reboot]
сервер перестал отвечать.

[WiFiSetup] Stopped
[LightWebUIMgr] Creating settings server...
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
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] mDNS already running: http://espdevice.local
[WiFiSetup] mDNS service added and updated
[WebManager] Settings server started
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 3 networks
[WiFiSetup] Network 1:  (RSSI: -62, Encryption: Open)
[WiFiSetup] Network 2: FIREFLY (RSSI: -63, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: FIREFLY_HOUSE (RSSI: -88, Encryption: WPA/WPA2)
[WiFiSetup] Scan completed: 3 networks
