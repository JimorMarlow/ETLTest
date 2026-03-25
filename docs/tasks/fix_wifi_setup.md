# Errors

## Подключается, но с первого раза не дожидается окончания подлючения

Пытаюсь подключиться к сети sd_wifi, в терминале вижу, что идет подключение и подключается, но на кнопке Join нет кольцевого индикатора и сразу же выпадает с ошибкой "Connection failed. Check password [Retry]"

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
[WiFiSetup] Found 11 networks
[WiFiSetup] Network 1: 228 (RSSI: -53, Encryption: WPA/WPA2)
[WiFiSetup] Network 2: TP-LINK_675E (RSSI: -63, Encryption: WPA/WPA2)
[WiFiSetup] Network 3: room301m (RSSI: -89, Encryption: WPA/WPA2)
[WiFiSetup] Network 4: WiFi-MTD (RSSI: -92, Encryption: WPA2)
[WiFiSetup] Network 5: DIRECT-c3-HP M130 LaserJet (RSSI: -86, Encryption: WPA2)
[WiFiSetup] Network 6:  (RSSI: -85, Encryption: Open)
[WiFiSetup] Network 7: DIRECT-Rv-Pantum P3010 Series (RSSI: -58, Encryption: WPA2)
[WiFiSetup] Network 8: TGP2 (RSSI: -90, Encryption: WPA2)
[WiFiSetup] Network 9: MikroTik-99D048 (RSSI: -91, Encryption: Open)
[WiFiSetup] Network 10: Keenetic-2238 (RSSI: -93, Encryption: WPA2)
[WiFiSetup] Network 11: sd_wifi (RSSI: -43, Encryption: WPA2)
[WiFiSetup] Scan completed: 11 networks
[WiFiSetup] Request: /api/connect
[WiFiSetup] API: /api/connect
[WiFiSetup] Starting async connection to: sd_wifi
[WiFiSetup] Async connection started
[WiFiSetup] Restarting HTTP server after STA connection...
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] mDNS: http://espdevice.local

