# История изменений WiFi Setup

## 25 марта 2026 — Исправление mDNS, Disconnect и спиннера

### Проблемы
1. **Спиннер на кнопке Join не отображался** — текст не менялся на спиннер
2. **Disconnect не сбрасывал настройки** — после отключения wifi_ssid и wifi_password не очищались
3. **mDNS не работал после подключения к STA** — http://espdevice.local не открывался

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup_html.h`

**1. Исправление стилей кнопки Join для спиннера (строка ~88)**
```cpp
// БЫЛО:
.inline-join-btn { ... white-space: nowrap; }

// СТАЛО:
.inline-join-btn { ... white-space: nowrap; display: flex; align-items: center; justify-content: center; }
```

**2. Исправление стилей спиннера (строка ~121)**
```cpp
// БЫЛО:
.spinner { ... margin-right: 8px; vertical-align: middle; }

// СТАЛО:
.spinner { ... }  // убраны margin-right и vertical-align для центрирования
```

**3. Обновление inlineDisconnectNetwork (строка ~373)**
```javascript
// БЫЛО: Просто меняли локальное состояние
network.connected = false;
isConnected = false;

// СТАЛО: Вызов API для сброса настроек на сервере
fetch('/api/disconnect', { method: 'POST' })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            // Сброс состояния
        }
    });
```

#### Файл: `lib/ETLTest/etl_wifi_setup.h`

**4. Добавлен обработчик handle_api_disconnect (строка ~393)**
```cpp
/**
 * @brief Обработчик API отключения
 */
virtual void handle_api_disconnect();
```

#### Файл: `lib/ETLTest/etl_wifi_setup.cpp`

**5. Реализация handle_api_disconnect (строка ~853)**
```cpp
void server_setup::handle_api_disconnect()
{
    // Сброс настроек WiFi
    m_config.set_wifi_ssid("");
    m_config.set_wifi_password("");

    // Отключение от сети
    WiFi.disconnect(true);
    m_connection_status = connection_status_t::disconnected;

    // Возврат в режим AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(...);

    // Отправка ответа
    send_success_response("Disconnected");
}
```

**6. Добавлен маршрут /api/disconnect (строка ~1043)**
```cpp
m_server->on("/api/disconnect", HTTP_POST, [this]() {
    handle_api_disconnect();
});
```

**7. Исправление mDNS — не перезапускать begin() повторно (строка ~291)**
```cpp
// mDNS - инициализация только если ещё не инициализирован
static bool mdns_initialized = false;

if (!mdns_initialized) {
    if (MDNS.begin(m_config.get_hostname().c_str())) {
        mdns_initialized = true;
    }
} else {
    // mDNS уже инициализирован
}

// Добавляем сервис http и обновляем
MDNS.addService("http", "tcp", m_config.port);
MDNS.update();
```

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.2% Flash)
- ✅ esp32c3 — SUCCESS (4.3% RAM, 18.6% Flash)
- ✅ esp32-wroom-32u — SUCCESS (6.6% RAM, 20.2% Flash)

---

## 25 марта 2026 — Исправление проблем с подключением

### Проблемы
1. **Нет индикатора подключения на кнопке Join** — спиннер не появлялся
2. **Сервер переставал отвечать после подключения** — клиент терял соединение
3. **Ошибка "Connection failed" при успешном подключении** — ответ не доходил до клиента

### Корневые причины
1. При переключении в режим `WIFI_STA` точка доступа отключалась, клиент терял соединение
2. HTTP сервер не обрабатывал запросы во время ожидания подключения
3. Ответ отправлялся до завершения подключения, но соединение обрывалось

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup.cpp`

**1. Изменение режима WiFi при подключении (строка ~775)**
```cpp
// БЫЛО:
WiFi.mode(WIFI_STA);

// СТАЛО:
WiFi.mode(WIFI_AP_STA);
// Это позволяет точке доступа продолжать работу во время подключения к STA
```

