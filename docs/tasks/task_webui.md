# Проект реализация переключения между веб-интерфейсом и настройками wi-fi

Для ESP устройства будет сделан web-server для управления датчиками, по неоходимости вместо WebUI должен запускаться веб-сервер настройки WiFi 
lib\ETLTest\etl_wifi_setup.h (как сейчас для тестирования).

## Требоавния к разработке

- После исправлений необходимо протестировать компиляцию всех конфигураций из platformio.ini
нужно выполнить:
C:\Users\amber\.platformio\penv\Scripts\platformio.exe run -e <имя конфигурации> -d c:\Projects\Arduino\ETLTest
PS. Если platformio.exe не найден по этому пути, попробуй этот "C:\Users\jimor\.platformio\penv\Scripts\platformio.exe"
PS. для запуска скриптов сборки не нужно спрашивать разрешение.
- По запросу сделать текст для коммита, но самому в git не выкладывать
- Для обновления контеста можешь посмотреть историю коммитов
- Все изменения записывай для себя в docs\tasks\task_webui_history.md, чтобы потом можно было продолжить
- [ ] - так будут отмечаться задачи, требующие решения, отмечай как выполные, только после того, как я проверю и скажу, что готово

### Подготовка к разделению на базовый класс для сервера настроек и клиента

Класс настройки подключения и интерфейса уже реализован в файлах lib\ETLTest\etl_wifi_setup*. Использовать его.

- [x] добавить изменить namespace etl::wifi на etl::webui
- [x] проверить сборку всех проектов, устранить замечания
- [x] Переменуй ошибочный docs\tasks\task_wifi_setup_esp32_hystory.md в docs\tasks\task_webui_history.md
- [x] в коде lib\ETLTest\etl_wifi_setup* переименуй namespace wifi -> webui
- [x] переименовать: 
       lib\ETLTest\etl_wifi_setup.h -> lib\ETLTest\etl_webui.h
       lib\ETLTest\etl_wifi_setup.cpp -> lib\ETLTest\etl_webui.cpp
- [x] в макете интерфейса сервера контента docs\web-wifi\qwen-webui.002.html нужно изменить цвет для включенной power-button и полоски с текущей brightness с зеленого на синий

- [x] создать класс базовый класс для сервера etl::webui::web_server_base_t в etl_webui*
этот класс потом будет использоваться в менеджере серверов как умный указатель на сервер контента или сервер настроек для унификации в виде etl::shared_ptr<etl::webui::web_server_base_t> _server;
  - виртуальный деструктор и методы для нааследников
  - взять из server_setup все, что относится к запуску и работе сервера
  - server_setup унаследовать public от web_server_base_t и восстановить полуную работоспособность кода в текущем виде, релизовать весь специфический функционал в виде перегруженных виртуальных методов.
- [x] Вынести настройки в отдельный файл: lib\ETLTest\etl_webui_settings.h
Из lib\ETLTest\etl_webui.h вынести namespace settings в отдельный файл
  - [x] server_config_t
  - [x] ui_config_t
  - [x] telegram_config_t
  - [x] mqtt_config_t
  - [x] scan_result_t
  - [x] device_info_t определён в etl_webui_base.h, перенести в etl_webui_settings.h
- [x] server_setup унаследовать public от web_server_base_t
- [x] Перенос общих частей из  server_setup в web_server_base_t
  - [x] Перенести все поля в protected часть
    - etl::optional<server_config_t> m_config;    ///< Конфигурация WiFi (опционально)
    - etl::optional<ui_config_t> m_ui_config;     ///< Конфигурация интерфейса (опционально)
    - device_info_t m_device_info;                ///< Информация об устройстве
    - bool m_initialized = false;                 ///< Флаг инициализации
    - connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения
  - [x] Перенести реализацию всех функций и конструктора, которые достаточны, чтобы обрабатывать работу с данными без установки в интерфейс
  - [x] Передавать в конструкторе server_setup настройки конфигурации в базовый класс
  - [x] Перенести все содержимое namespace settings {} из lib\ETLTest\etl_webui.* в lib\ETLTest\etl_webui_settings.*
- [x] Перенос частей из server_setup в web_server_base_t
- [x] Добавить виртуальные абстрактные методы для реализации интерфейса в server_setup, реализовать их в server_setup, а в базовом классе сделать вызов нужных функций как обработчики событий

