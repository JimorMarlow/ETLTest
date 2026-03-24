# Задача: Рефакторинг etl_wifi_setup

## Цель
Изменить структуры данных и API библиотеки `etl_wifi_setup` для корректной работы с библиотекой `gyverlibs/FileData`.

## Контекст
Текущая реализация использует `String` поля в структуре `server_config_t`, что приводит к проблемам при бинарном сохранении через `FileData`. Требуется перейти на фиксированные массивы символов.

---

## Требования

### 1. Разделение структур данных

**Файлы:** `lib/ETLTest/etl_wifi_setup.h`, `lib/ETLTest/etl_wifi_setup.cpp`

#### 1.1. Выделить `device_info_t` на уровень `server_config_t`

**Было:**
```cpp
struct server_config_t
{
    String hostname = "espdevice";
    String ap_ssid = "ESP_Device_AP";
    String ap_password = "password123";
    String wifi_ssid = "";
    String wifi_password = "";
    uint16_t port = 80;
    uint32_t update_interval = 500;

    struct device_info_t {
        String name = "ESP Device v1.0.0";
        String description = "Smart home device based on ESP8266/ESP32";
        String icon_svg = "";
        void clear();
        device_info_t& operator=(const device_info_t& other);
    } device;
};
```

**Стало:**
```cpp
// Отдельная структура информации об устройстве (НЕ сохраняется в FS, остаётся со String)
struct device_info_t
{
    String name = "ESP Device v1.0.0";
    String description = "Smart home device based on ESP8266/ESP32";
    String icon_svg = "";  // SVG может быть большим, размер неизвестен заранее
    
    void clear();
    device_info_t& operator=(const device_info_t& other);
};

// Конфигурация WiFi (сохраняется в FS, использует фиксированные массивы)
struct server_config_t
{
    // Только WiFi настройки
    // ... поля с фиксированными массивами ...
};
```

#### 1.2. Изменить сигнатуру конструктора и метода запуска

**Было:**
```cpp
class server_setup
{
    explicit server_setup(const server_config_t& cfg = server_config_t());
    virtual bool begin();
};
```

**Стало:**
```cpp
class server_setup
{
    explicit server_setup(const server_config_t& cfg = server_config_t());
    virtual bool begin(const device_info_t& device_info);
};
```

---

### 2. Замена String на фиксированные массивы

#### 2.1. Определить константы размеров

**Файл:** `lib/ETLTest/etl_wifi_setup.h`

В начале файла (перед структурами) добавить define'ы:

```cpp
// Размеры буферов для строк в server_config_t
#define WIFI_CONFIG_HOSTNAME_SIZE     32
#define WIFI_CONFIG_SSID_SIZE         32
#define WIFI_CONFIG_PASSWORD_SIZE     64
#define WIFI_CONFIG_ICON_SIZE         2048  // Для SVG иконки
```

#### 2.2. Изменить структуру `server_config_t`

**Требования:**
- Все строковые поля заменить на `char` массивы фиксированного размера
- Инициализация массивов нулями (пустые строки)
- Никаких полей переменной длины или сложных конструкторов

**Пример:**
```cpp
struct server_config_t
{
    // Конфигурация сети
    char hostname[WIFI_CONFIG_HOSTNAME_SIZE] = "";
    char ap_ssid[WIFI_CONFIG_SSID_SIZE] = "ESP_Device_AP";
    char ap_password[WIFI_CONFIG_PASSWORD_SIZE] = "password123";
    char wifi_ssid[WIFI_CONFIG_SSID_SIZE] = "";
    char wifi_password[WIFI_CONFIG_PASSWORD_SIZE] = "";
    uint16_t port = 80;
    uint32_t update_interval = 500;

    void trace() const;
    void clear();  // Новый метод для сброса к значениям по умолчанию
};
```

#### 2.3. Изменить структуру `device_info_t`

**Важно:** `device_info_t` **НЕ изменяется** — остаётся со `String` полями, т.к.:
- Не сохраняется в FS (передаётся отдельно при запуске)
- SVG иконка может быть произвольного размера

```cpp
struct device_info_t
{
    String name = "ESP Device v1.0.0";
    String description = "Smart home device based on ESP8266/ESP32";
    String icon_svg = "";

    void clear();
    device_info_t& operator=(const device_info_t& other);
};
```

---

### 3. Реализация Set/Get методов

#### 3.1. Добавить методы для `server_config_t`

**Файл:** `lib/ETLTest/etl_wifi_setup.h`

```cpp
struct server_config_t
{
    // ... поля ...

    // Setters
    void set_hostname(const String& value);
    void set_ap_ssid(const String& value);
    void set_ap_password(const String& value);
    void set_wifi_ssid(const String& value);
    void set_wifi_password(const String& value);

    // Getters
    String get_hostname() const;
    String get_ap_ssid() const;
    String get_ap_password() const;
    String get_wifi_ssid() const;
    String get_wifi_password() const;
};
```

#### 3.2. Методы для `device_info_t`

Не требуются — структура уже использует `String` и имеет прямые поля для доступа.

