# История разработки: WiFi Setup Refactor

## Задача
Рефакторинг кода `lib\ETLTest\etl_wifi_setup*` для улучшения архитектуры и подготовки к расширению функциональности.

## Цель
- Разделить настройки WiFi и настройки интерфейса на независимые структуры
- Сделать настройки опциональными (пользователь выбирает какие настройки нужны)
- Заложить основу для будущих расширений (Telegram Bot, MQTT)
- Подготовить код для включения в библиотеку ETL

## Выполненные изменения

### Этап 1: Разделение настроек WiFi и UI

#### 1.1 Создана новая структура `ui_config_t`
**Файлы:** `lib\ETLTest\etl_wifi_setup.h`, `lib\ETLTest\etl_wifi_setup.cpp`

Вынесены поля настроек интерфейса из `server_config_t`:
```cpp
struct ui_config_t
{
    char language[WIFI_CONFIG_LANGUAGE_SIZE] = "en";  // Язык интерфейса (ISO 639-1)
    bool dark_theme = false;                    // Тёмная тема
    bool large_font = false;                    // Увеличенный шрифт
    bool use_bold_values = false;               // Bold шрифт для ключевых значений
    
    void clear();
    void trace() const;
    // Setters и Getters
};
```

#### 1.2 Обновлена структура `server_config_t`
Удалены поля настроек интерфейса. Оставлены только WiFi настройки:
```cpp
struct server_config_t
{
    // Конфигурация сети
    char hostname[WIFI_CONFIG_HOSTNAME_SIZE] = "espdevice";
    char ap_ssid[WIFI_CONFIG_SSID_SIZE] = "ESP_Device_AP";
    char ap_password[WIFI_CONFIG_PASSWORD_SIZE] = "password123";
    char wifi_ssid[WIFI_CONFIG_SSID_SIZE] = "";
    char wifi_password[WIFI_CONFIG_PASSWORD_SIZE] = "";
    uint16_t port = 80;
    uint32_t update_interval = 500;
    // Поля настроек интерфейса удалены - перенесены в ui_config_t
};
```

#### 1.3 Переименованы функции работы с WiFi настройками
- `init_config()` → `init_wifi_config()`
- `save_config()` → `save_wifi_config()`
- `load_config()` → `load_wifi_config()`

#### 1.4 Добавлены функции для UI настроек
- `init_ui_config()`
- `save_ui_config()`
- `load_ui_config()`

#### 1.5 Изменен тип возвращаемого значения `load_wifi_config()`
```cpp
etl::optional<server_config_t> load_wifi_config();
```
Возвращает `etl::optional` с конфигом, если он был инициализирован, или пустой `optional`, если нет.

#### 1.6 Обновлен класс `server_setup`
- Изменен тип `m_config` с `server_config_t` на `etl::optional<server_config_t>`
- Добавлен `m_ui_config` типа `etl::optional<ui_config_t>`
- Обновлена логика работы с настройками
- Конструктор теперь принимает `etl::optional<server_config_t>& cfg = {}`

### Этап 2: Подготовка к расширению

#### 2.1 Добавлены скелеты для будущих настроек
- `telegram_config_t` - для Telegram Bot интеграции
- `mqtt_config_t` - для MQTT сервера

Для каждой конфигурации добавлены функции:
- `init_telegram_config()`, `save_telegram_config()`, `load_telegram_config()`
- `init_mqtt_config()`, `save_mqtt_config()`, `load_mqtt_config()`

### Этап 3: Обновление main.cpp

#### 3.1 Обновлен вызов инициализации
```cpp
// Было:
etl::wifi::settings::init_config(web_config, ...);

// Стало:
etl::wifi::settings::init_wifi_config(web_config, ...);
```

#### 3.2 Обновлено получение конфигурации
```cpp
// Было:
const String& hostname_cfg = wifi_server->get_config().get_hostname();

// Стало:
const String& hostname_cfg = wifi_server->get_wifi_config().has_value() 
    ? wifi_server->get_wifi_config()->get_hostname() 
    : "espdevice";
```

## Тестирование
- ✅ Компиляция для nodemcuv3 (ESP8266) - SUCCESS
- ✅ Компиляция для d1_mini_lite (ESP8266) - SUCCESS
- ✅ Компиляция для esp32-wroom-32u (ESP32) - SUCCESS
- ✅ Компиляция для esp32c3 (ESP32-C3) - SUCCESS

## Структура коммита
```
refactor: WiFi Setup - разделение настроек WiFi и UI

- Вынесены настройки интерфейса в ui_config_t
- server_config_t содержит только WiFi настройки
- Добавлены функции init_ui_config, save_ui_config, load_ui_config
- Переименованы функции: init_config -> init_wifi_config, и т.д.
- load_wifi_config возвращает etl::optional<server_config_t>
- Добавлены скелеты telegram_config_t и mqtt_config_t
- server_setup использует etl::optional для m_config и m_ui_config
- Код готов для включения в библиотеку ETL
- Компиляция: nodemcuv3 SUCCESS, d1_mini_lite SUCCESS, 
  esp32-wroom-32u SUCCESS, esp32c3 SUCCESS
```

## Следующие шаги
- [x] Обновить веб-интерфейс для работы с опциональными настройками (показывать ui-settings-container только если настройки инициализированы)
- [ ] Протестировать на реальных устройствах
- [ ] Обновить документацию

## Исправления (после первоначальной реализации)

### Исправление: Скрытие контейнера UI настроек при неинициализированных настройках

**Проблема:** При `init_ui_settings = false` в симуляции, контейнер настроек интерфейса всё равно показывался.

**Причина:** У контейнера `.ui-settings-container` не было `id`, поэтому переменная `uiSettingsContainer` в JavaScript не была инициализирована.

**Решение:**
1. В `handle_api_config()` добавлен флаг `ui_config_initialized`, который передаётся клиенту
2. В HTML добавлен `id="uiSettingsContainer"` к контейнеру
3. В JavaScript добавлено объявление переменной `uiSettingsContainer`
4. В JavaScript функции `loadDeviceConfig()` сохранение флага в `window.deviceConfig.ui_config_initialized`
5. В функции `applyUISettings()` проверка флага и скрытие контейнера через `classList.add('hidden')`

**Изменённые файлы:**
- `lib/ETLTest/etl_wifi_setup.cpp` — добавлена передача флага `ui_config_initialized`
- `lib/ETLTest/etl_wifi_setup_html.h` — добавлен `id="uiSettingsContainer"`, обновлены функции `loadDeviceConfig()` и `applyUISettings()`