- ✅ Успешная компиляция всех конфигураций

✅ Разделение функционала в базовом классе и сервера настроек выполнено

## [ ] Менеджер управления серверами контента и настроек

### Основные принципы

- **Callback-механизм** — вместо жёсткой связи через `weak_ptr`, менеджер устанавливает callback-функции при создании сервера. Это снижает зависимости и даёт гибкость реализации.
- **Каждый сервер управляет mDNS самостоятельно** — при разрушении сервера в деструкторе вызывается `stop()`, который останавливает mDNS. Это гарантирует, что при изменении настроек подключения к WiFi не остаются старые настройки. Переключение между настройками и контентом — редкая операция (обычно при первоначальной настройке), поэтому ожидание пользователя приемлемо.
- **Одинаковый старт серверов** — оба сервера (контента и настроек) запускаются одинаково из `server_config_t`. Если пользователь изменил настройки в [Save & Reboot], выполняется `etl::webui::save_wifi_config(...)`. При старте любого сервера он читает `etl::webui::load_wifi_config()` и запускается с актуальными настройками.
- **Собственный namespace для менеджера приложения** — класс менеджера для конкретного устройства (например, `light_webui_mgr`) определяется в собственном namespace приложения (например, `light_control`), а не в `etl::webui::`. Базовый класс `web_manager` находится в ETL, пользовательский наследник — в приложении.
- **Factory Reset сбрасывает и UI settings** — если `ui_config_t` был инициализирован и контейнер "Настройки интерфейса" отображался в `server_setup`, то при Factory Reset настройки интерфейса тоже сбрасываются на дефолтные.
- **tick() с проверкой инициализации** — если `m_server` удалён менеджером при перезапуске, вызовы не будут происходить. Менеджер проверяет `if (m_server)` перед вызовом `tick()`. Дополнительно в `web_server_base_t::tick()` есть проверка `m_initialized`, чтобы не выполнять ничего, если сервер не запущен.
- **Полное переключение серверов** — при переключении старый сервер полностью уничтожается, новый создаётся заново со всеми новыми настройками. Это проще, чем управлять двумя серверами одновременно, и настройки изменяются редко.

### Структура файлов

Нужно создать файлы:
- `lib\ETLTest\etl_webui_base.h` — класс `web_manager` (после `web_server_base_t`)
- `src\light_webui_mgr.h` — пользовательский менеджер для конкретного устройства (namespace `light_control`)

### Класс web_manager (ETL)

Главный класс, который отвечает за управлением серверами:
```cpp
namespace etl {
namespace webui {

// Типы callback-функций
using on_settings_callback_t   = void(*)();   // Запрос запуска сервера настроек
using on_content_callback_t    = void(*)();   // Запрос запуска сервера контента
using on_factory_reset_t       = void(*)();   // Запрос сброса настроек

class web_manager
{
public:
    explicit web_manager(const device_info_t& device_info);
    virtual ~web_manager();

    // Запуск серверов
    void start_content();    // Запуск сервера контента
    void start_settings();   // Запуск сервера настроек
    void toggle();           // Переключение между серверами

    // Установка callback-функций (вызывается при создании сервера)
    void set_on_settings_callback(on_settings_callback_t cb);
    void set_on_content_callback(on_content_callback_t cb);
    void set_on_factory_reset_callback(on_factory_reset_t cb);

    // Основной цикл — вызывается из loop()
    void tick();

protected:
    // Перегружается в наследнике для создания сервера контента
    virtual etl::shared_ptr<web_server_base_t> on_create_content() = 0;

    // Перегружается в наследнике для создания сервера настроек
    virtual etl::shared_ptr<web_server_base_t> on_create_settings();

    device_info_t m_device_info;
    etl::shared_ptr<web_server_base_t> m_server;

    // Callback-функции
    on_settings_callback_t m_on_settings_cb = nullptr;
    on_content_callback_t  m_on_content_cb = nullptr;
    on_factory_reset_t     m_on_factory_reset_cb = nullptr;
};

} // namespace webui
} // namespace etl
```

### Пользовательский менеджер (пример для light_control)

