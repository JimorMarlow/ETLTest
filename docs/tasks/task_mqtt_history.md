# История задачи: MQTT Менеджер

## Дата начала: 15 апреля 2026

## Выполненные этапы

### Этап 1-6: Базовая реализация (уже выполнена ранее)
- PubSubClient подключен в platformio.ini
- Создан etl_mqtt.h/cpp с полноценным интерфейсом
- Реализованы: connect/disconnect/publish/subscribe
- Интеграция с WiFi менеджером
- Система статусов и callback'ов
- Автопереподключение и обработка разрывов

### Этап 7: Конфигурация через систему настроек
**Выполнено:**
- Дополнен `mqtt_config_t` в etl_webui_settings.h полями:
  - `broker_host[64]` - адрес брокера
  - `broker_port` - порт (по умолчанию 1883)
  - `username[32]` - имя пользователя
  - `password[64]` - пароль
  - `client_id[32]` - идентификатор клиента
  - `enabled` - флаг включения
- Реализованы методы `clear()` и `trace()` для mqtt_config_t
- Добавлены setters/getters для всех полей
- Файлы: `lib/ETLTest/etl_webui_settings.h`, `lib/ETLTest/etl_webui_settings.cpp`

### Этап 8: Тестирование компиляции
**Результат:** Все 4 конфигурации скомпилированы успешно
- ✅ d1_mini_lite - SUCCESS
- ✅ nodemcuv3 - SUCCESS
- ✅ esp32c3 - SUCCESS
- ✅ esp32-wroom-32u - SUCCESS

### Этап 9: Интеграция с wqtt.ru для управления светом
**Выполнено:**
- Создан `src/secret.h.example` - шаблон с настройками подключения
- Создан `src/secret.h` - файл с секретами (локальный, не коммитится)
  - Настройки: m1.wqtt.ru:16208
  - Макрос `HAS_MQTT_SECRETS` для условной компиляции
- Создан `src/light_mqtt.h` - менеджер для управления светом
- Создан `src/light_mqtt.cpp` - реализация интеграции
  - Подписка на топики:
    - `/home/guest/light/kitchen_workarea/set`
    - `/home/guest/light/kitchen_workarea/brightness/set`
  - Публикация в топики:
    - `/home/guest/light/kitchen_workarea/state`
    - `/home/guest/light/kitchen_workarea/brightness/state`
  - Связь с `light_control::data::app()`:
    - Подписка на изменения через `app().subscribe(etl::settings::sender_id::mqtt, callback)`
    - При получении MQTT сообщения - `app().set(updated_data, etl::settings::sender_id::mqtt)`
    - При изменении из webui - автоматическая публикация в MQTT

### Дополнительное исправление: WiFi менеджер как shared_ptr
**Проблема:** Требование Этапа 3 не было выполнено полностью - использовался сырой указатель вместо shared_ptr
**Решение:**
- `etl_mqtt.h`: заменил `etl::wifi::manager* m_wifi_manager` на `etl::shared_ptr<etl::wifi::manager> m_wifi_manager`
- `etl_mqtt.cpp`: метод `set_wifi_manager()` теперь хранит весь shared_ptr
- `stop()` теперь обнуляет `m_wifi_manager = nullptr`

### Дополнительное исправление: Сброс WiFiClient
**Проблема:** При переключении серверов webui (content <-> settings) WiFi останавливается и `WiFiClient` остаётся в "сломанном" состоянии
**Решение:**
- В `stop()`: `m_wifi_client.stop()` и пересоздание `m_wifi_client = WiFiClient()`
- В `begin()`: очистка `m_wifi_client` перед созданием `PubSubClient`
- `m_mqtt_client` теперь уничтожается в `stop()` и пересоздаётся в `begin()`

### Интеграция в main.cpp
**Выполнено:**
- Добавлен `light_mqtt_mgr` - глобальный MQTT менеджер для света
- Функция `start_light_mqtt()` - запускает MQTT при наличии WiFi
- В `loop()`: вызов `light_mqtt_mgr->tick()` для обработки сообщений
- Автоматический перезапуск MQTT при переключении серверов webui
- В `light_webui_mgr.h` добавлены методы `is_wifi_connected()` и `get_wifi_manager()`

## Измененные файлы

1. `lib/ETLTest/etl_webui_settings.h` - добавлены поля mqtt_config_t
2. `lib/ETLTest/etl_webui_settings.cpp` - реализация методов mqtt_config_t
3. `lib/ETLTest/etl_mqtt.h` - исправлен тип m_wifi_manager на shared_ptr, добавлен include etl_wifi.h
4. `lib/ETLTest/etl_mqtt.cpp` - исправлен scope типов, shared_ptr для wifi_mgr, сброс WiFiClient
5. `src/light_mqtt.h` - менеджер для управления светом (полная реализация)
6. `src/light_mqtt.cpp` - интеграция с wqtt.ru и light_control::data::app()
7. `src/secret.h` (локальный, не коммитится)
8. `src/secret.h.example` (шаблон для пользователей)
9. `src/main.cpp` - интеграция light_mqtt_mgr, запуск при WiFi, tick()
10. `src/light_webui_mgr.h` - добавлены is_wifi_connected() и get_wifi_manager()

## Примечания

- Архитектура разделена: общий MQTT менеджер в lib/ETLTest, специфичная реализация для света в src/
- secret.h НЕ должен попадать в репозиторий (добавлен в .gitignore)
- При отсутствии secret.h проект компилируется, но MQTT не подключается
- light_mqtt использует макрос HAS_MQTT_SECRETS для проверки наличия секретов
- Подписка на `light_control::data::app()` с `etl::settings::sender_id::mqtt`:
  - Если изменения пришли НЕ от mqtt (например из webui) - публикация в MQTT
  - Если изменения пришли от MQTT - применяются к устройству и webui обновляется
