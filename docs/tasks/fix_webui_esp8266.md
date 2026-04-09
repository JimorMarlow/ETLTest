# Проблема запуска сервера настроек на esp8266

После оптимизации памяти на ESP8266 нормальн заработал сервер контента и сервер настроек в режиме AP.
В режиме подключения к другой сети в режиме STA что-то не успевает выполниться и не обновляются PLACEHOLDER в HTML и не устанавливаются настройки интерфейса в страницу.

## Что сделать

- Пока не нужно ничего исправлять, мы вчера пытались разными способами, не получилось, я откатил изменения.
Давай попробуем диагностировать проблему, код пока не меняй, предложи варианты.

- Объясни как работает код сервера настроек. В какой момент после загрузки страницы выполняется установка настроек интерфейса и device_info? Какие скрипты запускаются на стороне html. Я попробую еще сам посмотреть и разобраться.
Запиши всю информацию в раздел Архитектура.

## Требоавния к разработке

- После исправлений необходимо протестировать компиляцию всех конфигураций из platformio.ini
нужно выполнить:
C:\Users\amber\.platformio\penv\Scripts\platformio.exe run -e <имя конфигурации> -d c:\Projects\Arduino\ETLTest
PS. Если platformio.exe не найден по этому пути, попробуй этот "C:\Users\jimor\.platformio\penv\Scripts\platformio.exe"
PS. для запуска скриптов сборки не нужно спрашивать разрешение.
- По запросу сделать текст для коммита, но самому в git не выкладывать
- Для обновления контеста можешь посмотреть историю коммитов
- Все изменения записывай для себя в этот файл в конец в раздел "History", чтобы потом можно было продолжить

## Архитектура

### Общая структура серверов

**Два режима работы:**
1. **Content Server** (`light_control_server`) — основной сервер управления устройством
2. **Settings Server** (`server_setup`) — сервер настройки WiFi

**Классы:**
- `web_server_base_t` (lib/ETLTest/etl_webui_base.cpp) — базовый класс, общая логика WiFi, HTTP, callbacks
- `web_manager` (lib/ETLTest/etl_webui_base.cpp) — менеджер переключения между серверами
- `server_setup` (lib/ETLTest/etl_webui.cpp) — сервер настроек
- `light_control_server` (src/light_webui.cpp) — контент-сервер

### Как работает Settings Server

**Инициализация:**
1. `web_manager::start_settings()` останавливает текущий сервер
2. Вызывает `on_create_settings()` → создает `server_setup`
3. Вызывает `m_server->begin(device_info)`:
   - Загружает WiFi config из FS
   - Загружает UI config из FS
   - Подключается к STA или запускает AP
   - Запускает HTTP сервер (`start_http_server()`)
   - Инициализирует mDNS

**Обработка запросов (`server_setup`):**
- `GET /` → отдает HTML_TEMPLATE из PROGMEM
- `GET /api/config` → возвращает device_info + UI config JSON
- `GET /api/status` → статус подключения
- `GET /api/scan` → сканирование WiFi сетей

### JavaScript на стороне HTML (Settings Server)

**Init функция запускается немедленно при загрузке скрипта:**
```javascript
async function init() {
    setLanguage(currentLang);       // Применить переводы
    updateStatusUI();               // Показать "Disconnected"
    applyDeviceConfig();            // Первая попытка (deviceConfig пустой)
    try {
        await loadDeviceConfig();   // FETCH /api/config → JSON с device_name, description, icon, ui settings
        applyDeviceConfig();        // Заменяет DEVICE_NAME_PLACEHOLDER, DEVICE_DESC_PLACEHOLDER, icon
        applyUISettings();          // Применяет dark_theme, large_font, bold_values
    } catch (error) { ... }
    await checkConnectionStatus();  // FETCH /api/status
    setTimeout(() => { scanNetworks(); }, INITIAL_SCAN_DELAY);  // FETCH /api/scan через 2 сек
}
```

**Ключевые моменты:**
- `loadDeviceConfig()` делает `fetch('/api/config')` и заполняет `window.deviceConfig`
- `applyDeviceConfig()` заменяет placeholder'ы: `deviceName.textContent = window.deviceConfig.deviceName`
- `applyUISettings()` проверяет `window.deviceConfig.ui_config_initialized` и применяет тему/шрифт