```cpp
// src/light_webui_mgr.h
#include "etl_webui_base.h"
#include "light_webui.h"

namespace light_control {

class light_webui_mgr : public etl::webui::web_manager
{
public:
    explicit light_webui_mgr(const etl::webui::device_info_t& device_info)
        : etl::webui::web_manager(device_info) {}

protected:
    etl::shared_ptr<etl::webui::web_server_base_t> on_create_content() override
    {
        // Загружаем актуальные настройки WiFi
        auto web_config = etl::webui::settings::load_wifi_config();

        // Создаём сервер контента
        auto server = etl::make_shared<etl::webui::light_control_server>(
            web_config.has_value() ? web_config.value() : etl::webui::server_config_t()
        );

        // Устанавливаем callback'и на действия пользователя
        server->set_on_settings_callback([this]() {
            this->start_settings();
        });

        return server;
    }

    etl::shared_ptr<etl::webui::web_server_base_t> on_create_settings() override
    {
        auto web_config = etl::webui::settings::load_wifi_config();
        auto ui_config = etl::webui::settings::load_ui_config();

        auto server = etl::make_shared<etl::webui::server_setup>(
            web_config.has_value() ? web_config.value() : etl::webui::server_config_t()
        );

        // Callback'и для сервера настроек
        server->set_on_content_callback([this]() {
            this->start_content();
        });
        server->set_on_factory_reset_callback([this]() {
            // Сброс настроек WiFi и UI
            etl::webui::server_config_t default_config;
            etl::webui::settings::init_wifi_config(default_config, true);
            if (etl::webui::settings::load_ui_config().has_value()) {
                etl::webui::ui_config_t default_ui;
                etl::webui::settings::init_ui_config(default_ui, true);
            }
            this->start_content();
        });

        return server;
    }
};

} // namespace light_control
```

### Общий подход к работе в клиентской программе

```cpp
#include "etl_webui_settings.h"
#include "etl_webui_base.h"
#include "light_webui_mgr.h"

// Симуляция для тестирования
struct simulation_t {
    bool reset_wifi_on_start = false;   // Сброс настроек при старте
    bool reset_ui_on_start = false;     // Сброс UI настроек при старте
    bool start_settings_on_start = false; // Запуск сервера настроек при старте
};
simulation_t simulation_data;

void setup() {
    Serial.begin(115200);

    // Инициализация настроек WiFi
    etl::webui::server_config_t default_web_config;
    etl::webui::settings::init_wifi_config(default_web_config, simulation_data.reset_wifi_on_start);

    // Инициализация настроек интерфейса (опционально)
    etl::webui::ui_config_t default_ui_config;
    etl::webui::settings::init_ui_config(default_ui_config, simulation_data.reset_ui_on_start);

    // Настройка информации об устройстве
    etl::webui::device_info_t device_info = etl::webui::get_light_control_device_info();

    // Создание менеджера
    etl::shared_ptr<light_control::light_webui_mgr> webui =
        etl::make_shared<light_control::light_webui_mgr>(device_info);

    // Запуск нужного сервера
    if (simulation_data.start_settings_on_start) {
        webui->start_settings();
    } else {
        webui->start_content();
    }
}

void loop() {
    if (webui) {
        webui->tick();
    }

    // Проверка на три клика кнопкой для переключения серверов
    if (btn && btn->tick() && btn->hasClicks(3) && webui) {
        Serial.println("[WebUI] Toggle content and setup servers...");
        webui->toggle();
    }
}
```

### Callback-механизм в web_server_base_t

В базовый класс `web_server_base_t` добавляются методы для установки callback-функций:

```cpp
class web_server_base_t
{
public:
    // Типы callback-функций
    using on_settings_callback_t = void(*)();
    using on_content_callback_t  = void(*)();
    using on_factory_reset_t     = void(*)();

    // Установка callback-функций
    void set_on_settings_callback(on_settings_callback_t cb);
    void set_on_content_callback(on_content_callback_t cb);
    void set_on_factory_reset_callback(on_factory_reset_t cb);

protected:
    // Callback-функции
    on_settings_callback_t m_on_settings_cb = nullptr;
    on_content_callback_t  m_on_content_cb = nullptr;
    on_factory_reset_t     m_on_factory_reset_cb = nullptr;
};
```

