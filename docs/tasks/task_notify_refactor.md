# Задача: рефакторинг etl::settings::notify

Статус: план, работа не начата. Создано 2026-11-24.

## Требования к разработке

- После каждого этапа собирать все окружения ETL (esp8266, esp32c3, esp32-wroom-32u) и все
  шесть конфигураций ETLTest (см. `AGENTS.md` проекта).
- Загрузку на плату и монитор выполнять только по явному запросу пользователя.
- В git ничего не коммитить и не пушить; текст коммита готовить по запросу.
- Все изменения и замеры записывать в `docs/tasks/task_notify_refactor_history.md`, чтобы
  работу можно было продолжить в любой момент.
- Пункты `- [ ]` закрывать только после того, как пользователь проверит результат.

## Цель

Переделать механизм нотификаций `etl::settings::notify` так, чтобы он:

- не использовал `std::function` и не выделял память под каждую подписку;
- позволял нескольким компонентам подписаться на одно и то же событие;
- не имел неопределённого поведения при отписке из собственного callback и при повторной подписке;
- был защищён от рекурсивной рассылки;
- умел передавать полезную нагрузку события, чтобы `wifi::manager` и `mqtt::manager` пользовались
  одной шиной, а не вторым параллельным механизмом.

## Карта кода (что смотреть в первую очередь)

ETL:

| Место | Что там |
|---|---|
| `src/etl/etl_settings.h:72-151` | класс `notify`: массив, subscribe/unsubscribe/is_subscribed, notify_changed |
| `src/etl/etl_settings.h:155` | `data<T>` — настройки, наследует `notify` |
| `src/etl/etl_settings.h:252-261` | `set(data, source, update_now)` — рассылка после записи |
| `src/etl/etl_settings.cpp:7-32` | `to_string(sender_id)` — синхронизируется вручную |
| `src/etl/etl_wifi.h:105,338` | `manager : notify`, собственный массив `m_status_callbacks` |
| `src/etl/etl_wifi.cpp:420-443,581-590` | `subscribe_status` / `unsubscribe_status` / `notify_status_change` |
| `src/etl/etl_mqtt.h:295`, `etl_mqtt.cpp:472-501` | то же дублирование в mqtt |
| `src/etl/etl_webui_base.h:335,391,403` | `on_wifi_status_changed`, `m_wifi_subscriber_id`, `s_instance_counter` |
| `src/etl/etl_webui_base.cpp:486-488,520` | генерация подписчика `user1 + (n % 3)` |
| `src/etl/etl_test.cpp:1150-1282` | эталон семантики: `test_settings()` и тест `connection_state` |

ETLTest:

| Место | Что там |
|---|---|
| `src/light_webui.cpp:124,160-175` | подписка/отписка веб-сервера на настройки |
| `src/light_webui.h:242` | `bool m_subscribed` |
| `src/light_mqtt.cpp:81,92-102,127` | `subscribe_status(user1)`, подписка на настройки, отписка |
| `src/light_amqtt.cpp:82,93-103,128` | то же в форке |
| `src/etl_amqtt.{h,cpp}` | локальная копия модуля mqtt: `subscribe_status`, `m_status_callbacks`, `notify_status_change` |
| `src/main.cpp:145-154` | подписка «системы» на настройки |

## Подтверждённые дефекты текущей реализации

1. **Один подписчик на `sender_id`.** Второй `subscribe()` молча затирает первый
   (`etl_settings.h:97-101`). Отсюда хак `user1 + (n % 3)` (`etl_webui_base.cpp:486-488`) и
   коллизия с `light_mqtt.cpp:81`, который тоже занимает `user1`: сегодня в ETLTest они
   затирают друг друга.
2. **`std::function` в куче** на каждую подписку (`etl_settings.h:101`) — нарушение правил ETL
   (STL и лишние аллокации) плюс фрагментация кучи на ESP8266.
3. **UB при отписке из своего callback.** `unsubscribe()` делает `delete` (`107-118`), пока
   `notify_changed` выполняет этот же объект (`147`). То же при повторной `subscribe()` того же id.
4. **Нет защиты от рекурсии.** `set()` внутри callback запускает рассылку заново (`130-149`).
5. **Плата за неиспользуемое:** массив из 10 указателей и `virtual ~notify()` в каждом
   `data<T>` даже без подписчиков (`76,79`); слот `broadcast` (индекс 0) не используется.
6. **Дублирование шины** в wifi (`etl_wifi.h:338`, `etl_wifi.cpp:420-443`) и mqtt
   (`etl_mqtt.cpp:472-501`) — из-за отсутствия payload в `notify`.
