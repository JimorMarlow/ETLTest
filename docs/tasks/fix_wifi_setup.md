# Errors

- Разрабатывется web-интерфейс для ESP8266 для выбора и подключения к wi-fi сетям.
код располжен в файлах lib\ETLTest\etl_wifi_setup*
- После исправлений необходимо протестировать компиляцию всех конфигураций
Например, для nodemcuv3 нужно выполнить:
C:\Users\amber\.platformio\penv\Scripts\platformio.exe run -e nodemcuv3 -d c:\Projects\Arduino\ETLTest
- По запросу сделать текст для коммита, но самому в git не выкладывать
- Для обновления контеста можешь посмотреть историю коммитов
- Все изменения записывай для себя в docs\tasks\hystory.md, чтобы потом можно было продолжить

## Нет подключения к выбранной точке доступа

Обновляется список при старте: все хорошо.
Пытаюсь подключиться к сети sd_wifi, в терминале вижу, что идет подключение и подключается, но на кнопке Join нет кольцевого индикатора и сразу же выпадает с ошибкой "Connection failed. Check password [Retry]"
nodemcuv3

UPD1. Ситуация не изменилась. выдается ошибка подключения, при попытке нажать Retry сервер перестает отвечать
Обновил лог
UPD2. Крутящийся спинер на кнопке Join так и не появился. Требование: после нажатия Join вместе текста кнопки должен появиться крутящийся спинер. После подключения или ошибки - должен вернуться нормальный текст. 

[LittleFS] etl::little_fs::begin(): OK
[wifi::settings] init_config()
[wifi::settings] init_config() result: OK
[wifi::settings] resetting to default ...
[wifi::settings] loaded from memory:
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       =
wifi_password   =
port            = 80
update_interval = 500
========================
[wifi::settings] reset to default: FAILED
[WiFiSetup] Initializing...
[wifi::settings] load_config()
[wifi::settings] load_config() loaded from FS
[WiFiSetup] Settings loaded
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       = 
wifi_password   =
port            = 80
update_interval = 500
========================
[WiFiSetup] Loaded saved settings
[WiFiSetup] Starting AP mode...
[WiFiSetup] Starting AP: ESP_Device_AP
[WiFiSetup] AP IP address: 192.168.4.1
[WiFiSetup] AP started successfully
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] mDNS: http://espdevice.local

=== WiFi Server Info ===
Mode:     AP
IP Addr:  192.168.4.1
Hostname: http://192.168.4.1/
mDNS:     http://espdevice.local
=========================

[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 12 networks
[WiFiSetup] Network 1: TP-LINK_675E (RSSI: -70, Encryption: WPA/WPA2)
[WiFiSetup] Network 2: 228 (RSSI: -52, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: room301m (RSSI: -91, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: WiFi-MTD (RSSI: -89, Encryption: WPA2)
[WiFiSetup] Network 5: TP-Link 2.4G (RSSI: -88, Encryption: WPA2)
[WiFiSetup] Network 6: DIRECT-c3-HP M130 LaserJet (RSSI: -90, Encryption: WPA2)
[WiFiSetup] Network 7: DIRECT-Rv-Pantum P3010 Series (RSSI: -61, Encryption: WPA2)
[WiFiSetup] Network 8: MikroTik-99D048 (RSSI: -91, Encryption: Open)
[WiFiSetup] Network 9:  (RSSI: -91, Encryption: WPA2)
[WiFiSetup] Network 10: DIRECT-Cs-Pantum M6800 Series (RSSI: -88, Encryption: WPA2)
[WiFiSetup] Network 11: TP-Link_Е4567 (RSSI: -93, Encryption: WPA2)
[WiFiSetup] Network 12: sd_wifi (RSSI: -43, Encryption: WPA2)
[WiFiSetup] Scan completed: 12 networks
[WiFiSetup] Request: /api/connect
[WiFiSetup] API: /api/connect
[WiFiSetup] Connecting to: sd_wifi
......
[WiFiSetup] Connected successfully
[WiFiSetup] IP address: 192.168.88.93
[WiFiSetup] Restarting HTTP server after STA connection...
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] mDNS: http://espdevice.local

## Если после подключения нажать "Обновить", то сервер перестает отвечать

UPD. Ошибка не ушла