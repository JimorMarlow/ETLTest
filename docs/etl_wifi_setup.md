# ETL WiFi Setup Server

Библиотека для настройки WiFi подключения устройств на базе ESP8266/ESP32 через веб-интерфейс.

## Описание

**ETL WiFi Setup Server** — это готовое решение для первичной настройки WiFi подключения встраиваемых устройств. Библиотека предоставляет веб-интерфейс, доступный через точку доступа устройства, для выбора WiFi сети, ввода пароля и настройки параметров устройства.

### Возможности

- **Режим точки доступа (AP)** — устройство создает собственную WiFi сеть для настройки
- **Сканирование сетей** — отображение доступных WiFi сетей с уровнем сигнала
- **Подключение к WiFi** — подключение к выбранной сети с сохранением настроек
- **Сохранение настроек** — энергонезависимая память (LittleFS) для конфигурации
- **Сброс к заводским настройкам** — возможность полного сброса конфигурации
- **Веб-интерфейс** — адаптивный UI с поддержкой темной/светлой темы
- **Мультиязычность** — поддержка английского и русского языков
- **mDNS** — доступ по имени `hostname.local`
- **Гибкая настройка** — кастомизация информации об устройстве, иконки, параметров AP

### Платформы

- ✅ ESP8266 (NodeMCU v3, D1 Mini, etc.)
- ✅ ESP32 (ESP32 DevKit, ESP32-C3, etc.)

---

## Структура страницы

Веб-интерфейс имеет следующую структуру:

```
┌─────────────────────────────────────────────────────┐
│ Settings / Настройки                        [RU]    │ ← header (data-i18n="title")
├─────────────────────────────────────────────────────┤
│ [Device Icon]  Device Name                          │
│                Description                          │ ← device-info-container
├─────────────────────────────────────────────────────┤
│ Настройки интерфейса                                │ ← ui-settings-container
│ ┌─────────────────────────────────────────────────┐ │
│ │ Тёмная тема                             [toggle]│ │
│ │ Увеличенный шрифт                       [toggle]│ │
│ │ Ключевые значения (Bold)                [toggle]│ │
│ └─────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│ WiFi Setup / Настройки WiFi                         │ ← section-title (data-i18n="main_title")
├─────────────────────────────────────────────────────┤
│ [●] Статус подключения              [Refresh]       │ ← status-section
│     IP адрес                                        │
├─────────────────────────────────────────────────────┤
│ Select Network / Выберите сеть                      │ ← section-title
│ ┌─────────────────────────────────────────────────┐ │
│ │ 📶 Network Name           🔒                   │ │ ← networks-list
│ │     Excellent • WPA2                            │ │
│ └─────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│ Access Point Settings / Настройки точки доступа     │ ← ap-settings-section
│ ┌─────────────────────────────────────────────────┐ │
│ │ AP SSID:     [ESP_Device_AP]                    │ │
│ │ AP Password: [••••••••••]  [Show]               │ │
│ └─────────────────────────────────────────────────┘ │
│ [Apply AP Settings]                                 │
├─────────────────────────────────────────────────────┤
│ [Save & Reboot]                                     │
│ [Factory Reset]                                     │
└─────────────────────────────────────────────────────┘
```

### Элементы управления

| Элемент | Описание |
|---------|----------|
| **header** | Заголовок страницы с кнопкой переключения языка (EN/RU) |
| **device-info-container** | Информация об устройстве: иконка, название, описание |
| **ui-settings-container** | Настройки интерфейса: темная тема, шрифт, bold-значения |
| **section-title** | Заголовок раздела (стиль как у "Select Network") |
| **status-section** | Статус подключения с индикатором и кнопкой обновления |
| **networks-list** | Список доступных WiFi сетей с уровнем сигнала |
| **ap-settings-section** | Настройки точки доступа (SSID, пароль) |
| **btn** | Кнопки действий: Save & Reboot, Factory Reset |

### Особенности интерфейса

- **Темная/светлая тема** — переключение с сохранением в постоянной памяти
- **Увеличенный шрифт** — все текстовые элементы увеличиваются на 20%
- **Bold-значения** — ключевые значения (статус, IP) отображаются жирным
- **Переключение языков** — работает по кругу: EN → RU → EN
- **Адаптивный дизайн** — оптимизирован для мобильных устройств

---

## Использование

### Быстрый старт

Минимальный код для запуска WiFi Setup Server:

