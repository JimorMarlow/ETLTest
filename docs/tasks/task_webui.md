# Проект реализация переключения между веб-интерфейсом и настройками wi-fi

Для ESP устройства будет сделан web-server для управления датчиками, по неоходимости вместо WebUI должен запускаться веб-сервер настройки WiFi 
lib\ETLTest\etl_wifi_setup.h (как сейчас для тестирования).

## Требоавния к разработке

- После исправлений необходимо протестировать компиляцию всех конфигураций из platformio.ini
нужно выполнить:
C:\Users\amber\.platformio\penv\Scripts\platformio.exe run -e <имя конфигурации> -d c:\Projects\Arduino\ETLTest
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
- [x] проверь компиляцию всех env в platformio

[STOP] - дальше не читай, это еще в процессе продумывания и разработки. После детализации будем двигаться дальше


- [ ] Добавить виртуальные абстрактные методы для реализации интерфейса в server_setup, реализовать их в server_setup, а в базовом классе сделать вызов нужных функций как обработчики событий


## Архитектура

Нужно создать файлы:
lib\ETLTest\etl_webui.* с классом для управления веб-серверами

Главный класс, который отвечает за управлением серверами:
пространство имен etl::webui
класс: web_manager

Он включает в себя создание и вызов сервера контента
server_content
который является базовым для клиентских программ.

Общий подход в новом приложении:
- наследуешь свой сервер контента для управления датчиками или показа результатов
нр: class kitchen_light_webui : public etl::webui::web_server_base_t
- переопределяешь класс: 
class kl_webui : public etl::webui::web_manager ...
virtual etl::shared_ptr<etl::webui::web_server_base_t> on_create_content() override;

в нем создаешь экземпляр kitchen_light_webui и отдаешь менеджеру

- Менеджер выполняет следующие функции:
  - при создании получает etl::wifi::device_info_t device_info и запоминает его. При созаднии сервера контента или настроек передает эту информацию в них
  - вызов метода для создания клиентского сервера контента, и запоминает его
  - создание server_setup для настройки подключения к wifi, управление интерфейсом, подключения к mqtt (в перспективе), подключения к телеграм боту (в переспективе)
  - в один момент времени работает только один из этих серверов на одних и тех же настройках, портах и IP адресе.
  - сервер контента должен реализовать вызов менеджера через запомненный etl::weak_ptr<etl::webui::web_manager> parent по нажатию на кнопку " settings", менеджер должен разрушить сервер контента и запустить сервер настроек. 
  - Сервер настроек по команде "back", "Save & Reboot", "Factory Reset" должен сказать менеджеру, что изменение настроек завершено и менеджер закроет сервер настроек, запросит on_create_content() для создания контента и запустит его. пользователь обновит интерфейс и увидит новый сервер. Возможно, придется переподключиться к нужной wifi сети
  - void tick() - этот метод будет вызываться из loop(), в нем менеджер должен выолнять вызовы для активного сервера
  server->handle();        // Обновление статуса
  server->handle_client(); // Обработка HTTP запросов

### Режимы работы

| Режим | Активация | HTTP порт | mDNS | Поведение |
|-------|-----------|-----------|------|-----------|
| **WebUI** | Обычный старт | 80 | `espdevice.local` | Сервер управления устройством |
| **WiFi Setup** | 3 нажатия кнопки | 80 | `espdevice.local` | Сервер настройки WiFi |
| **Factory Reset** | Кнопка при старте | - | - | Сброс настроек, запуск WiFi Setup |

Имя для обоих серверов одинаковое, настройки берут тоже одни и теже, сервер забирается в одинаковом режиме для точки доступа.

### Переключение серверов

**Один порт 80, сервера переключаются:**
- По умолчанию запускается **WebUI Server**
- При 3 нажатиях кнопки: WebUI → остановка → WiFi Setup → запуск
- После [Save & Reboot] или [Factory Reset] или [Back] : WiFi Setup → остановка → WebUI → запуск

### Структура классов

```
┌─────────────────────────────────────────────────────────────┐
│                    etl::webui::web_server_base_t            │
│                    (базовый класс)                          │
│  - hostname, port, mDNS                                     │
│  - start(), stop(), handle()                                │
│  - device_info_t (name, description, icon_svg)              │
└─────────────────────────────────────────────────────────────┘
         │                                    │
         │                                    │
         ▼                                    ▼
┌────────────────────┐            ┌─────────────────────────┐
│   server_content_t │            │  wifi::server_setup_t   │
│   (WebUI Server)   │            │  (WiFi Setup Server)    │
│                    │            │                         │
│  - Light UI        │            │  - AP режим             │
│  - LED control API │            │  - Сканирование сетей   │
│  - Brightness API  │            │  - Подключение к WiFi   │
│  - Device config   │            │  - Сохранение настроек  │
└────────────────────┘            └─────────────────────────┘
```

### Хранение настроек

```cpp
// Настройки WiFi перемещены в ETL библиотеку
// Общий доступ через etl::wifi::settings

// В main.cpp:
etl::wifi::server_config_t web_config; // default settings
    // В setup() или до начала работы с WiFi
etl::wifi::settings::init_wifi_config(web_config, simulation_data.reset_wifi_on_start);

// Для получения 
etl::optional<etl::wifi::settings::server_config_t> etl::wifi::settings::load_wifi_config();
```

```cpp
// Настройки интерфейса
// Общий доступ через etl::wifi::settings
etl::wifi::ui_config_t ui_config; // default UI settings 
etl::wifi::settings::init_ui_config(ui_config, simulation_data.reset_ui_on_start);

// Получение настроек интерфейса
... etl::wifi::settings::load_ui_config(); // Может отсутствовать, проверять результат
```