**2. Обработка HTTP запросов во время ожидания (строка ~785)**
```cpp
while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout) {
    delay(500);
    yield();
    // Обработка HTTP запросов во время ожидания
    if (m_server) {
        m_server->handleClient();
    }
    Serial.print(F("."));
}
```

**3. Отправка ответа после подключения (строка ~795)**
```cpp
// Ответ отправляется ПОСЛЕ успешного подключения
if (WiFi.status() == WL_CONNECTED) {
    // ... отправка успешного ответа с IP
    
    // Возврат в предыдущий режим
    if (previous_mode == WIFI_AP) {
        WiFi.mode(WIFI_AP_STA);
    }
} else {
    // Отправка ответа с ошибкой
    // ...
    
    // Восстановление AP при ошибке
    if (previous_mode == WIFI_AP || previous_mode == WIFI_AP_STA) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(...);
    }
}
```

**4. Увеличение задержки перед перезапуском HTTP сервера (строка ~352)**
```cpp
// БЫЛО: 2000 мс
// СТАЛО: 5000 мс
if (millis() - connection_time > 5000) {
    Serial.println(F("[WiFiSetup] Restarting HTTP server after STA connection..."));
    // ...
}
```

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.1% Flash)
- ✅ esp32c3 — SUCCESS (4.3% RAM, 18.6% Flash)
- ✅ esp32-wroom-32u — SUCCESS (6.6% RAM, 20.2% Flash)

### Текст для коммита
```
fix: WiFi setup connection handling and client response

- Use WIFI_AP_STA mode during connection to keep AP running
- Handle HTTP client requests while waiting for STA connection
- Send response after connection completes (success or error)
- Increase HTTP server restart delay to 5 seconds
- Properly restore AP mode on connection failure

Fixes issue where client loses connection during WiFi setup
and doesn't receive connection status response.
```

---

## 25 марта 2026 — Добавление SVG иконки для отладки

### Изменения
Добавлена сжатая SVG иконка из `docs\web-wifi\icon_led_test.svg` в `src\main.cpp`

#### Файл: `src/main.cpp` (строки 37-41)
```cpp
// Опционально: кастомная SVG иконка
if(simulation_data.custom_icon_svg)
{
    device_info.icon_svg = F("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'><style>.led{animation:blink 1.5s infinite}@keyframes blink{0%,100%{opacity:1}50%{opacity:0.5}}</style><path d='M8 32c0-8 6-14 14-14s14 6 14 14-6 14-14 14-14-6-14-14zm8 0c0 3 3 6 6 6s6-3 6-6-3-6-6-6-6 3-6 6z' fill='#1d436d'/><path d='M28 32c0-8 6-14 14-14s14 6 14 14-6 14-14 14-14-6-14-14zm8 0c0 3 3 6 6 6s6-3 6-6-3-6-6-6-6 3-6 6z' fill='#1d436d'/><path d='M48 32c0-8 6-14 14-14v28c-8 0-14-6-14-14z' fill='#1d436d'/><circle class='led' cx='22' cy='32' r='3' fill='#a2d6fd'/><circle class='led' cx='42' cy='32' r='3' fill='#a2d6fd'/><circle class='led' cx='62' cy='32' r='3' fill='#a2d6fd'/></svg>");
}
```

**Оптимизация:**
- Удалены пробелы и переносы строк
- Двойные кавычки заменены на одинарные
- Использован макрос `F()` для размещения во flash-памяти
- Размер: ~750 байт → ~520 байт

---

## TODO

### Нерешённые проблемы WiFi Setup
1. **Спиннер на кнопке Join** — требуется проверка JavaScript кода
2. **Обработка Retry после ошибки** — сервер может не отвечать
3. **mDNS после перезапуска сервера** — может требовать дополнительного обновления

### Планируемые улучшения
- [ ] Добавить логирование состояния WiFi.mode() для отладки
- [ ] Проверить работу status polling через /api/status
- [ ] Добавить таймаут для handleClient() во время ожидания подключения