```cpp
#include <Arduino.h>
#include "etl_wifi_setup.h"
#include "etl/etl_littlefs.h"

etl::wifi::server_setup wifi_server;

void setup() {
    Serial.begin(115200);

    // Инициализация LittleFS через ETL обертку
    // Автоматически учитывает отличия ESP8266/ESP32,
    // выполняет форматирование при первом старте
    if (!etl::little_fs::begin()) {
        Serial.println("LittleFS mount failed!");
        return;
    }

    // Конфигурация WiFi по умолчанию
    etl::wifi::server_config_t wifi_config;
    wifi_config.set_hostname("mydevice");
    wifi_config.set_ap_ssid("MyDevice_AP");
    wifi_config.set_ap_password("12345678");

    // Инициализация настроек WiFi
    etl::wifi::settings::init_wifi_config(wifi_config);

    // Информация об устройстве
    etl::wifi::device_info_t device_info;
    device_info.name = "My Device v1.0";
    device_info.description = "Smart home controller";

    // Запуск сервера
    wifi_server.set_config(wifi_config);
    if (wifi_server.begin(device_info)) {
        Serial.println("WiFi Setup Server started");
        Serial.print("IP: ");
        Serial.println(wifi_server.get_ip_address());
    }
}

void loop() {
    // Обработка WiFi событий и HTTP запросов
    wifi_server.handle();
    wifi_server.handle_client();

    // Основная логика устройства
    // ...
}
```

### Доступ к веб-интерфейсу

После запуска устройство создает точку доступа:

- **SSID:** `ESP_Device_AP` (или заданный в `set_ap_ssid()`)
- **Пароль:** `password123` (или заданный в `set_ap_password()`)
- **IP адрес:** `192.168.4.1`
- **Веб-интерфейс:** http://192.168.4.1

После подключения к внешней WiFi сети:

- **mDNS:** http://`hostname`.local
- **IP адрес:** http://`WiFi.localIP()`

### Расширенная настройка

#### Настройка интерфейса

```cpp
// Инициализация настроек интерфейса
etl::wifi::ui_config_t ui_config;
ui_config.set_language("ru");      // Язык: "en" или "ru"
ui_config.set_dark_theme(true);    // Темная тема
ui_config.set_large_font(true);    // Увеличенный шрифт
ui_config.set_use_bold_values(true); // Bold для значений

etl::wifi::settings::init_ui_config(ui_config);
```

#### Кастомная иконка устройства

```cpp
etl::wifi::device_info_t device_info;
device_info.name = "Temperature Sensor";
device_info.description = "DS18B20 based sensor";
device_info.icon_svg = R"rawliteral(
<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>
  <circle cx='32' cy='32' r='24' fill='#3498db'/>
  <text x='32' y='40' text-anchor='middle' fill='white'>°C</text>
</svg>
)rawliteral";
```

#### Загрузка сохраненных настроек

```cpp
#include "etl/etl_littlefs.h"

void setup() {
    // Инициализация LittleFS через ETL обертку
    if (!etl::little_fs::begin()) {
        Serial.println("LittleFS failed!");
        return;
    }

    // Попытка загрузки сохраненных настроек
    if (wifi_server.load_settings()) {
        Serial.println("Loaded saved settings");
    }

    // Настройка конфигурации
    etl::wifi::server_config_t wifi_config;
    wifi_config.set_hostname("mydevice");
    wifi_server.set_config(wifi_config);

    // Запуск сервера
    wifi_server.begin(device_info);
}
```

#### Сброс настроек

```cpp
// Сброс WiFi настроек к заводским
etl::wifi::server_config_t default_config;
etl::wifi::settings::init_wifi_config(default_config, true); // true = сброс

// Сброс настроек интерфейса
etl::wifi::ui_config_t default_ui;
etl::wifi::settings::init_ui_config(default_ui, true);
```

---

## Архитектура

### Классы и структуры

#### `etl::wifi::server_setup`

Основной класс библиотеки. Управляет WiFi подключением, HTTP сервером и обработкой запросов.

**Основные методы:**

| Метод | Описание |
|-------|----------|
| `begin(device_info)` | Инициализация сервера с информацией об устройстве |
| `stop()` | Остановка сервера, освобождение ресурсов |
| `handle()` | Обработка WiFi событий (вызывать в `loop()`) |
| `handle_client()` | Обработка HTTP запросов (вызывать в `loop()`) |
| `get_connection_status()` | Получить статус подключения |
| `is_connected()` | Проверка подключения к WiFi |
| `get_ip_address()` | Получить IP адрес устройства |
| `get_mode()` | Режим работы: "AP", "STA", "AP+STA" |
| `scan_networks(results)` | Сканирование доступных сетей |
| `connect_to_network(ssid, password)` | Подключение к сети |
| `save_settings()` | Сохранение настроек в LittleFS |
| `load_settings()` | Загрузка настроек из LittleFS |
| `reset_settings()` | Сброс к заводским настройкам |
| `reboot()` | Перезагрузка устройства |

