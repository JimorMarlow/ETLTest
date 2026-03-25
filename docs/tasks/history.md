# История изменений WiFi Setup

## 25 марта 2026 — Исправление видимости спиннера на кнопке Join

### Проблема
Спиннер не отображался на кнопке Join во время подключения, потому что:
1. `opacity: 0.5` для disabled кнопки делал спиннер почти невидимым
2. Не было класса для состояния "подключение"

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup_html.h`

**1. Изменены стили кнопки Join (строка ~89)**
```css
/* БЫЛО: */
.inline-join-btn:disabled { opacity: 0.5; cursor: not-allowed; }

/* СТАЛО: */
.inline-join-btn:disabled { opacity: 0.7; cursor: not-allowed; }
.inline-join-btn.connecting { opacity: 1; cursor: wait; }
```

**2. Добавлен класс `connecting` при показе спиннера (строка ~444)**
```javascript
// БЫЛО:
joinBtn.classList.add('btn-with-spinner');

/* СТАЛО: */
joinBtn.classList.add('btn-with-spinner', 'connecting');
```

**3. Удаление класса `connecting` при восстановлении кнопки (строки ~489, ~523)**
```javascript
// БЫЛО:
joinBtn.classList.remove('btn-with-spinner');

/* СТАЛО: */
joinBtn.classList.remove('btn-with-spinner', 'connecting');
```

**4. Улучшено логирование (строка ~448)**
```javascript
console.log('[WiFiSetup] Join button set to spinner, disabled=', joinBtn.disabled, 'classList=', joinBtn.classList);
```

### Как это работает
1. При нажатии Join кнопка получает класс `connecting`
2. Класс `connecting` устанавливает `opacity: 1`, делая спиннер видимым
3. Курсор меняется на `wait`
4. После подключения или ошибки класс удаляется

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.5% Flash)

### Проверка
1. Откройте консоль браузера (F12)
2. Нажмите кнопку Join
3. Проверьте сообщение:
   ```
   [WiFiSetup] Join button set to spinner, disabled= true classList= btn-with-spinner connecting
   ```
4. Спиннер должен быть виден (белый, вращающийся)

---

## 25 марта 2026 — Диагностика сети и mDNS

### Проблема
ESP получает адрес **192.168.88.x** от точки доступа sd_wifi, в то время как локальная сеть пользователя использует **10.x.x.x**. Это означает, что sd_wifi раздаёт адреса из другой подсети.

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup.cpp`

**1. Добавлена подробная информация о сети при подключении (строка ~816)**
```cpp
if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n[WiFiSetup] Connected successfully"));
    Serial.print(F("[WiFiSetup] IP address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("[WiFiSetup] Subnet Mask: "));
    Serial.println(WiFi.subnetMask());
    Serial.print(F("[WiFiSetup] Gateway IP: "));
    Serial.println(WiFi.gatewayIP());
    Serial.print(F("[WiFiSetup] DNS IP: "));
    Serial.println(WiFi.dnsIP());
    // ...
}
```

### Диагностика

**Что проверить:**
1. Откройте Serial Monitor после подключения к sd_wifi
2. Посмотрите логи:
   ```
   [WiFiSetup] Connected successfully
   [WiFiSetup] IP address: 192.168.88.93
   [WiFiSetup] Subnet Mask: 255.255.255.0
   [WiFiSetup] Gateway IP: 192.168.88.1
   [WiFiSetup] DNS IP: 192.168.88.1
   ```

**Вывод:**
- Если ESP получает адрес 192.168.88.x, значит **sd_wifi раздаёт адреса этой подсети**
- Если ваш десктоп в сети 10.x.x.x — они в разных подсетях
- mDNS не работает между разными подсетями

**Решение:**
1. **Подключите десктоп к той же Wi-Fi сети sd_wifi** — тогда он получит адрес 192.168.88.x и mDNS заработает
2. **Используйте IP адрес напрямую** — http://192.168.88.93/
3. **Настройте маршрутизацию** между подсетями на роутере

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.5% Flash)

---

## 25 марта 2026 — Добавление отладки спиннера и пояснение mDNS

