# История разработки: Переключение между WebUI и WiFi Setup

## Контекст

Для ESP устройства реализуется переключение между веб-интерфейсом управления датчиками и сервером настройки WiFi.

## Текущее состояние

### Выполнено

#### Этап 1: Макет WebUI
- [x] **Task 1.1:** Создан HTML макет для условной подсветки рабочей зоны
  - Файл: `docs\web-wifi\qwen-webui.001.html` (базовая версия)
  - Файл: `docs\web-wifi\qwen-webui.002.html` (текущая версия)
  
  **Реализованные функции в qwen-webui.002.html:**
  - Status bar с иконкой устройства и статусными индикаторами [W] [M] [T]
  - Кнопка настроек с переключением темной темы
  - Power button с индикацией включения
  - Brightness section со слайдером и кнопками +/-
  - Темная тема в стиле iOS (цвета из docs/etl_wifi_setup.md)
  - Адаптивный дизайн для мобильных устройств

#### Подготовка к разделению (Task 2.x)
- [x] **Task 2.1:** Изменить namespace etl::wifi на etl::webui (отмечено в task_webui.md)
- [x] **Task 2.2:** Проверить сборку всех проектов (отмечено в task_webui.md)
- [x] **Task 2.3:** Переименован файл истории в task_webui_history.md

### В процессе

### Ожидает выполнения

[STOP] - Дальнейшие задачи в процессе продумывания

---

## История изменений

### 1 апреля 2026 г.

**qwen-webui.002.html:**
- Добавлена рамка вокруг кнопки settings (в стиле кнопок brightness +/-)
- Power button смещена вверх (margin-top: -20px) для центрирования
- Иконка устройства перенесена в status-bar
- Status иконки [W] [M] [T] размещены с gap 2px
- Header центрирован, device-info-container упрощен
- Добавлен обработчик кнопки settings - переключение темной темы
- Обновлена документация в docs/tasks/task_light_webui.md

**Переименование namespace wifi -> webui:**
- Обновлены файлы:
  - `lib\ETLTest\etl_wifi_setup.h` - namespace etl::wifi -> etl::webui
  - `lib\ETLTest\etl_wifi_setup.cpp` - namespace etl::wifi -> etl::webui
  - `lib\ETLTest\etl_wifi_setup_html.h` - namespace etl::wifi -> etl::webui
  - `src\main.cpp` - все ссылки на etl::wifi:: заменены на etl::webui::
- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266)
  - ✅ esp32c3 (ESP32-C3)
  - ✅ esp32-wroom-32u (ESP32)

**Изменение цветовой схемы power-button и brightness:**
- Power-button: зелёный (#34C759) → синий неоновый (#007AFF)
  - border-color: #007AFF
  - box-shadow: rgba(0, 122, 255, 0.4/0.5)
- Brightness-fill: зелёный (#34C759/#30D158) → синий (#007AFF/#0A84FF)

**Создание базового класса web_server_base_t:**
- Создан файл `lib\ETLTest\etl_webui_base.h` с базовым классом `etl::webui::web_server_base_t`
- Базовый класс включает:
  - Виртуальный деструктор
  - Чистые виртуальные методы: begin(), stop(), handle(), is_initialized(), get_connection_status(), is_connected(), get_ip_address(), get_mode(), get_hostname(), get_port()
  - protected-члены: m_initialized, m_connection_status
  - device_info_t и connection_status_t перенесены в базовый класс
- `server_setup` унаследован от `web_server_base_t`
- Реализованы методы get_hostname() и get_port() в server_setup
- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266)
  - ✅ esp32c3 (ESP32-C3)
  - ✅ esp32-wroom-32u (ESP32)

### 2 апреля 2026 г.

**Перенос namespace settings в etl_webui_settings.*:**
- Обновлён `lib\ETLTest\etl_webui_settings.h`:
  * Добавлены include: `<etl/etl_memory.h>`, `<etl/etl_optional.h>`
  * Добавлен `namespace settings` с объявлениями функций: init_wifi_config, save_wifi_config, load_wifi_config, init_ui_config, save_ui_config, load_ui_config, init_telegram_config, save_telegram_config, load_telegram_config, init_mqtt_config, save_mqtt_config, load_mqtt_config

- Обновлён `lib\ETLTest\etl_webui_settings.cpp`:
  * Добавлены include: `etl/etl_littlefs.h`, `etl/etl_settings.h`, `<etl/etl_memory.h>`
  * Перенесён `namespace settings` с реализацией всех функций из etl_webui.cpp
  * Реализации структур server_config_t, ui_config_t, telegram_config_t, mqtt_config_t перемещены после закрывающей скобки namespace settings

- Обновлён `lib\ETLTest\etl_webui.h`:
  * Удалён `namespace settings` (перенесён в etl_webui_settings.h)

- Обновлён `lib\ETLTest\etl_webui.cpp`:
  * Удалён `namespace settings` с реализацией (перенесён в etl_webui_settings.cpp)

- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266) — 6.91 сек
  - ✅ esp32c3 (ESP32-C3) — 9.92 сек
  - ✅ esp32-wroom-32u (ESP32) — 13.87 сек

