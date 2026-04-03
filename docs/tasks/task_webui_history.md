# История изменений по задаче task_webui.md

## Менеджер управления серверами контента и настроек

### Выполненные задачи:

#### Task 2.1: Callback-механизм в web_server_base_t
- Файл: `lib\ETLTest\etl_webui_base.h`
- Добавлены типы callback-функций:
  - `on_settings_callback_t` - для запроса запуска сервера настроек
  - `on_content_callback_t` - для запроса запуска сервера контента
  - `on_factory_reset_t` - для запроса сброса настроек
- Добавлены методы установки callback'ов:
  - `set_on_settings_callback()`
  - `set_on_content_callback()`
  - `set_on_factory_reset_callback()`
- Добавлены protected поля для хранения callback'ов:
  - `m_on_settings_cb`
  - `m_on_content_cb`
  - `m_on_factory_reset_cb`
- Файл: `lib\ETLTest\etl_webui_base.cpp`
- Реализация методов установки callback'ов

#### Task 2.2: Обновление web_server_base_t::tick()
- Файл: `lib\ETLTest\etl_webui_base.cpp`
- Добавлена проверка `m_initialized` в начале `tick()`
- Если сервер не инициализирован, `tick()` возвращается сразу

#### Task 3.1: Базовый класс web_manager
- Файл: `lib\ETLTest\etl_webui_base.h`
- Создан класс `web_manager` после `web_server_base_t`
- Методы:
  - `start_content()` - создание и запуск сервера контента
  - `start_settings()` - создание и запуск сервера настроек
  - `toggle()` - переключение между серверами
  - `tick()` - вызов `m_server->tick()` с проверкой `m_server`
  - Методы установки callback'ов
- Виртуальные методы:
  - `on_create_content()` - pure virtual, override в приложении
  - `on_create_settings()` - virtual, реализация по умолчанию создаёт server_setup
- Protected поля:
  - `m_device_info`
  - `m_server`
  - Callback-поля

- Файл: `lib\ETLTest\etl_webui_base.cpp`
- Реализация всех методов `web_manager`
- `toggle()` определяет тип сервера по режиму WiFi (AP = settings)
- `on_create_settings()` создаёт server_setup с загруженными настройками

#### Task 4.1: Пользовательский менеджер light_webui_mgr
- Файл: `src\light_webui_mgr.h`
- Namespace: `light_control`
- Наследование от `etl::webui::web_manager`
- Реализация `on_create_content()`:
  - Загружает настройки WiFi через `load_wifi_config()`
  - Создаёт `light_control_server`
  - Устанавливает callback'и для переключения на настройки и factory reset
- Реализация `on_create_settings()`:
  - Загружает настройки WiFi через `load_wifi_config()`
  - Создаёт `server_setup`
  - Устанавливает callback'и для возврата к контенту и factory reset
- Приватный метод `handle_factory_reset()`:
  - Сбрасывает WiFi настройки к дефолтным
  - Сбрасывает UI настройки (если были инициализированы)
  - Запускает сервер настроек

#### Task 4.2: Обновление light_control_server для callback'ов
- Файл: `src\light_webui.h`
- Добавлен метод `handle_api_settings()` для обработки кнопки Settings

- Файл: `src\light_webui.cpp`
- Реализация `handle_api_settings()`:
  - Отправляет успешный ответ клиенту
  - Вызывает `m_on_settings_cb` для переключения на сервер настроек
- Добавлен маршрут `/api/settings` в `setup_http_routes()`

- Файл: `src\light_webui_html.h`
- Уже содержит кнопку Settings (`settingsBtn`) и функцию `showSettingsDialog()`

#### Task 4.3: Обновление server_setup для callback'ов
- Файл: `lib\ETLTest\etl_webui.h`
- Добавлен метод `handle_api_back()` для обработки кнопки Back

- Файл: `lib\ETLTest\etl_webui.cpp`
- Обновлён `handle_api_reset()`:
  - Теперь вызывает `m_on_factory_reset_cb` вместо прямого `reboot()`
  - Если callback не установлен, выполняет reboot как раньше
- Реализация `handle_api_back()`:
  - Отправляет успешный ответ клиенту
  - Вызывает `m_on_content_cb` для переключения на сервер контента
- Добавлен маршрут `/api/back` в `setup_http_routes()`

- Файл: `lib\ETLTest\etl_wifi_setup_html.h`
- Добавлена кнопка "Back" (`backBtn`) перед "Save & Reboot"
- Добавлен перевод для `back_btn` в en и ru
- Добавлена JavaScript функция `goBack()`:
  - Вызывает POST `/api/back`
  - Логгирует результат в консоль
- Добавлен event listener для `backBtn`

#### Task 5.1: Обновление main.cpp
- Файл: `src\main.cpp`
- Убрано прямое создание `wifi_server`
- Создан `webui_manager` типа `light_control::light_webui_mgr`
- Запуск через `start_content()` или `start_settings()` в зависимости от `simulation_data.start_webui_settings_on_start`
- В `loop()` вызывается `webui_manager->tick()`

#### Task 5.2: Обновление simulation_t
- Файл: `src\main.cpp`
- Уже существует поле `start_webui_settings_on_start` для тестирования запуска сервера настроек

### Структура файлов после изменений:

```
lib\ETLTest\
├── etl_webui_base.h          # web_server_base_t + web_manager (базовые классы ETL)
├── etl_webui_base.cpp        # Реализация базовых классов
├── etl_webui.h               # server_setup (WiFi Setup Server)
├── etl_webui.cpp             # Реализация server_setup
├── etl_webui_settings.h      # Конфигурационные структуры
├── etl_wifi_setup_html.h     # HTML шаблон для server_setup

src\
├── light_webui.h             # light_control_server (WebUI Server)
├── light_webui.cpp           # Реализация light_control_server
├── light_webui_html.h        # HTML шаблон для light_control_server
├── light_webui_mgr.h         # Пользовательский менеджер (namespace light_control)
└── main.cpp                  # Интеграция с менеджером
```

### Тестирование компиляции:

- [x] d1_mini_lite (ESP8266) — **SUCCESS** (RAM: 80.8%, Flash: 51.4%, 10.12s)
- [x] nodemcuv3 (ESP8266) — **SUCCESS** (RAM: 80.8%, Flash: 51.4%, 12.77s)
- [x] esp32c3 — **SUCCESS** (RAM: 12.6%, Flash: 56.4%, 20.54s)
- [x] esp32-wroom-32u — **SUCCESS** (RAM: 14.4%, Flash: 82.0%, 24.97s)

Все конфигурации успешно собраны без ошибок!

### Функциональное тестирование:

- [ ] WebUI доступен после старта
- [ ] 3 нажатия переключают на WiFi Setup (нужно добавить обработку кнопки)
- [ ] WebUI останавливается при переключении (mDNS перезапускается)
- [ ] [Save & Reboot] переключает обратно на WebUI с новыми настройками
- [ ] [Back] переключает обратно на WebUI без сохранения
- [ ] [Factory Reset] сбрасывает WiFi и UI настройки, запускает WiFi Setup
- [ ] mDNS работает для обоих серверов после переключения
- [ ] tick() не вызывает ошибок при переключении серверов

### Примечания:

1. Callback-механизм работает через лямбда-функции в `light_webui_mgr`
2. Менеджер полностью управляет жизненным циклом серверов
3. При переключении старый сервер уничтожается (деструктор вызывает stop())
4. mDNS перезапускается вместе с сервером
5. Factory Reset сбрасывает и WiFi, и UI настройки