### Проблемы
1. **Спиннер на кнопке Join не отображался** — нужна отладка для понимания проблемы
2. **mDNS не виден в других подсетях** — это ограничение протокола mDNS

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup_html.h`

**1. Добавлены console.log для отладки спиннера (строка ~448)**
```javascript
// Блокировка UI и показ спиннера
joinBtn.disabled = true;
joinBtn.innerHTML = '<span class="spinner"></span>';
joinBtn.classList.add('btn-with-spinner');
// ...
console.log('[WiFiSetup] Join button set to spinner, disabled=', joinBtn.disabled);

try {
    console.log('[WiFiSetup] Starting fetch to /api/connect...');
    const response = await fetch('/api/connect', {...});
    console.log('[WiFiSetup] Response received:', response.status);
```

### Пояснения

**Спиннер:**
- Добавлены логи для отслеживания состояния кнопки
- Спиннер должен показываться сразу после нажатия Join
- Проверьте консоль браузера (F12) на наличие сообщений `[WiFiSetup]`

**mDNS:**
- mDNS работает только в пределах одной подсети (broadcast domain)
- Если ESP получает адрес 192.168.88.x, а компьютер в сети 192.168.0.x или 10.x.x.x — mDNS не будет виден
- Это нормальное поведение протокола mDNS/Bonjour
- Для доступа из других сетей:
  - Используйте IP адрес напрямую (например, http://192.168.88.93/)
  - Настройте mDNS forwarding на маршрутизаторе
  - Подключайте устройства к одной подсети

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.4% Flash)

---

## 25 марта 2026 — Исправление спиннера, mDNS и статуса подключения

### Проблемы
1. **Спиннер на кнопке Join не отображался** — перерисовка происходила слишком быстро
2. **mDNS не работал после перезагрузки в режиме STA** — http://espdevice.local не открывался в локальной сети
3. **Статус контейнер не обновлялся после перезагрузки** — показывал "Disconnected" вместо "Connected"

### Внесённые изменения

#### Файл: `lib/ETLTest/etl_wifi_setup_html.h`

**1. Добавлена задержка перед перерисовкой после подключения (строка ~448)**
```javascript
// БЫЛО: Сразу вызывали renderNetworks()
renderNetworks();

// СТАЛО: Задержка 500мс, чтобы пользователь увидел спиннер
await new Promise(resolve => setTimeout(resolve, 500));
renderNetworks();
```

**2. Добавлена функция checkConnectionStatus() (строка ~258)**
```javascript
async function checkConnectionStatus() {
    try {
        const response = await fetch('/api/status');
        const status = await response.json();
        if (status.connected) {
            isConnected = true;
            setStatus('connected', `${status.ssid} • ${status.ip}`);
            // Найти сеть в списке и отметить как подключенную
            const networkIndex = networks.findIndex(n => n.ssid === status.ssid);
            if (networkIndex >= 0) {
                networks[networkIndex].connected = true;
                selectedNetwork = networkIndex;
            }
        }
    } catch (error) {
        console.log('[WiFiSetup] Status check failed (may be in AP mode):', error.message);
    }
}
```

**3. Вызов checkConnectionStatus() в init() (строка ~255)**
```javascript
async function init() {
    // ...
    // Проверка статуса подключения при загрузке страницы
    await checkConnectionStatus();
    // ...
}
```

#### Файл: `lib/ETLTest/etl_wifi_setup.cpp`

**4. Улучшено логирование mDNS (строка ~295)**
```cpp
// Добавлено подробное логирование инициализации mDNS
if (!mdns_initialized) {
    Serial.print(F("[WiFiSetup] Initializing mDNS: "));
    if (MDNS.begin(m_config.get_hostname().c_str())) {
        // ...
        mdns_initialized = true;
    }
} else {
    Serial.print(F("[WiFiSetup] mDNS already running: http://"));
    // ...
}

// Добавлен явный вызов MDNS.update() с логом
MDNS.addService("http", "tcp", m_config.port);
MDNS.update();
Serial.println(F("[WiFiSetup] mDNS service added and updated"));
```

### Тесты компиляции
- ✅ nodemcuv3 — SUCCESS (46.5% RAM, 44.4% Flash)
- ✅ esp32c3 — SUCCESS (4.3% RAM, 18.6% Flash)
- ✅ esp32-wroom-32u — SUCCESS (6.6% RAM, 20.2% Flash)

---

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