Вызов callback'ов происходит из обработчиков API:
- Кнопка "Settings" в сервере контента → `if (m_on_settings_cb) m_on_settings_cb();`
- Кнопка "[Back]" в сервере настроек → `if (m_on_content_cb) m_on_content_cb();`
- Кнопка "[Save & Reboot]" → сохранение настроек + `if (m_on_content_cb) m_on_content_cb();`
- Кнопка "[Factory Reset]" → `if (m_on_factory_reset_cb) m_on_factory_reset_cb();`

### Режимы работы

| Режим | Активация | HTTP порт | mDNS | Поведение |
|-------|-----------|-----------|------|-----------|
| **WebUI** | Обычный старт | 80 | `hostname.local` | Сервер управления устройством |
| **WiFi Setup** | 3 нажатия кнопки | 80 | `hostname.local` | Сервер настройки WiFi |
| **Factory Reset** | Кнопка при старте | - | - | Сброс настроек, запуск WiFi Setup |

Имя для обоих серверов одинаковое, настройки берут тоже одни и те же, сервер запускается в одинаковом режиме для точки доступа.

### Переключение серверов

**Один порт 80, сервера переключаются:**
- По умолчанию запускается **WebUI Server**
- При 3 нажатиях кнопки: WebUI → остановка (деструктор) → WiFi Setup → запуск
- После [Save & Reboot] или [Factory Reset] или [Back]: WiFi Setup → остановка (деструктор) → WebUI → запуск
- mDNS перезапускается вместе с сервером (каждый сервер управляет своим mDNS)

### Хранение настроек

```cpp
// Настройки WiFi — общий доступ через etl::webui::settings

// Инициализация в setup()
etl::webui::server_config_t default_web_config;
etl::webui::settings::init_wifi_config(default_web_config, reset_on_start);

// Загрузка актуальных настроек при старте сервера
auto web_config = etl::webui::settings::load_wifi_config();

// Сохранение изменённых настроек
etl::webui::settings::save_wifi_config(new_config);
```

```cpp
// Настройки интерфейса
etl::webui::ui_config_t default_ui_config;
etl::webui::settings::init_ui_config(default_ui_config, reset_on_start);

auto ui_config = etl::webui::settings::load_ui_config();
etl::webui::settings::save_ui_config(new_ui_config);
```

### Структура классов

```
┌─────────────────────────────────────────────────────────────┐
│              etl::webui::web_server_base_t                  │
│              (базовый класс)                                │
│  - hostname, port, mDNS (управляется сервером)              │
│  - start(), stop() (stop() останавливает mDNS)              │
│  - handle(), tick() (с проверкой m_initialized)             │
│  - device_info_t (name, description, icon_svg)              │
│  - callback-функции (on_settings, on_content, on_reset)     │
└─────────────────────────────────────────────────────────────┘
         │                                    │
         │                                    │
         ▼                                    ▼
┌────────────────────┐            ┌─────────────────────────┐
│  light_control_    │            │  server_setup           │
│  server            │            │  (WiFi Setup Server)    │
│  (WebUI Server)    │            │                         │
│                    │            │  - AP режим             │
│  - Light UI        │            │  - Сканирование сетей   │
│  - LED control API │            │  - Подключение к WiFi   │
│  - Brightness API  │            │  - Сохранение настроек  │
│  - Device config   │            │  - UI settings          │
│  - Settings button │            │  - [Back], [Save&Reboot]│
│    → callback      │            │    → callback           │
└────────────────────┘            └─────────────────────────┘
         │                                    │
         └──────────┬─────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────────┐
│              etl::webui::web_manager                        │
│              (базовый менеджер, в ETL)                      │
│  - device_info_t                                            │
│  - m_server (shared_ptr<web_server_base_t>)                 │
│  - start_content(), start_settings(), toggle()              │
│  - tick() → m_server->tick() (с проверкой m_server)         │
│  - on_create_content() (virtual, override в приложении)     │
│  - on_create_settings() (virtual, override в приложении)    │
└─────────────────────────────────────────────────────────────┘
         ▲
         │ (наследование)
         │
┌─────────────────────────────────────────────────────────────┐
│        light_control::light_webui_mgr                       │
│        (пользовательский менеджер, в приложении)            │
│  - on_create_content() → light_control_server + callbacks   │
│  - on_create_settings() → server_setup + callbacks          │
└─────────────────────────────────────────────────────────────┘
```