#### 3.3. Требования к реализации setters

**Файл:** `lib/ETLTest/etl_wifi_setup.cpp`

Для каждого setter'а:
1. Очистить буфер нулями (`memset`)
2. Скопировать строку с контролем длины (`strncpy` или `snprintf`)
3. Гарантировать завершающий `\0`

**Пример:**
```cpp
void server_config_t::set_hostname(const String& value)
{
    memset(hostname, 0, WIFI_CONFIG_HOSTNAME_SIZE);
    strncpy(hostname, value.c_str(), WIFI_CONFIG_HOSTNAME_SIZE - 1);
    hostname[WIFI_CONFIG_HOSTNAME_SIZE - 1] = '\0';  // Гарантировать \0
}
```

---

### 4. Обновление HTML макета

**Файл:** `lib/ETLTest/etl_wifi_setup_html.h`

#### 4.1. Изменить JavaScript объект `translations`

Убрать поля, которые больше не передаются через API:
- `device_name` → передаётся отдельно через `device_info_t`
- `device_description` → передаётся отдельно через `device_info_t`

#### 4.2. Изменить обработчик `/api/config`

**Файл:** `lib/ETLTest/etl_wifi_setup.cpp`

Обновить метод `handle_api_config()`:
- Устройство получает `device_info_t` как отдельный параметр
- Конфигурация содержит только WiFi настройки

---

### 5. Обновление settings namespace

**Файл:** `lib/ETLTest/etl_wifi_setup.cpp`

#### 5.1. Изменить типы данных

```cpp
namespace settings
{
    const String    data_path = "/settings/wifi.cfg";
    const uint16_t  data_update_delay = 0;
    server_config_t default_wifi_cfg;
    
    // Отдельно для device_info (не сохраняется в FS)
    // device_info_t передаётся при запуске
    
    etl::shared_ptr<etl::settings::data<etl::wifi::server_config_t>> wifi_cfg;
    
    bool init_config(const server_config_t& default_cfg, bool reset_to_default = false);
    bool save_config(const server_config_t& cfg);
    server_config_t load_config();
}
```

#### 5.2. Обновить `load_config()`

Убрать обновление `device_info` из загруженной конфигурации (т.к. его там больше нет).

---

### 6. Объяснение параметра "key" в FileData

**Библиотека:** `gyverlibs/FileData`

**Параметр `key` (тип `uint8_t`)** — это **ключ первой записи**.

**Принцип работы:**
1. При инициализации `FileData` указывается ключ (например, `'A'`, `'F'`)
2. Если файл не существует **ИЛИ** не содержит указанный ключ → в файл записываются данные по умолчанию
3. Если файл существует и ключ совпадает → производится обычное чтение без перезаписи

**Рекомендации:**
- Не использовать значения `0` и `255`
- Использовать символы: `'A'`...`'Z'`, `'a'`...`'z'`
- Пример: `FileData("/config.bin", sizeof(Config), 'F')`

**Важно для данной задачи:**
- При изменении структуры `server_config_t` может потребоваться сменить ключ, чтобы старые конфигурации не загружались некорректно
- Или реализовать миграцию данных

---

## Чек-лист выполнения

- [x] 1. Создать define'ы размеров буферов
- [x] 2. Выделить `device_info_t` в отдельную структуру (на один уровень с `server_config_t`)
- [x] 3. Заменить все `String` поля в `server_config_t` на `char[]` массивы
- [x] 4. `device_info_t` оставить без изменений (со `String` полями)
- [x] 5. Реализовать setter'ы для `server_config_t` с очисткой буфера и контролем `\0`
- [x] 6. Реализовать getter'ы для `server_config_t` (возвращают `String`)
- [x] 7. Изменить сигнатуру `begin()` для приёма `device_info_t`
- [x] 8. Обновить `handle_api_config()` для работы с разделёнными данными
- [x] 9. Обновить HTML шаблон (translations, логика отображения)
- [x] 10. Обновить `settings::load_config()` (убрать обновление device info)
- [x] 11. Добавить метод `clear()` для `server_config_t`
- [x] 12. Проверить компиляцию для всех сред (d1_mini_lite, nodemcuv3, esp32c3)

---

## Статус

✅ **Задача выполнена** — все пункты чек-листа реализованы и протестированы.

## Правила разработки

Следовать правилам из `docs/rules/qwen_make_no_mistakes.md`:

1. **Не выдумывать API** — проверять существование методов в коде
2. **Раскрывать неопределённость** — добавлять `// FIXME` комментарии при сомнениях
3. **Верификация перед изменением** — читать файл перед редактированием
4. **Честная коммуникация** — признавать ошибки и исправлять

---

## Примечания

- Структура `server_config_t` теперь содержит **только WiFi настройки** и использует фиксированные `char[]` массивы для сохранения в FS
- Структура `device_info_t` **остаётся со `String` полями**, т.к. не сохраняется в FS (передаётся отдельно при запуске)
- SVG иконка в `device_info_t::icon_svg` может быть произвольного размера
- Set/Get методы для `server_config_t` обеспечивают безопасную работу со строками через `String` API
