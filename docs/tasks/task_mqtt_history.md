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
- ✅ d1_mini_lite - SUCCESS (8.53s)
- ✅ nodemcuv3 - SUCCESS (16.71s)
- ✅ esp32c3 - SUCCESS (21.47s)
- ✅ esp32-wroom-32u - SUCCESS (21.64s)

**Исправленные ошибки:**
1. Missing forward declaration для `etl::wifi::status_t` - добавлен `#include "etl_wifi.h"` в etl_mqtt.h
2. Область видимости типов в etl_mqtt.cpp - убраны префиксы `manager::` перед `status_t` и `config_t`

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
  - Связь с `light_control::data::app()` (заглушки для методов)

## Измененные файлы

1. `lib/ETLTest/etl_webui_settings.h` - добавлены поля mqtt_config_t
2. `lib/ETLTest/etl_webui_settings.cpp` - реализация методов mqtt_config_t
3. `lib/ETLTest/etl_mqtt.h` - добавлен #include "etl_wifi.h"
4. `lib/ETLTest/etl_mqtt.cpp` - исправлены области видимости типов
5. `src/secret.h.example` - создан шаблон
6. `src/secret.h` - создан файл секретов (локальный)
7. `src/light_mqtt.h` - создан менеджер света
8. `src/light_mqtt.cpp` - реализация менеджера света

## Примечания

- Архитектура разделена: общий MQTT менеджер в lib/ETLTest, специфичная реализация для света в src/
- secret.h НЕ должен попадать в репозиторий (добавлен в .gitignore)
- При отсутствии secret.h проект компилируется, но MQTT не подключается
- light_mqtt использует макрос HAS_MQTT_SECRETS для проверки наличия секретов
