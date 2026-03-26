# Ошибки найденные при тестировании задачи 

задача: docs\tasks\task_wifi_setup_esp32.md
история: docs\tasks\task_wifi_setup_esp32_hystory.md

## Не сохранились изменения при смене имени точки доступа AP 

После смены AP SSID в контейнере Access Points Settings
"ESP_Device_AP" -> "ESP_Device_test"
[Apply AP Settings]
настройки применились, точка перезапустилась с новым именем. Нашел ее в сети, подключился и открыл 
http://espdevice.local
По логам все верно, но в настройках осталось значение "ESP_Device_AP". Нужно сохранять настройки и после перезапуска брать новые.

среда тестирования: [env:esp32c3]

UPD: После изменения настроек по [Apply AP Settings] настройки изменились на "ESP_Device_test"
нажел [Save & Reboot] - точка работает как "ESP_Device_test", но в настройках показывает "ESP_Device_AP"
отключил устройство и подключил заново, открыл http://espdevice.local в другой странице - тоже самое.

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
[WiFiSetup] Found 7 networks
[WiFiSetup] Network 1: sd_wifi (RSSI: -52, Encryption: WPA2)
[WiFiSetup] Network 2: 228 (RSSI: -65, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: TP-LINK_675E (RSSI: -74, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: TP-LINK_0572 (RSSI: -84, Encryption: WPA2)
[WiFiSetup] Network 5: SNR-CPE-85E0 (RSSI: -86, Encryption: WPA2)
[WiFiSetup] Network 6: BIOMED2 (RSSI: -89, Encryption: WPA/WPA2)
[WiFiSetup] Network 7: room301m (RSSI: -90, Encryption: WPA/WPA2)
[WiFiSetup] Scan completed: 7 networks
[WiFiSetup] Request: /api/ap_settings
[WiFiSetup] API: /api/ap_settings
[WiFiSetup] Saving settings...
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_test
ap_password     = password123
wifi_ssid       =
wifi_password   =
port            = 80
update_interval = 500
========================
[wifi::settings] save_config()
[wifi::settings] save_config() result: OK
OK
[WiFiSetup] Starting AP: ESP_Device_test
[WiFiSetup] AP IP address: 192.168.4.1
[WiFiSetup] AP restarted, client should reconnect
[WiFiSetup] Request: /api/save
[WiFiSetup] API: /api/save
[WiFiSetup] Saving settings...
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_test
ap_password     = password123
wifi_ssid       =
wifi_password   =
port            = 80
update_interval = 500
========================
[wifi::settings] save_config()
[wifi::settings] save_config() result: FAILED
FAILED
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 7 networks
[WiFiSetup] Network 1: sd_wifi (RSSI: -52, Encryption: WPA2)
[WiFiSetup] Network 2: 228 (RSSI: -67, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: TP-LINK_675E (RSSI: -76, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: TP-LINK_0572 (RSSI: -82, Encryption: WPA2)
[WiFiSetup] Network 5: BIOMED2 (RSSI: -86, Encryption: WPA/WPA2)
[WiFiSetup] Network 6: room301m (RSSI: -91, Encryption: WPA/WPA2)
[WiFiSetup] Network 7: DIRECT-Cs-Pantum M6800 Series (RSSI: -91, Encryption: WPA2)
[WiFiSetup] Scan completed: 7 networks