### Summary of Server Lifecycle
```
boot
  |
  v
web_manager::start_content()
  |
  +-> on_create_content() -> light_control_server
  +-> begin() -> connect_from_saved_config() -> STA or AP
  +-> start_http_server() -> ESP8266WebServer created, routes set
  |
  v
loop() -> webui_manager->tick() -> m_server->tick() -> handle() + handle_client()
  |
  v
User clicks "Settings" on content server
  |
  +-> POST /api/settings -> send_success_response() + schedule_settings_cb()
  +-> 10 ticks later in handle_client() -> m_on_settings_cb() -> start_settings()
  |
  v
web_manager::start_settings()
  |
  +-> stop() old server (MDNS.end, server->stop, WiFi.disconnect)
  +-> on_create_settings() -> server_setup
  +-> begin() -> load_settings() -> connect_to_sta() or start_ap()
  +-> start_http_server() -> new ESP8266WebServer
  |
  v
Browser loads settings page
  |
  +-> GET / -> HTML_TEMPLATE sent
  +-> GET /api/config -> device info + UI config JSON
  +-> GET /api/status -> connection status JSON
  +-> GET /api/scan -> WiFi networks JSON
  |
  v
  JavaScript init() replaces placeholders and applies UI settings
  ```

### Проблема STA режима

**Механизм перезапуска HTTP сервера (`web_server_base_t::handle()`):**
```cpp
if (is_connected() && !http_server_restarted) {
    if (connection_time == 0) connection_time = millis();
    if (millis() - connection_time > 5000) {
        // Перезапуск HTTP сервера после STA подключения
        m_server->stop();
        m_server.reset();
        start_http_server();  // ← Сервер пересоздается!
        http_server_restarted = true;
        connection_time = 0;
    }
}
```

**Первоначальный диагноз (неверный):** Думали что проблема в перезапуске сервера — JavaScript делает `fetch('/api/config')` во время перезапуска и получает обрыв соединения.

**Реальный корень проблемы:** `serializeJson(doc, String)` на ESP8266 **обрезает большие JSON** (~5.7KB).

Причина: на ESP8266 с ~16KB свободной кучи `String` не может аллоцировать непрерывный блок памяти нужного размера. ArduinoJson успешно сериализует данные в память, но при копировании в `String` происходит **тихая обрезка**:

```
measureJson(doc)  → 5697 bytes (ожидаемый размер)
response.length() → 4867 bytes (фактический — обрезан на ~830 байт!)
```

В результате браузер получает **невалидный JSON** с ошибкой:
```
SyntaxError: Unterminated string in JSON at position 4851
```

Строка `device_icon_svg` (большая SVG иконка) обрезается на середине — JSON не может распарситься, `loadDeviceConfig()` падает, placeholder'ы не заменяются, UI настройки не применяются.

**Почему проявлялось именно в STA режиме:** В STA режиме после подключения перезапускается HTTP сервер, что дополнительно фрагментирует и без того ограниченную кучу ESP8266. В AP режиме сервер запускается один раз и куча более стабильна.

### Потенциальные решения

~~1. **Увеличить таймаут в JS** — добавить retry логику в `loadDeviceConfig()`~~ ❌ Не решает корень проблемы

~~2. **Изменить тайминг перезапуска** — увеличить 5 сек до 10-15 сек~~ ❌ Не решает корень проблемы

~~3. **Проверка ответа в JS** — retry при ошибке~~ ❌ Не помогает, JSON всегда битый

~~4. **Отключить автоперезапуск**~~ ❌ Не решает корень проблемы

~~5. **Graceful restart**~~ ❌ Избыточно

✅ **РЕШЕНИЕ (реализовано):** Отправлять JSON напрямую через `WiFiClient` без промежуточного `String`:
1. Формируем HTTP заголовки вручную
2. `serializeJson(doc, m_server->client())` — сериализация напрямую в `Print` интерфейс
3. Больше не требуется аллокация `String` → нет обрезки

## Логи для исправления
--- терминал в VSCode ---
[LightControl] API: /api/settings - scheduling settings callback
[WebUI] Executing pending settings callback
[WebManager] Starting settings server...
[WebManager] Stopping current server...
[WiFiSetup] Stopping...
[WiFiSetup] Stopped
[LightWebUIMgr] Creating settings server...
[wifi::settings] load_wifi_config()
[wifi::settings] load_wifi_config() loaded from FS
=== server_config_t settings ===
hostname        = espdevice
ap_ssid         = ESP_Device_AP
ap_password     = password123
wifi_ssid       = sd_wifi
wifi_password   = xsw2xsw2xsw2
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
wifi_ssid       = sd_wifi
wifi_password   = xsw2xsw2xsw2
port            = 80
update_interval = 500
========================
[WebUI] WiFi settings loaded
[wifi::settings] load_ui_config()
[wifi::settings] load_ui_config() loaded from FS
=== ui_config_t settings ===
language        = ru
dark_theme      = ⬜
large_font      = ✅
use_bold_values = ⬜
========================
[WebUI] UI settings loaded
[WiFiSetup] Loaded saved settings
[WiFiSetup] Connecting to saved network: sd_wifi
[WiFiSetup] Connecting to sd_wifi
.......
[WiFiSetup] Connected
[WiFiSetup] IP address: 10.0.5.180
[WiFiSetup] Connected to saved network
[WiFiSetup] Starting HTTP server...
[WiFiSetup] Setting up HTTP routes...
[WiFiSetup] HTTP server started on port 80
[WiFiSetup] Initializing mDNS: [WiFiSetup] mDNS: http://espdevice.local
[WiFiSetup] mDNS service added and updated
[WebManager] Settings server started
[WiFiSetup] Request: /
[WiFiSetup] Serving root page...
[WiFiSetup] Page sent
[WiFiSetup] Request: /api/config
[WiFiSetup] /api/config start, free heap: 16032
[WiFiSetup] /api/config JSON measured size: 5697 bytes
[WiFiSetup] /api/config headers sent (124 bytes), free heap: 8792
[WiFiSetup] /api/config JSON sent (5697 bytes), free heap: 7496
[WiFiSetup] Request: /api/status
[WiFiSetup] Request: /api/scan
[WiFiSetup] API: /api/scan
[WiFiSetup] Scanning networks...
[WiFiSetup] Found 7 networks
[WiFiSetup] Network 1: TP-LINK_675E (RSSI: -66, Encryption: WPA/WPA2)
[WiFiSetup] Network 2: MEGAFON_WIFI (RSSI: -88, Encryption: WPA2)
[WiFiSetup] Network 3: sd_wifi (RSSI: -55, Encryption: WPA2)
[WiFiSetup] Network 4: POCO X3 Pro (RSSI: -64, Encryption: WPA2)
[WiFiSetup] Network 5: DIRECT-Rv-Pantum P3010 Series (RSSI: -62, Encryption: WPA2)
[WiFiSetup] Network 6: 228 (RSSI: -63, Encryption: WPA/WPA2)
[WiFiSetup] Network 7: TP-LINK_0572 (RSSI: -88, Encryption: WPA2)
[WiFiSetup] Scan completed: 7 networks

--- консоль браузера ---
[WiFiSetup] Raw /api/config response length: 5681
(index):384 [WiFiSetup] Last 100 chars: nfig_initialized":true,"language":"ru","dark_theme":false,"large_font":true,"use_bold_values":false}
(index):433 [WiFiSetup] statusText: <div id=​"statusText" data-i18n=​"status_disconnected">​Disconnected​</div>​ statusDetails: <div class=​"status-details" id=​"statusDetails">​</div>​
(index):434 [WiFiSetup] boldValuesToggle.checked: false
(index):447 [WiFiSetup] bold-value removed from statusText
(index):451 [WiFiSetup] bold-value removed from statusDetails
(index):743 [WiFiSetup] Config loaded check: {hasPlaceholder: false, hasConfig: true, deviceName: 'Рабочая зона', result: 'Рабочая зона'}

## Hystory

### 2026-04-09 — Добавлена диагностика placeholder'ов в Refresh кнопку

