# Errors in task_webui.md — IN PROGRESS

## Сервер контента

После добавления сервера контента и менеджера конфигурация на esp32 работает, а на ESP8266 происходит падение (OOM) после старта.

**Статус: ЧАСТИЧНО ИСПРАВЛЕНО** ⚠️ — компиляция работает, нужно тестирование на устройстве

## Текущее состояние

Сборка **успешна** для обеих платформ:

| Платформа | RAM | Flash | Статус |
|-----------|-----|-------|--------|
| ESP8266 (d1_mini_lite) | 78.7% (64448/81920) | 48.2% (502952/1044464) | ✅ SUCCESS |
| ESP32-C3 | 12.6% (41140/327680) | 56.1% (1065492/1900544) | ✅ SUCCESS |

## Что сделано

### 1. MinimalHttpServer — новый ультра-лёгкий HTTP-сервер для ESP8266
**Файл:** `src/minimal_http_server.h`

Размер класса ~35 байт (без маршрутов, без `std::function`, без heap-аллокаций):
- ОДИН `DispatchFn` callback для ВСЕХ запросов (вместо 9+ отдельных handler'ов)
- Все буферы запросов — НА СТЕКЕ в `handleClient()`: `uri[64]`, `body[512]`
- НЕТ массивов маршрутов внутри класса
- Совместимый API: `send()`, `send_P()`, `hasArg()`, `arg()`, `method()`, `uri()`, `reply()`

### 2. `m_http_server` — член класса, НЕ heap-аллокация
**Файлы:** `src/light_webui.h`, `src/light_webui.cpp`

- `MinimalHttpServer m_http_server` объявлен как **член класса** `light_control_server`
- НЕ аллоцируется через `etl::make_shared` — размещён в том же блоке heap что и `light_control_server`
- `begin()` переопределён для ESP8266 — НЕ вызывает `web_server_base_t::begin()`, НЕ создаёт `m_server` через `shared_ptr`
- `handle_client()` переопределён — вызывает `m_http_server.handleClient()`

### 3. `begin()` виртуальная в базовом классе
**Файл:** `lib/ETLTest/etl_webui_base.h`

Сделана `virtual bool begin()` чтобы `light_control_server::begin()` для ESP8266 мог переопределить и обойти создание `m_server` через `shared_ptr`.

### 4. Единый dispatch callback для ESP8266
**Файлы:** `src/light_webui.cpp`, `lib/ETLTest/etl_webui.cpp`

- `_cb_dispatch()` для light_control_server — парсит URL через `strcmp()`, вызывает нужный handler
- `_ss_cb_dispatch()` для server_setup — аналогично
- `friend` объявления для доступа к protected методам

### 5. Удалены неиспользуемые API из light_control_server
Удалены методы которые нужны только в server_setup:
- `handle_api_scan()`, `handle_api_connect()`, `handle_api_disconnect()`
- `handle_api_save()`, `handle_api_reset()`, `handle_api_ap_settings()`

### 6. Отключение mDNS на ESP8266
- mDNS не инициализируется на ESP8266
- `MDNS.update()` убран из `handle_client()`

### 7. Клиентский debounce 100ms
**Файл:** `src/light_webui_html.h`

`sendState()` с `setTimeout` — не шлёт запросы чаще раза в 100ms.

### 8. Серверный debounce 50ms + StaticJsonDocument
**Файл:** `src/light_webui.cpp`

`handle_api_control()` — не чаще 50ms, `StaticJsonDocument<256>` без аллокаций.

### 9. Логирование heap для диагностики
**Файлы:** `src/light_webui.cpp`, `lib/ETLTest/etl_webui_base.cpp`

Serial-лог `ESP.getFreeHeap()` после каждого шага инициализации.

## Что нужно проверить при тестировании

### 1. Запуск сервера
Проверить serial log:
```
[LightControl] Init (ESP8266, no shared_ptr)...
[LightControl] Free heap before: <число>   ← должно быть > 1500
[LightControl] Settings loaded
[LightControl] AP started
[LightControl] Free heap after AP: <число> ← должно быть > 1500
[LightControl] Free heap after server: <число> ← должно быть > 500 (минимум!)
[LightControl] Server started (ESP8266)
```

**КРИТИЧНО:** Если после `Free heap after server` остаётся < 500 байт — сервер может падать при обработке HTTP-запросов.

### 2. Обработка HTTP-запросов
- Подключиться к AP `ESP_Device_AP`
- Открыть `http://192.168.4.1` в браузере
- Страница должна загрузиться (HTML)
- JS должен загрузить `/api/state`, `/api/device_info`, `/api/ui_config`

### 3. Нагрузка — ползунок яркости
- Активно подвигать ползунок brightness вперёд-назад
- НЕ должно быть Exception 29 (LoadProhibited) или OOM
- В serial log должно быть стабильное значение heap

### 4. server_setup
- Нажать Settings → должен переключиться на server_setup
- server_setup тоже использует MinimalHttpServer на ESP8266
- Проверить что WiFi scan работает

## Известные проблемы

- **~1600 байт heap до сервера, ~192 байт после WiFiServer::begin()** — `WiFiServer` внутри аллоцирует буферы сокетов. Это неизбежно на ESP8266. 192 байт может не хватить для обработки HTTP-запросов.
- Если heap после `m_http_server.begin()` < 500 байт — нужно искать другие пути:
  - Уменьшить размер HTML (убрать SVG дубликаты)
  - Использовать другую плату с большим RAM (NodeMCU, ESP32)

## Изменённые файлы

- `src/minimal_http_server.h` — **НОВЫЙ** минимальный HTTP-сервер (~35 байт)
- `src/light_webui.cpp` — `begin()`, `handle_client()`, `_cb_dispatch()` для ESP8266
- `src/light_webui.h` — `m_http_server` член, `begin()` override, `start_http_server()` stub/decl
- `src/light_webui_html.h` — клиентский debounce 100ms
- `lib/ETLTest/etl_webui.cpp` — `_ss_cb_dispatch()` для server_setup
- `lib/ETLTest/etl_webui.h` — `friend` объявление, `_ss_cb_dispatch()` declaration
- `lib/ETLTest/etl_webui_base.h` — `begin()` virtual, `etl_web_server_t` = `MinimalHttpServer`
- `lib/ETLTest/etl_webui_base.cpp` — логирование heap, убран MDNS.update()

## Версия

v0.3.1 — оптимизация HTTP сервера для ESP8266