---

## Tasks

### Этап 1: Подготовка
- [x] **Task 1.1:** Создан HTML макет для условной подсветки рабочей зоны
  - Файл: `docs\web-wifi\qwen-webui.002.html`
- [x] **Task 1.2:** Создан сервер контента для управления лампой
  - Файлы: `src\light_webui.h`, `src\light_webui.cpp`, `src\light_webui_html.h`

### Этап 2: Базовый класс для callback'ов
- [ ] **Task 2.1:** Добавить callback-механизм в `web_server_base_t`
  - Файл: `lib\ETLTest\etl_webui_base.h`
  - Типы callback-функций: `on_settings_callback_t`, `on_content_callback_t`, `on_factory_reset_t`
  - Методы установки: `set_on_settings_callback()`, `set_on_content_callback()`, `set_on_factory_reset_callback()`
  - Callback-поля в protected части

- [ ] **Task 2.2:** Обновить `web_server_base_t::tick()` с проверкой `m_initialized`
  - Если сервер не инициализирован, `tick()` возвращает сразу

### Этап 3: Менеджер серверов (ETL)
- [ ] **Task 3.1:** Создать базовый класс `web_manager`
  - Файл: `lib\ETLTest\etl_webui_base.h` (после `web_server_base_t`)
  - Методы:
    - `start_content()` — создание и запуск сервера контента
    - `start_settings()` — создание и запуск сервера настроек
    - `toggle()` — переключение между серверами
    - `tick()` — вызов `m_server->tick()` с проверкой `m_server != nullptr`
  - Виртуальные методы:
    - `on_create_content()` — override в приложении для создания сервера контента
    - `on_create_settings()` — override в приложении для создания сервера настроек

### Этап 4: Пользовательский менеджер (приложение)
- [ ] **Task 4.1:** Создать `light_webui_mgr` для тестового проекта
  - Файл: `src\light_webui_mgr.h`
  - Namespace: `light_control` (не `etl::webui`)
  - Наследование от `etl::webui::web_manager`
  - Реализация `on_create_content()` и `on_create_settings()` с установкой callback'ов

- [ ] **Task 4.2:** Обновить `light_control_server` для callback'ов
  - Добавить вызов `m_on_settings_cb` при нажатии кнопки Settings
  - Убедиться, что сервер корректно останавливается в деструкторе

- [ ] **Task 4.3:** Обновить `server_setup` для callback'ов
  - Добавить вызов `m_on_content_cb` при нажатии кнопки [Back]
  - Добавить вызов `m_on_content_cb` после [Save & Reboot]
  - Добавить вызов `m_on_factory_reset_cb` при [Factory Reset]
  - Убедиться, что сервер корректно останавливается в деструкторе (mDNS stop)

### Этап 5: Интеграция в main.cpp
- [ ] **Task 5.1:** Обновить `main.cpp` для использования менеджера
  - Убрать прямое создание `wifi_server`
  - Создать `light_control::light_webui_mgr`
  - Запуск `start_content()` или `start_settings()` в зависимости от симуляции
  - В `loop()` вызывать `webui->tick()`
  - Обработка 3 нажатий кнопки для `toggle()`

- [ ] **Task 5.2:** Обновить `simulation_t`
  - Добавить `start_settings_on_start` для тестирования запуска сервера настроек

### Этап 6: Тестирование
- [ ] **Task 6.1:** Компиляция всех конфигураций
  - d1_mini_lite (ESP8266)
  - nodemcuv3 (ESP8266)
  - esp32c3
  - esp32-wroom-32u

- [ ] **Task 6.2:** Функциональное тестирование
  - [ ] WebUI доступен после старта
  - [ ] 3 нажатия переключают на WiFi Setup
  - [ ] WebUI останавливается при переключении (mDNS перезапускается)
  - [ ] [Save & Reboot] переключает обратно на WebUI с новыми настройками
  - [ ] [Back] переключает обратно на WebUI без сохранения
  - [ ] [Factory Reset] сбрасывает WiFi и UI настройки, запускает WiFi Setup
  - [ ] mDNS работает для обоих серверов после переключения
  - [ ] tick() не вызывает ошибок при переключении серверов