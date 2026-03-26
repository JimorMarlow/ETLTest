# Ошибки найденные при тестировании задачи 

задача: docs\tasks\task_wifi_setup_esp32.md
история: docs\tasks\task_wifi_setup_esp32_hystory.md

## Не сохранились изменения при смене имени точки доступа AP 

- В FS считаны настройки с "ESP_Device_test". Запускается точка доступа с этим именем, но когда подключаюсь и открываю в браузере, вижу в настройках "ESP_Device_AP". 
- После смены AP SSID в контейнере Access Points Settings
"ESP_Device_AP" -> "ESP_Device_test"
[Apply AP Settings]
настройки применились, точка перезапустилась с тем же именем.
По логам все верно, в настройках теперь значение "ESP_Device_test"
- нажимаю [Save & Reboot] - точка работает как "ESP_Device_test", но в настройках показывает "ESP_Device_AP"
- закрыл страницу и открыл заново в другой вкладке - тоже самое
- открыл с дескотопа в контейнере "Настройки точки доступа" вижу SSID точки доступа: ESP_Device_AP

Проверь, в поля настроек попадают имя и пароль для режима AP из считанных настроек, и обновляются ли они.

среда тестирования: [env:esp32c3]


[LittleFS] etl::little_fs::begin(): OK
[wifi::settings] init_config()
[wifi::settings] init_config() result: OK
[WiFiSetup] Initializing...
[wifi::settings] load_config()
[wifi::settings] load_config() loaded from FS
[WiFiSetup] Settings loaded
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_test
ap_password     = password123
wifi_ssid       =
wifi_password   =
port            = 80
update_interval = 500
========================
[WiFiSetup] Loaded saved settings
[WiFiSetup] Starting AP mode...
[WiFiSetup] Starting AP: ESP_Device_test
[WiFiSetup] AP IP address: 192.168.4.1
[WiFiSetup] AP started successfully
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] Initializing mDNS: [WiFiSetup] mDNS: http://espdevice.local
[WiFiSetup] mDNS service added and updated

=== WiFi Server Info ===
Mode:     OFF
IP Addr:  192.168.4.1
Hostname: http://espdevice.local
mDNS:     http://espdevice.local
=========================

[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 8 networks
[WiFiSetup] Network 1: sd_wifi (RSSI: -53, Encryption: WPA2)
[WiFiSetup] Network 2: DIRECT-Rv-Pantum P3010 Series (RSSI: -58, Encryption: WPA2)
[WiFiSetup] Network 3: 228 (RSSI: -62, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: TP-LINK_675E (RSSI: -68, Encryption: WPA/WPA2)
[WiFiSetup] Network 5: DIRECT-Cs-Pantum M6800 Series (RSSI: -85, Encryption: WPA2)
[WiFiSetup] Network 6: TP-LINK_0572 (RSSI: -89, Encryption: WPA2)
[WiFiSetup] Network 7:  (RSSI: -91, Encryption: Unknown)
[WiFiSetup] Network 8: Keenetic-2238 (RSSI: -92, Encryption: WPA2)
[WiFiSetup] Scan completed: 8 networks
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan