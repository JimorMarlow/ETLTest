# Errors

## Подключается, но с первого раза не дожидается окончания подлючения

после обновления списка в режиме AP, выбрал sd_wifi, ввел пароль, по логам вижу, что идет подключение, 
однако в интерфейсе уже вижу ошибку "Не удалось подключиться [Retry]". Дожидаюсь завершения подключения, нажимаю Retry, в этот раз подлкючается и отмечает зеленой галочкой подключение. По логам видно, что выполнялось второе подлючение

UPD. Все равно не дожидается первого подключения, приходится нажимать Retry

UPD2. Теперь работает корректно - ошибка исправлена.
Нужно добавить на кнопку JOIN кольцевой индикатор на время, пока идет подключение к точке (вместо текста JOIN пока идет ожидание обновления до получения коннекта или ошибки)

## Не отображается выбранная сеть

После подключения в списке сетей есть галочка, что сеть подключена и снизу кнопка [Отключить], но в верхнем контейнере со статусом пишет 
```
Не подключено   [Обновить]
sd_wifi - undefined
```
После "Save & Reboot" - то же самое, выбранная сеть с галочкой в списке. В контейнере статуса над списком - "Не подключено"

PS. Кнопка [Отключить] бывает разная, то красного цвета целиком с белым текстом, то красный текст с белой рамкой. Сделай, чтобы было одинаково - с заливкой красного цвета.

UPD: 
- Кнопка работает правильно. ОК
- Выбранная сеть показывается после "Save & Reboot" и в списке и в контейнере статуса. ОК


## Подключение к сети, не выводится IP 

Подключилось корректно, вывело полученный IP, но в трейсе "IP Addr:  (IP unset)"
Подключил телефон к этой же сети sd_wifi, и не могу открыть страницу по адресу: http://espdevice.local

UPD. Не изменилось, вижу, что подключилась к sd-wifi, но IP должен быть из 10.*.*.*, а тут как для локальной сети

[LittleFS] etl::little_fs::begin(): OK
[wifi::settings] init_config()
[wifi::settings] init_config() result: OK
[WiFiSetup] Initializing...
[wifi::settings] load_config()
[wifi::settings] load_config() loaded from FS
[WiFiSetup] Settings loaded
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       = sd_wifi
wifi_password   = xsw2xsw2xsw2
port            = 80
update_interval = 500
========================
[WiFiSetup] Loaded saved settings
[WiFiSetup] Connecting to saved network: sd_wifi
[WiFiSetup] Connecting to sd_wifi
........
[WiFiSetup] Connected
[WiFiSetup] IP address: 192.168.88.93
[WiFiSetup] Connected to saved network
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] mDNS: http://espdevice.local

=== WiFi Server Info ===
Mode:     AP
IP Addr:  (IP unset)
Hostname: http://192.168.4.1/
mDNS:     http://espdevice.local
=========================

## Пропадает признак connected у элемента списка при смене фокуса

- Если подключиться к точке, нажать на нее - показывает кнопка "Отключиться"
- Если нажать на другую кнопку - появится полее ввода пароля и [Join]
- Выбираем обратно точку, к которой подключены - тоже появляется поле ввода пароля и [Join], статус подключенности тоже пропадает. В верхнем контейнера со стусом - все ОК.