#### `etl::wifi::server_config_t`

Конфигурация WiFi сервера. Сохраняется в LittleFS.

**Поля:**
- `hostname` — имя устройства для mDNS
- `ap_ssid` — SSID точки доступа
- `ap_password` — пароль точки доступа
- `wifi_ssid` — SSID внешней сети (сохраняется после подключения)
- `wifi_password` — пароль внешней сети
- `port` — порт веб-сервера (по умолчанию 80)

#### `etl::wifi::ui_config_t`

Конфигурация интерфейса. Сохраняется в LittleFS.

**Поля:**
- `language` — язык интерфейса ("en", "ru")
- `dark_theme` — темная тема
- `large_font` — увеличенный шрифт
- `use_bold_values` — bold для ключевых значений

#### `etl::wifi::device_info_t`

Информация об устройстве. **Не сохраняется** в LittleFS, передается при запуске.

**Поля:**
- `name` — название устройства
- `description` — описание
- `icon_svg` — SVG иконка (опционально)

### HTTP API Endpoints

| Endpoint | Метод | Описание |
|----------|-------|----------|
| `/` | GET | Главная страница веб-интерфейса |
| `/api/scan` | GET | Сканирование WiFi сетей |
| `/api/connect` | POST | Подключение к сети |
| `/api/disconnect` | POST | Отключение от сети |
| `/api/status` | GET | Статус подключения |
| `/api/config` | GET | Конфигурация устройства |
| `/api/save` | POST | Сохранение настроек |
| `/api/reset` | POST | Сброс настроек |
| `/api/ui_settings` | POST | Настройки интерфейса |

### Обработчики событий

```
┌─────────────────────────────────────────────────────────┐
│                    loop()                               │
│  ┌─────────────────────┐  ┌─────────────────────────┐   │
│  │ wifi_server.handle()│  │ wifi_server.handle_client() │
│  └─────────┬───────────┘  └───────────┬─────────────┘   │
│            │                          │                 │
│            ▼                          ▼                 │
│  ┌─────────────────────┐  ┌─────────────────────────┐   │
│  │ Обновление статуса  │  │ Обработка HTTP запросов │   │
│  │ WiFi подключения    │  │ и роутинг по endpoints  │   │
│  └─────────────────────┘  └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

**Поток обработки:**

1. `handle()` — проверяет статус WiFi, управляет переключением режимов AP/STA
2. `handle_client()` — обрабатывает входящие HTTP запросы, вызывает соответствующие обработчики
3. Обработчики API — выполняют действия (сканирование, подключение, сохранение) и возвращают JSON ответы

---

## Рекомендации

### Цветовая схема

Для создания согласованного интерфейса используйте следующие цвета:

#### Светлая тема (Light Mode)

| Элемент | Цвет | Hex |
|---------|------|-----|
| Фон страницы | Background | `#FFFFFF` |
| Фон контейнеров | Container | `#F2F2F7` |
| Основной текст | Primary Text | `#1C1C1E` |
| Вторичный текст | Secondary Text | `#8E8E93` |
| Границы | Borders | `#C6C6C8` |
| Акцент (кнопки, ссылки) | Accent | `#007AFF` |
| Успех | Success | `#34C759` |
| Ошибка | Error | `#FF3B30` |
| Предупреждение | Warning | `#FF9500` |

#### Темная тема (Dark Mode)

| Элемент | Цвет | Hex |
|---------|------|-----|
| Фон страницы | Background | `#1C1C1E` |
| Фон контейнеров | Container | `#2C2C2E` |
| Основной текст | Primary Text | `#FFFFFF` |
| Вторичный текст | Secondary Text | `#98989D` |
| Границы | Borders | `#38383A` |
| Акцент (кнопки, ссылки) | Accent | `#0A84FF` |
| Успех | Success | `#30D158` |
| Ошибка | Error | `#FF453A` |
| Предупреждение | Warning | `#FF9F0A` |

### Размеры шрифтов

Библиотека использует базовый размер шрифта с масштабированием:

| Настройка | Базовый размер | Увеличенный (+20%) |
|-----------|----------------|---------------------|
| Заголовок страницы | 17px | 20px |
| Заголовок раздела | 17px | 20px |
| Название устройства | 20px | 24px |
| Описание устройства | 15px | 18px |
| Текст статуса | 15px | 18px |
| Детали статуса | 13px | 15px |
| Название сети | 15px | 18px |
| Сигнал сети | 13px | 15px |
| Кнопки | 15px (h: 44px) | 18px (h: 52px) |
| Поля ввода | 15px (h: 44px) | 18px (h: 52px) |
| Модальные окна | 17px | 20px |