### Симуляция в тестовом проекте

```cpp
struct simulation_t {
    bool reset_wifi_on_start = false;   // Сброс настроек при старте
    bool custom_device_info = true;     // Кастомная информация об устройстве
    bool custom_icon_svg = false;       // Кастомная иконка
    bool triple_press = false;          // Симуляция 3 нажатий кнопки
    bool long_press_at_start = false;   // Симуляция долгого нажатия при старте
};
```

---

## Tasks

### Этап 1: Макет WebUI
- [x] **Task 1.1:** Создан HTML макет для условной подсветки рабочей зоны - это будет главный экран контента
  - Файл: `docs\web-wifi\qwen-webui.001.html`
  
### Этап 2: Базовый класс Web Server
- [ ] **Task 2.1:** Создать базовый класс в ETL
  - Файл: `ETL/src/etl/etl_web_server_base.h`
  - Содержимое:
    - `device_info_t` (перенести из wifi_setup)
    - `etl_web_server_base_t` — виртуальные методы:
      - `begin(device_info_t)` — запуск сервера
      - `stop()` — остановка сервера
      - `handle()` — обработка событий
      - `is_initialized()` — статус инициализации
  - Перенести `device_info_t` из `etl_wifi_setup.h` в базовый класс

- [ ] **Task 2.2:** Обновить WiFi Setup Server
  - Наследование от `etl_web_server_base_t`
  - Переименовать `server_setup` → `server_setup_t`
  - Сохранить всю текущую функциональность

### Этап 3: Реализация WebUI Server
- [ ] **Task 3.1:** Создать структуру файлов
  - `src/webui_server.h` — заголовок класса
  - `src/webui_server.cpp` — реализация
  - `src/webui_html.h` — HTML шаблон (из макета)

- [ ] **Task 3.2:** Реализовать класс WebUI Server
  - Наследование от `etl_web_server_base_t`
  - HTTP сервер на порту 80
  - mDNS: `kitchenlight.local`
  - API endpoints:
    - `GET /` — главная страница
    - `GET /api/status` — статус устройства (вкл/выкл, яркость)
    - `POST /api/light` — управление светом (power, brightness)
    - `GET /api/config` — конфигурация устройства

- [ ] **Task 3.3:** Интеграция с WiFi settings
  - Использование общих настроек из ETL
  - Загрузка конфигурации hostname для mDNS

### Этап 4: Менеджер серверов
- [ ] **Task 4.1:** Создать менеджер переключения
  - Файл: `src/server_manager.h/cpp`
  - Функции:
    - `start_webui()` — запуск WebUI сервера
    - `start_wifi_setup()` — запуск WiFi Setup сервера
    - `stop_current()` — остановка текущего сервера
    - `switch_to_wifi_setup()` — переключение на WiFi Setup
    - `switch_to_webui()` — переключение на WebUI

- [ ] **Task 4.2:** Обновить main.cpp
  - Инициализация `etl::wifi::settings::init_config()` в setup()
  - Запуск WebUI сервера по умолчанию
  - Обработка симуляции 3 нажатий:
    ```cpp
    if (simulation_data.triple_press) {
        server_manager.switch_to_wifi_setup();
        simulation_data.triple_press = false;
    }
    ```
  - Обработка Factory Reset:
    ```cpp
    if (simulation_data.long_press_at_start) {
        wifi_server.reset_settings();
        server_manager.switch_to_wifi_setup();
    }
    ```

### Этап 5: Тестирование
- [ ] **Task 5.1:** Компиляция всех конфигураций
  - nodemcuv3 (ESP8266)
  - esp32c3
  - esp32-wroom-32u

- [ ] **Task 5.2:** Функциональное тестирование
  - [ ] WebUI доступен после старта
  - [ ] 3 нажатия переключают на WiFi Setup
  - [ ] WebUI останавливается при переключении
  - [ ] [Save & Reboot] переключает обратно на WebUI
  - [ ] Factory Reset сбрасывает настройки и запускает WiFi Setup
  - [ ] mDNS работает для обоих серверов

---

## План работ (последовательность)

```
1. Макет WebUI (docs\web-wifi\qwen-webui.001.html)
   ↓
2. Базовый класс etl_web_server_base_t (ETL библиотека)
   ↓
3. Обновление WiFi Setup Server (наследование от базового)
   ↓
4. WebUI Server (src/webui_server*)
   ↓
5. Server Manager (src/server_manager*)
   ↓
6. Интеграция в main.cpp
   ↓
7. Тестирование
```

---

## Примечания

### Настройки WiFi в ETL
- Настройки WiFi перемещены в библиотеку ETL
- Общий доступ через `etl::wifi::settings`
- `server_config_t` содержит:
  - hostname, ap_ssid, ap_password, wifi_ssid, wifi_password
  - port, update_interval

### Базовый класс для WebUI
- `etl_web_server_base_t` предоставляет общий интерфейс
- Клиенты могут наследоваться и реализовывать собственный UI
- `device_info_t` передаётся при запуске:
  - name — название устройства
  - description — описание
  - icon_svg — SVG иконка (опционально)

### Симуляция в тестовом проекте
- `simulation_t` в main.cpp для тестирования без реального железа
- `triple_press` — симуляция 3 нажатий кнопки
- `long_press_at_start` — симуляция долгого нажатия при старте
- `reset_wifi_on_start` — сброс настроек WiFi при старте