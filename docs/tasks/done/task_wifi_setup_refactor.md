# Интерфейсные улучшения для веб-интерфейса

У меня есть код для lib\ETLTest\etl_wifi_setup*, который служит для настройки сети, в которой будет работать устройства на базе ESP8266/ESP32. 

Необходимо выполнить добавить несколько улучшений и переработать код

PS. Я на всякий случай сохранил коммит "feat: task_wifi_refactor starting...." к которому можно будет откатиться, если что-то пойдет не так.

## Требования при работке

- Разрабатывать с учетом правила docs\rules\qwen_make_no_mistakes.md
- Этот код после отладки будет помещен в бибилиотеку ETL (https://github.com/JimorMarlow/ETL). Поэтому может использовать все неоходимое из нее и использовать общий стиль оформления.
- Код должен работать на ESP8266 и ESP32. Участка кода, специфические для каждого контроллера разделять с помощью нужных дефайн. Пример:
```cpp
// Для включения нужной wi-fi библиотеки
#pragma once
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#elif ESP32
  #include <WiFi.h>
#else
  #pragma message("ERROR: no Wi-Fi lib specified")
#endif
```
- После исправлений необходимо протестировать компиляцию всех конфигураций из platformio.ini
Нужно выполнить:
C:\Users\amber\.platformio\penv\Scripts\platformio.exe run -e <имя_конфигурации> -d c:\Projects\Arduino\ETLTest
- По запросу сделать текст для коммита, но самому в git не выкладывать
- Для обновления контеста можешь посмотреть историю коммитов
- Все изменения записывай для себя в docs\tasks\task_webui_darktheme_history.md, чтобы потом можно было продолжить

## Изучить историю разработки

В ходе работ над задачей docs\tasks\task_webui_darktheme.md были добавлены поля в структуру server_config_t, отвечающие за настройки интерфейса. История действий была записана в docs\tasks\task_webui_darktheme_history.md

## Вынести настройки интерфейса в отдельную структуру

- Из server_config_t вынести поля настройки интерфейса с отдельную структуру на том же уровне, что server_config_t
ui_config_t, реализовать ctear(), trace() для нее
// Настройки интерфейса
char language[WIFI_CONFIG_LANGUAGE_SIZE] = "en";  // Язык интерфейса (ISO 639-1)
bool dark_theme = false;                    // Тёмная тема
bool large_font = false;                    // Увеличенный шрифт
bool use_bold_values = false;               // Bold шрифт для ключевых значений

- etl::wifi::server_config_t load_config(); переделать на получение 
etl::optional<server_config_t> etl::wifi::server_config_t load_config()
которая будет возвращять wifi_cfg->get(), если он был проинициализирован, или nullptr, если не был
- в настройках web-интерфейса нужно будет не показывать секции
WiFi Setup / Настройки WiFi                         │ ← section-title (data-i18n="main_title")
Select Network / Выберите сеть                      │ ← section-title
Access Point Settings / Настройки точки доступа     │ ← ap-settings-section
[Apply AP Settings] 
если у нас нет настроек сети.
- server_setup должен хранить вместо 
server_config_t m_config;                   ///< Конфигурация WiFi
опциональное поле данных
std::optiopnal<server_config_t> m_config;                   ///< Конфигурация WiFi

Это позволит потом пользователю выбирать какие настройки нужны, а какие нет.

- добавить после настроек wifi - настройки интерфейса
etl::shared_ptr<etl::settings::data<etl::wifi::ui_config_t>> ui_cfg;
- сделать по аналогии с 
init_config
save_config
load_config

функции для сохранения настроек интерфейса 
init_ui_config
save_ui_config
load_ui_config

- Переименовать
init_config -> init_wifi_config
save_config -> save_wifi_config
load_config -> load_wifi_config

- В интерфейсе сделать показ контейнера ui-settings-container только если они были проинициализированны

- etl::wifi::server_setup должен в сохраннении и считывании учитывать какой блок данных присутствует
убрать из комментариев слово WiFi, так как теперь это общие настройки
/**
* @brief Сохранение настроек WiFi
* @return true при успешном сохранении
*/
virtual bool save_settings();

/**
* @brief Загрузка сохранённых настроек WiFi
* @return true если настройки загружены успешно
*/
virtual bool load_settings();

## Заложить на будущее еще две структуры с настройками

- Telegram Bot интеграция
- MQTT сервер - настройки

заложить туда пока комментарий TODO и сделать скелет. 

## Правило для полей структур, которые будут сохраняться с помощью etl::settings::data

- Поля во все структуры должны быть сделана по аналогии с server_config_t
- никаких динамических типов с выделением внутри памяти, только строки фиксированной длины, или стандартные типы, так как данные сохраняются бинарно во флеш-память

## Файл истории

Создай файл docs\tasks\task_wifi_setup_refactor_history.md и записывай туда все изменения, чтобы ты потом прочитать и восстановить контекст в следующей сессии

## Файл испавлений

Создай файл docs\tasks\fix_wifi_setup_refactor.md, сделай в нем заголовок и ссылку на этот документ.
Я буду записывать туда замечания и содержимой вывода для поиска ошибок и отладки

