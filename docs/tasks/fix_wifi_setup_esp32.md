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
- сервер запустился нормально
- открылся на телефоне, выполнил подключение к sd_wifi. Подключено. нажал [Save & Reboot]. выполнилась перезагрузка, поключение к sd_wifi осталось
- открылся с десктопа по локальной сети, проверил подключение, выбрал sd_wifi из списка и нажал [Disconnect]

[WiFiSetup] Connected successfully
[WiFiSetup] IP address: 10.0.1.99
[WiFiSetup] Subnet Mask: 255.0.0.0
[WiFiSetup] Gateway IP: 10.0.0.1
[WiFiSetup] DNS IP: 10.0.0.100
[WiFiSetup] Request: /api/disconnect
[WiFiSetup] API: /api/disconnect
[WiFiSetup] Disconnected from WiFi, AP restarted
[WiFiSetup] Request: /api/ap_settings
[WiFiSetup] API: /api/ap_settings
[WiFiSetup] Starting AP: ESP_Device_test
[WiFiSetup] AP IP address: 192.168.4.1
[WiFiSetup] AP restarted, client should reconnect
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 10 networks
[WiFiSetup] Network 1: sd_wifi (RSSI: -54, Encryption: WPA2)
[WiFiSetup] Network 2: 228 (RSSI: -63, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: TP-LINK_675E (RSSI: -78, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: BIOMED2 (RSSI: -83, Encryption: WPA/WPA2)
[WiFiSetup] Network 5: TP-LINK_0572 (RSSI: -87, Encryption: WPA2)
[WiFiSetup] Network 6: DIRECT-Cs-Pantum M6800 Series (RSSI: -87, Encryption: WPA2)
[WiFiSetup] Network 7: room301m (RSSI: -91, Encryption: WPA/WPA2)
[WiFiSetup] Network 8: WiFi-MTD (RSSI: -92, Encryption: WPA2)
[WiFiSetup] Network 9: HUAWEI-222 (RSSI: -92, Encryption: WPA2)
[WiFiSetup] Network 10: NVRa486dbb5e8b1 (RSSI: -94, Encryption: WPA2)
[WiFiSetup] Scan completed: 10 networks