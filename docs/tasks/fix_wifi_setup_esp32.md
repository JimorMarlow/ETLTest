# Ошибки найденные при тестировании задачи 

задача: docs\tasks\task_wifi_setup_esp32.md
история: docs\tasks\task_wifi_setup_esp32_hystory.md

## При нажатии Disconnect появляется ошибка и сервер перестет отвечать

среда тестирования: [env:esp32c3]
- сервер запустился нормально
- открылся на телефоне, выполнил подключение к sd_wifi. Подключено. нажал [Save & Reboot]. выполнилась перезагрузка, поключение к sd_wifi осталось
- открылся с десктопа по локальной сети, проверил подключение, выбрал sd_wifi из списка и нажал [Disconnect]

[WiFiSetup] Scanning networks...
[WiFiSetup] Found 9 networks
[WiFiSetup] Network 1: sd_wifi (RSSI: -54, Encryption: WPA2)
[WiFiSetup] Network 2: 228 (RSSI: -66, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: DIRECT-Rv-Pantum P3010 Series (RSSI: -74, Encryption: WPA2)
[WiFiSetup] Network 4: TP-LINK_675E (RSSI: -81, Encryption: WPA/WPA2)
[WiFiSetup] Network 5: SNR-CPE-85E0 (RSSI: -82, Encryption: WPA2)
[WiFiSetup] Network 6: DIRECT-Cs-Pantum M6800 Series (RSSI: -88, Encryption: WPA2)
[WiFiSetup] Network 7: TP-LINK_0572 (RSSI: -89, Encryption: WPA2)
[WiFiSetup] Network 8: room301m (RSSI: -91, Encryption: WPA/WPA2)
[WiFiSetup] Network 9: TT_FREE (RSSI: -94, Encryption: Open)
[WiFiSetup] Scan completed: 9 networks
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Returning cached scan results
[WiFiSetup] Request: /api/disconnect
[WiFiSetup] API: /api/disconnect
[WiFiSetup] Disconnected from WiFi, AP restarted
[174495][E][WiFiClient.cpp:429] write(): fail on fd 49, errno: 113, "Software caused connection abort"