**Проблема:** В STA режиме после загрузки страницы не обновляются PLACEHOLDER и не применяются UI настройки.

**Решение:** Добавлена проверка при нажатии кнопки [Refresh]:
- `isDeviceConfigLoaded()` — проверяет что `deviceName` НЕ содержит `DEVICE_NAME_PLACEHOLDER`
- Если placeholder обнаружен → выполняется `reinitDeviceConfig()` (повторный fetch /api/config + apply)
- Если после первого reinit всё еще не загружено → retry через 1 секунду

**Измененные файлы:**
- `lib/ETLTest/etl_wifi_setup_html.h`:
  - Добавлена функция `isDeviceConfigLoaded()` — проверяет наличие placeholder
  - Добавлена функция `reinitDeviceConfig()` — повторная инициализация config
  - Изменена `scanNetworks()` — в начале вызывает проверку и reinit при необходимости
  - Удалены дубликаты функций `scanNetworks()` и `renderNetworks()`

**Компиляция:**
- nodemcuv3: SUCCESS (RAM: 53.8%, Flash: 50.7%)
- esp32c3: SUCCESS (RAM: 12.6%, Flash: 55.9%)

**Тестирование:** ❌ Placeholder и настройки не применились, нажатие на [Refresh] ничего не изменило

### 2026-04-09 — Диагностика: битый JSON от /api/config

**Обнаружена новая проблема:** В консоли браузера ошибка:
```
Failed to load device config: SyntaxError: Unterminated string in JSON at position 5037 (line 1 column 5038)
```

**Диагноз:** Сервер отправляет **битый JSON** — незавершённая строка. Проблема НЕ в тайминге перезапуска сервера, а в сериализации JSON.

**Вероятная причина:** `m_device_info.icon_svg` — большая SVG строка. На ESP8266 с ограниченной памятью `serializeJson` может обрезать строку.

**Добавлена диагностика:**
- C++: `Serial.printf("[WiFiSetup] /api/config JSON size: %d bytes, free heap: %d\n", response.length(), ESP.getFreeHeap());`
- JS: Логирование длины ответа и последних 100 символов для выявления обрезки

**Следующий шаг:** Прошить устройство с новой диагностикой, посмотреть:
  1. Размер JSON в Serial логах
  2. Свободную кучу в момент отправки
  3. Последние 100 символов ответа в консоли браузера
  4. Сравнить с позицией ошибки (5037) — это покажет где именно обрезается

### 2026-04-09 — ✅ ИСПРАВЛЕНО: обрезка JSON на ESP8266

**Корень проблемы:** `serializeJson(doc, String)` на ESP8266 **обрезает большие JSON** (~5.7KB) потому что `String` не может аллоцировать непрерывный блок памяти при ~16KB free heap.

**До исправления:**
```
measureJson(doc) → 5697 bytes (ожидаемый)
response.length() → 4867 bytes (фактический — обрезан на ~830 байт!)
```

**Решение:** Отправлять JSON напрямую через `WiFiClient` без промежуточного `String`:
1. Формируем HTTP заголовки вручную (`HTTP/1.1 200 OK\r\n...`)
2. Отправляем заголовки через `m_server->client().write()`
3. Вызываем `serializeJson(doc, m_server->client())` — сериализация напрямую в `Print` интерфейс
4. Больше не требуется аллокация `String` → нет обрезки

**После исправления:**
```
measureJson(doc)   → 5697 bytes
JSON sent          → 5697 bytes ✅
response length    → 5681 bytes (браузер) ✅
hasPlaceholder: false, hasConfig: true, deviceName: 'Рабочая зона' ✅
```

**Измененные файлы:**
- `lib/ETLTest/etl_webui.cpp` — переписан `handle_api_config()` для прямой отправки в WiFiClient
- `lib/ETLTest/etl_wifi_setup_html.h` — добавлены функции диагностики (`isDeviceConfigLoaded`, `reinitDeviceConfig`) для fallback при загрузке config

**Компиляция:**
- nodemcuv3: SUCCESS (RAM: 54.3%, Flash: 51.1%)
- esp32c3: SUCCESS

**Тестирование:** ✅ ESP8266 STA режим — placeholder'ы заменены, UI настройки применены