### Шрифты

Рекомендуемый стек шрифтов (используется в библиотеке):

```css
font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 
             'Helvetica Neue', Arial, sans-serif;
```

### Отступы и размеры

| Элемент | Значение |
|---------|----------|
| Padding страницы | 16px (мобильные: 12px) |
| Max ширина контейнера | 480px |
| Border radius (контейнеры) | 10-12px |
| Border radius (кнопки) | 8-10px |
| Border radius (поля ввода) | 8px |
| Gap между элементами | 12-20px |
| Высота кнопок | 44px (large: 52px) |
| Высота полей ввода | 44px (large: 52px) |

### Иконка устройства

Рекомендации для SVG иконки:

- **Размер:** viewBox `0 0 64 64` или `0 0 512 512`
- **Цвета:** используйте акцентные цвета библиотеки (`#1d436d`, `#a2d6fd`)
- **Стиль:** минималистичный, плоский дизайн
- **Анимация:** допустима CSS анимация для индикаторов

Пример:
```cpp
device_info.icon_svg = R"rawliteral(
<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>
  <rect x='8' y='8' width='48' height='48' rx='8' fill='#1d436d'/>
  <circle cx='32' cy='32' r='16' fill='#a2d6fd'/>
</svg>
)rawliteral";
```

### Best Practices

1. **Инициализация LittleFS** — используйте `etl::little_fs::begin()` для инициализации файловой системы. Эта обертка автоматически:
   - Учитывает различия между ESP8266 и ESP32
   - Выполняет форматирование при первом запуске
   - Создает необходимые директории для настроек

   ```cpp
   #include "etl/etl_littlefs.h"

   if (!etl::little_fs::begin()) {
       Serial.println("LittleFS failed!");
       return;
   }
   ```

2. **Сохранение настроек** — вызывайте `save_settings()` перед перезагрузкой
3. **Обработка в loop()** — обязательно вызывайте `handle()` и `handle_client()`
4. **Таймаут подключения** — используйте разумные таймауты (5-10 секунд)
5. **Безопасность** — меняйте пароль AP по умолчанию на уникальный
6. **mDNS** — используйте уникальные hostname для каждого устройства

---

## Примеры

### Полное устройство с датчиком

```cpp
#include <Arduino.h>
#include "etl_wifi_setup.h"
#include "etl/etl_littlefs.h"

etl::wifi::server_setup wifi_server;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // LittleFS через ETL обертку
    // Автоматическое форматирование при первом старте
    if (!etl::little_fs::begin()) {
        Serial.println("LittleFS failed!");
        return;
    }

    // WiFi конфиг
    etl::wifi::server_config_t wifi_cfg;
    wifi_cfg.set_hostname("temp_sensor_01");
    wifi_cfg.set_ap_ssid("TempSensor_AP");
    wifi_cfg.set_ap_password("sensor123");
    etl::wifi::settings::init_wifi_config(wifi_cfg);

    // UI конфиг
    etl::wifi::ui_config_t ui_cfg;
    ui_cfg.set_language("ru");
    ui_cfg.set_dark_theme(false);
    etl::wifi::settings::init_ui_config(ui_cfg);

    // Информация об устройстве
    etl::wifi::device_info_t dev_info;
    dev_info.name = "Temperature Sensor v2.0";
    dev_info.description = "DS18B20 temperature monitoring";
    dev_info.icon_svg = R"rawliteral(
    <svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'>
      <circle cx='32' cy='32' r='28' fill='#e74c3c'/>
      <text x='32' y='38' text-anchor='middle' fill='white'
            font-size='20' font-weight='bold'>°C</text>
    </svg>
    )rawliteral";

    // Запуск
    wifi_server.set_config(wifi_cfg);
    if (wifi_server.begin(dev_info)) {
        Serial.println("=== WiFi Server Ready ===");
        Serial.print("Mode: "); Serial.println(wifi_server.get_mode());
        Serial.print("IP: "); Serial.println(wifi_server.get_ip_address());
        Serial.print("mDNS: http://");
        Serial.print(wifi_cfg.get_hostname());
        Serial.println(".local");
    }
}

void loop() {
    wifi_server.handle();
    wifi_server.handle_client();

    // Логика датчика температуры
    // float temp = readTemperature();
    // publishToMqtt(temp);

    delay(1000);
}
```

---

## Лицензия

Библиотека распространяется в составе ETL Library (https://github.com/JimorMarlow/ETL)

## Поддержка

- Платформы: ESP8266, ESP32
- Документация: `docs/etl_wifi_setup.md`
- История изменений: `docs/tasks/done/task_webui_darktheme_hystory.md`