**Перенос общих частей из server_setup в web_server_base_t:**
- Обновлён `lib\ETLTest\etl_webui_base.h`:
  - Добавлен конструктор `explicit web_server_base_t(const etl::optional<server_config_t>& cfg = {})`
  - Добавлен метод `bool init_mdns(const String& hostname)` для инициализации mDNS
  - Перенесены protected-поля:
    - `etl::optional<server_config_t> m_config`
    - `etl::optional<ui_config_t> m_ui_config`
    - `device_info_t m_device_info`
    - `bool m_initialized`
    - `connection_status_t m_connection_status`
    - `etl::shared_ptr<etl_web_server_t> m_server`

- Создан `lib\ETLTest\etl_webui_base.cpp` с реализацией:
  - Конструктор базового класса
  - Метод `init_mdns()` для инициализации mDNS сервиса

- Обновлён `lib\ETLTest\etl_webui.h`:
  - Удалены дублирующие поля из `server_setup` (перенесены в базовый класс)
  - Удалён `m_server` из protected-секции `server_setup`
  - Оставлены только специфические поля: `m_scan_cache`, `m_scan_timestamp`, `SCAN_CACHE_TIME`

- Обновлён `lib\ETLTest\etl_webui.cpp`:
  - Добавлен `#include "etl_webui_base.cpp"`
  - Обновлён конструктор `server_setup` для передачи конфигурации в базовый класс: `: web_server_base_t(cfg)`

- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266) — 11.54 сек
  - ✅ esp32c3 (ESP32-C3) — 18.75 сек
  - ✅ esp32-wroom-32u (ESP32) — 20.27 сек

**Вынос настроек в отдельный файл etl_webui_settings.h:**
- Создан файл `lib\ETLTest\etl_webui_settings.h` с конфигурационными структурами:
  - `server_config_t` — настройки WiFi сервера (hostname, AP/STA credentials, port, update_interval)
  - `ui_config_t` — настройки интерфейса (language, dark_theme, large_font, use_bold_values)
  - `telegram_config_t` — настройки Telegram бота (TODO)
  - `mqtt_config_t` — настройки MQTT (TODO)
  - `scan_result_t` — результат сканирования WiFi сети
- Создан файл `lib\ETLTest\etl_webui_settings.cpp` с реализацией методов структур
- Обновлён `lib\ETLTest\etl_webui.h`:
  - Добавлен `#include "etl_webui_settings.h"`
  - Удалены дублирующие определения структур (перенесены в etl_webui_settings.h)
- Обновлён `lib\ETLTest\etl_webui.cpp`:
  - Добавлен `#include "etl_webui_settings.cpp"` в конец файла
  - Удалены дублирующие реализации методов структур
- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266) — 17.23 сек
  - ✅ esp32c3 (ESP32-C3) — 17.75 сек
  - ✅ esp32-wroom-32u (ESP32) — 23.06 сек

**Перенос device_info_t и connection_status_t в etl_webui_settings.h:**
- Из `lib\ETLTest\etl_webui_base.h` перенесены структуры:
  - `device_info_t` — информация об устройстве (name, description, icon_svg)
  - `connection_status_t` — статус подключения к WiFi
- В `lib\ETLTest\etl_webui_base.h` добавлен `#include "etl_webui_settings.h"`
- Обновлены комментарии в `lib\ETLTest\etl_webui.h` о расположении структур
- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266) — 6.87 сек
  - ✅ esp32c3 (ESP32-C3) — 8.86 сек
  - ✅ esp32-wroom-32u (ESP32) — 11.92 сек