7. **Порядок вызовов** = порядок enum, а не порядок подписки (`134-149`), и это нигде не описано.
8. **Мелочи:** `to_string()` руками; комментарий «Статический массив» (`75`) неверен;
   `set()` уведомляет подписчиков даже при неудачной записи на флеш (`252-261`);
   источник не может получить своё же событие.

## Текущее состояние MQTT (важно для планирования)

MQTT в ETL ещё не доведён до рабочего состояния:

- `etl_mqtt` построен на PubSubClient: библиотека устарела, при сборке даёт предупреждения,
  есть проблемы и в работе;
- планировался переход на `async-mqtt-client` (<https://github.com/marvinroger/async-mqtt-client>),
  но он не завершён, поэтому отладочный код в ETLTest временно закрыт `USE_WIFI_UI_SERVER`;
- после обновления системы нотификаций планируется отказаться от PubSubClient и перейти на
  асинхронный клиент (конкретный выбор ещё не сделан, будем решать позже).

Следствие для этой задачи: **весь MQTT-код — черновик, совместимость с ним не поддерживается.**
`etl_mqtt.*`, `src/etl_amqtt.*`, `src/light_mqtt.*`, `src/light_amqtt.*` можно менять как угодно
под новую шину, вплоть до удаления. Пункты ниже про mqtt — не про сохранение старого поведения,
а про то, чтобы новая шина сразу подошла будущему асинхронному клиенту.

## Целевой API (согласован с пользователем 2026-11-24)

POD-callback и handle подписки:

```cpp
namespace etl::settings {

    // Дескриптор подписки вместо занятия слота sender_id
    struct subscription {
        uint8_t slot = 0xFF;
        uint8_t generation = 0;   // защита от повторного unsubscribe после переиспользования слота
    };

    // Источник события — только метка; слотов больше нет.
    // Каждое изменение рассылается всем подписчикам, поэтому отдельного режима
    // broadcast нет: источник указывается всегда.
    enum class sender_id : uint8_t {
        system,    // главное приложение (датчики, автоматика)
        webui,     // веб-интерфейс (обновление от пользователя)
        setup,     // сервер настроек
        view,      // пользовательский интерфейс
        wifi,      // менеджер WiFi
        mqtt,      // MQTT
        ntp        // синхронизация времени по NTP после подключения к WiFi
    };

    template <typename TEvent, uint8_t MaxSubscribers = 4>
    class notify {
    public:
        using callback_t = void (*)(void* ctx, TEvent event);

        subscription subscribe(void* ctx, callback_t fn);
        bool unsubscribe(subscription s);

    protected:
        void notify_changed(TEvent event);   // вызывает ВСЕХ, включая источник
    };

} // namespace etl::settings
```

Семантика, которую надо зафиксировать в тестах:

- `sender_id` — только метка источника: `count`, `user1..user3`, `broadcast` и слоты удаляются,
  список подписчиков динамический, свободные слоты больше не нужны;
- `broadcast` не нужен: он существовал только чтобы обойти правило «источник не получает своё
  событие», а в новой модели это правило снято — всем уходит каждое событие, фильтрует клиент;
- `sender_id::ntp` — обычный источник событий синхронизации времени по NTP;
- источник события тоже получает уведомление, фильтрация — на стороне клиента
  (`if (source == sender_id::webui) return;`);
- если когда-нибудь понадобится обновить данные без рассылки (например, при первой загрузке),
  это делается отдельным явным методом `set_silent()`, а не значением enum;
- `settings::data<T>` — это `notify<sender_id>`, `wifi::manager` — `notify<etl::wifi::status_t>`;
- capacity подписчиков — шаблонный параметр, чтобы не платить за 10 слотов в каждом объекте;
- отписка и повторная подписка внутри callback безопасны (отложенное удаление);
- рекурсивный `set()` из callback не размножает рассылку бесконечно.

Альтернативы: (а) интерфейс `observer { virtual void changed(TEvent) = 0; }` — без кучи и без
контекста; (б) `std::function` по значению — **отклонено**: совместимость со старым API не нужна,
проектов пока мало, переделываем всех сразу.

## Этапы работ

### Этап 0. Подготовка (полдня)

- [ ] Зафиксировать текущее поведение: прогнать `test_settings()` на всех трёх окружениях ETL,
      записать вывод в history.
- [ ] Записать в history текущие RAM/Flash по трём env (база для сравнения после рефакторинга).
- [ ] Согласовать с пользователем целевой API и ответы на открытые вопросы (см. ниже).

### Этап 1. Убрать `std::function` и кучу

- [ ] В `notify` заменить `change_callback*` на POD-пару `{void* ctx; void (*fn)(void*, sender_id);}`.
- [ ] Убрать `new`/`delete` из `subscribe()`/`unsubscribe()`/деструктора.
- [ ] Перегрузку `subscribe(sender_id, std::function)` **не оставлять**: решено, что совместимость
      со старым API не нужна, потребители мигрируют сразу.
- [ ] Сборка трёх env ETL, тест settings зелёный, RAM не выросла.

### Этап 2. Безопасная рассылка

- [ ] Отложенное удаление: во время обхода помечать слоты, чистить после цикла.
- [ ] Флаг «идёт рассылка»; `set()` из callback — лог + запрет (или очередь отложенных событий).
- [ ] Новые тесты: отписка из своего callback; повторная `subscribe()` в callback; рекурсивный `set()`.
- [ ] Сборка трёх env.

### Этап 3. Разделить источник и подписчика

- [ ] `subscribe()` возвращает `subscription` (handle), `unsubscribe(handle)`.
- [ ] Снять ограничение «один подписчик на id»: несколько подписчиков на одно событие.
- [ ] Обновить `etl_test.cpp`: сценарии с двумя подписчиками на одно имя.
- [ ] Сборка трёх env.

### Этап 4. Обобщить по payload и убрать дублирование

- [ ] `notify<TEvent, MaxSubscribers>`; `data<T>` = `notify<sender_id>`, `wifi::manager` =
      `notify<etl::wifi::status_t>`, `mqtt::manager` — по той же схеме.
- [ ] Удалить `subscribe_status`/`unsubscribe_status`/`m_status_callbacks`/`notify_status_change`
      из `etl_wifi.*` и `etl_mqtt.*`.
- [ ] `etl_webui_base`: убрать `s_instance_counter` и `user1 + (n % 3)`, хранить `subscription`.
- [ ] Удалить `sender_id::count`, `sender_id::user1..user3` и `sender_id::broadcast`; добавить
      `sender_id::ntp` (синхронизация времени по NTP после подключения к WiFi).
- [ ] `to_string(sender_id)` перевести на X-macro: один список `ETL_SENDER_ID_LIST(X)` задаёт и
      enum, и строки, чтобы они не расходились.
- [ ] Сборка трёх env, `test_all()` на плате по запросу пользователя.

### Этап 5. Версия и выпуск ETL

- [ ] `src/etl/etl_version.h`: 1.1.5 → **2.0.0** (ломающее изменение), затем `python sync_version.py`.
- [ ] Раздел в `README.md` в существующем стиле (русский): явно перечислить удалённое
      (`user1..user3`, `count`, `broadcast`, слоты, `std::function`) и добавленное
      (`sender_id::ntp`, handle, X-macro для `to_string`).
- [ ] `library.json`: `headers` и `build.srcFilter` синхронизированы, если менялся состав файлов.
- [ ] Три env + `test_all` на подключённой плате (спросить пользователя).

### Этап 6. Миграция ETLTest

- [ ] `src/version.h`: 1.2.1 → **2.0.0** — версия ETLTest держится на уровне ETL (он компаньон
      библиотеки), затем `python sync_version.py` и обновление `package.json`.
- [ ] `platformio.ini`: `ETL@^1.1.4` → `^2.0.0`.
- [ ] `src/light_webui.cpp:160-175` — `subscribe(cb)` вместо `subscribe(sender_id::webui, cb)`,
      handle в `light_webui.h:242` как `etl::optional<subscription>`.
- [ ] `src/light_webui.cpp:124` — `unsubscribe(handle)`.
- [ ] `src/light_mqtt.cpp:92-102,127` — то же; новый guard `source == sender_id::mqtt → return`.
- [ ] `src/light_mqtt.cpp:81` — `subscribe_status` больше нет, подписка на статус через общую шину.
- [ ] `src/main.cpp:145-154` — `subscribe(cb)` с фильтром источника.
- [ ] `src/etl_amqtt.{h,cpp}` — удалить форк: mqtt-код в ETLTest считается черновиком
      (см. «Текущее состояние MQTT»), совместимость не нужна, модуль переписывается под
      будущий асинхронный клиент вместе с библиотечным.
- [ ] Шесть конфигураций собираются; при необходимости — проверка на трёх платах по запросу.

### Этап 7. Moonshine

- [ ] `platformio.ini`: `ETL@^1.1.5` → `^2.0.0`.
- [ ] `src/version.h`: 1.7.3 → **1.8.0** (MINOR). В Moonshine правок почти нет; если их не
      появится, мажор до 2.0.0 был бы обманом — 1.8.0 честно означает «обновили зависимость».
      Если по ходу всё же придётся править его код — поднять до 2.0.0, решить по факту.
- [ ] Проверить `set_wifi_manager()` (`web-ui.cpp:706,717`) и `on_wifi_status_changed` — если
      сигнатуры не менялись, правок нет.
- [ ] Сборка `esp32-wroom-38u`; на плате — по запросу.

## Таблица изменений ETLTest (чек-лист миграции)

| Файл | Строки | Действие |
|---|---|---|
| `platformio.ini` | lib_deps ETL | `^1.1.4` → `^2.0.0` |
| `light_webui.cpp` | 160-175 | `subscribe(handle, cb)`, guard по источнику |
| `light_webui.cpp` | 124 | `unsubscribe(handle)` |
| `light_webui.h` | 242 | `bool m_subscribed` → `etl::optional<subscription>` |
| `light_mqtt.cpp` | 81 | `subscribe_status` → общая шина |
| `light_mqtt.cpp` | 92-102 | `subscribe(cb)`, guard `source == mqtt → return` |
| `light_mqtt.cpp` | 127 | `unsubscribe(handle)` |
| `light_amqtt.cpp` | 82, 93-103, 128 | удалить вместе с форком (MQTT — черновик) |
| `etl_amqtt.h` | 344 | удалить: форк не нужен, модуль будет переписан |
| `main.cpp` | 145-154 | `subscribe(cb)` + фильтр |

## Критерии готовности

- `test_settings()` и новый тест нескольких подписчиков зелёные на всех трёх env ETL.
- Отписка из своего callback, повторная подписка и рекурсивный `set()` не дают UB и не виснут.
- RAM хотя бы не выросла относительно замеров этапа 0.
- ETLTest: собираются все шесть конфигураций, `test_all()` проходит на плате.
- Moonshine: собирается `esp32-wroom-38u`, переключение setup/content работает.
- Нет регрессии «эха» webui ↔ mqtt (проверить, когда MQTT включён; сейчас код закрыт `USE_WIFI_UI_SERVER`).

## Риски

- **Смена семантики:** источник теперь получает своё событие. В webui/mqtt это может включить
  «эхо» — обязательна проверка на устройстве и, при необходимости, guard по источнику в клиентах.
- **Удаление `user1..user3`** ломает клиентов, которые их используют (ETLTest — точно).
- **Порядок вызовов** меняется с «по enum» на «по подписке»; если кто-то полагался на порядок — увидит разницу.
- **MQTT — черновик:** `etl_mqtt` на PubSubClient и форк `etl_amqtt` не в рабочем состоянии,
  совместимость не поддерживаем. Риск рассинхрона форка снимается его удалением; MQTT-код
  переписывается отдельной задачей под асинхронный клиент.
- **`shared_ptr` и время жизни подписчиков:** handle не спасает от висячего `this`, если объект
  уничтожен без `unsubscribe` — это по-прежнему ответственность клиента.

## Принятые решения (2026-11-24)

- [x] Handle: `struct subscription { uint8_t slot; uint8_t generation; }` — слот плюс поколение.
- [x] Совместимость со `std::function` не нужна: перегрузку не оставляем, потребителей переделываем сразу.
- [x] `sender_id::user1..user3` и `count` — удалить; вместо них добавить `sender_id::ntp`
      (обычный источник, без пометок «на будущее»).
- [x] `sender_id::broadcast` — удалить: в новой модели рассылка и так идёт всем подписчикам,
      фильтрация по источнику — на клиенте. Для обновления без рассылки — отдельный `set_silent()`.
- [x] `to_string(sender_id)` — генерировать из X-macro списка `ETL_SENDER_ID_LIST(X)`, а не
      писать switch руками.
- [x] Версии: ETL 1.1.5 → **2.0.0**; ETLTest 1.2.1 → **2.0.0** (компаньон, версии синхронны).
- [x] Moonshine 1.7.3 → **1.8.0** (MINOR: правок в его коде почти нет).
- [x] KitchenLight 0.1.8 → **0.2.0** (MINOR: миграция платформы и ETL без смены функционала);
      подробности — в `KitchenLight/docs/tasks/task_upgrade_to_etl2x.md`.
- [x] Форк `etl_amqtt` — удалить, MQTT будет переработан (см. «Текущее состояние MQTT»).

## Проверка (команды)

```powershell
# ETL — все окружения
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d S:\Projects\Arduino\ETL

# ETLTest — шесть конфигураций
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -d S:\Projects\Arduino\ETLTest

# Moonshine
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-38u -d S:\Projects\Arduino\Moonshine
```
