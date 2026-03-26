# Ошибки найденные при тестировании задачи 

задача: docs\tasks\task_wifi_setup_esp32.md
история: docs\tasks\task_wifi_setup_esp32_hystory.md

## Не виден сервер после перезагрузки

- Запустил новую версию со сбросом данных.
- Изменил имя и пароль AP, сохранил.
- переподключился к новой точке, обновил страницу http://espdevice.local, все работает
- подключился к sd_wifi, нажал [Save & Reboot]
- Пытаюсь подключиться с десктопа, подключенного к этой же сети, не видит. С телефона тоже.
- Заново обновил прошивку без сброса данных, сервер стартует, но не виден. Данные считались нормальные

Вот логи


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
ap_password     = password1234
wifi_ssid       = sd_wifi
wifi_password   = xsw2xsw2xsw2
port            = 80
update_interval = 500
========================
[WiFiSetup] Loaded saved settings
[WiFiSetup] Connecting to saved network: sd_wifi
[WiFiSetup] Connecting to sd_wifi
.
[WiFiSetup] Connected
[WiFiSetup] IP address: 10.0.5.151
[WiFiSetup] Connected to saved network
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] Initializing mDNS: [WiFiSetup] mDNS: http://espdevice.local
[WiFiSetup] mDNS service added and updated

=== WiFi Server Info ===
Mode:     OFF
IP Addr:  0.0.0.0
Hostname: http://espdevice.local