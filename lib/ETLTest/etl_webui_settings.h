#pragma once
/**
 * @file etl_webui_settings.h
 * @brief Конфигурационные структуры для WebUI серверов
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 *
 * Особенности:
 * - Содержит структуры для хранения настроек WiFi, UI, Telegram, MQTT
 * - Использует фиксированные массивы char для корректного бинарного сохранения
 * - Предназначен для использования с etl::settings::data
 */

#include <Arduino.h>
#include <etl/etl_memory.h>
#include <etl/etl_optional.h>

#if defined(ESP8266) || defined(ESP32)

// Размеры буферов для строк в server_config_t
#define WIFI_CONFIG_HOSTNAME_SIZE     32
#define WIFI_CONFIG_SSID_SIZE         32
#define WIFI_CONFIG_PASSWORD_SIZE     64
#define WIFI_CONFIG_LANGUAGE_SIZE     3

// Список доступных языков интерфейса (ISO 639-1)
static const char* const WIFI_SETUP_LANGUAGES[] PROGMEM = {
    "en",
    "ru"
};
static const size_t WIFI_SETUP_LANGUAGE_COUNT = sizeof(WIFI_SETUP_LANGUAGES) / sizeof(WIFI_SETUP_LANGUAGES[0]);

namespace etl
{
    namespace webui
    {
        /**
         * @brief Информация об устройстве
         *
         * НЕ сохраняется в постоянной памяти, передаётся отдельно при запуске сервера.
         * Использует String для поддержки произвольных размеров (особенно для SVG иконки).
         */
        struct device_info_t
        {
            String name = "ESP Device v1.0.0";          // Название устройства
            String description = "Smart home device based on ESP8266/ESP32";  // Описание
            String icon_svg = "";                       // SVG иконка устройства (опционально)

            /**
             * @brief Очистка информации об устройстве
             */
            void clear() {
                name.clear();
                description.clear();
                icon_svg.clear();
            }

            /**
             * @brief Оператор присвоения
             * @param other Другой объект device_info_t
             * @return Ссылка на текущий объект
             */
            device_info_t& operator=(const device_info_t& other) {
                if (this != &other) {
                    name = other.name;
                    description = other.description;
                    icon_svg = other.icon_svg;
                }
                return *this;
            }

            /**
             * @brief Вывод информации об устройстве в Serial
             */
            void trace() const {
                Serial.println(F("--- device info ---"));
                Serial.printf("name            = %s\n", name.c_str());
                Serial.printf("description     = %s\n", description.c_str());
                Serial.printf("icon_svg        = %s\n", icon_svg.c_str());
            }
        };

        /**
         * @brief Статус подключения к WiFi
         */
        enum class connection_status_t : uint8_t
        {
            disconnected,     // Не подключено
            connecting,       // В процессе подключения
            connected,        // Подключено к WiFi
            ap_mode,          // Режим точки доступа
            error             // Ошибка подключения
        };
        /**
         * @brief Конфигурация WiFi сервера
         *
         * Содержит параметры для точки доступа и внешней сети.
         * Сохраняется в энергонезависимой памяти через FileData.
         * Использует фиксированные массивы char для корректного бинарного сохранения.
         */
        struct server_config_t
        {
            // Конфигурация сети
            char hostname[WIFI_CONFIG_HOSTNAME_SIZE] = "espdevice";
            char ap_ssid[WIFI_CONFIG_SSID_SIZE] = "ESP_Device_AP";
            char ap_password[WIFI_CONFIG_PASSWORD_SIZE] = "password123";
            char wifi_ssid[WIFI_CONFIG_SSID_SIZE] = "";
            char wifi_password[WIFI_CONFIG_PASSWORD_SIZE] = "";
            uint16_t port = 80;                         // Порт веб-сервера
            uint32_t update_interval = 500;             // Интервал обновления данных (мс)

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear();

            /**
             * @brief Вывод конфигурации в Serial
             */
            void trace() const;

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

        /**
         * @brief Конфигурация интерфейса пользователя
         *
         * Содержит параметры настройки веб-интерфейса.
         * Сохраняется в энергонезависимой памяти через FileData.
         * Использует фиксированные массивы char для корректного бинарного сохранения.
         * При реализации придерживаться соглашений из docs\etl_wifi_setup.md
         */
        struct ui_config_t
        {
            char language[WIFI_CONFIG_LANGUAGE_SIZE] = "en";  // Язык интерфейса (ISO 639-1)
            bool dark_theme = false;                    // Тёмная тема
            bool large_font = false;                    // Увеличенный шрифт
            bool use_bold_values = false;               // Bold шрифт для ключевых значений

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear();

            /**
             * @brief Вывод конфигурации в Serial
             */
            void trace() const;

            // Setters
            void set_language(const String& value);
            void set_dark_theme(bool value);
            void set_large_font(bool value);
            void set_use_bold_values(bool value);

            // Getters
            String get_language() const;
            bool is_dark_theme() const;
            bool is_large_font() const;
            bool is_use_bold_values() const;
        };

        /**
         * @brief Конфигурация Telegram бота
         *
         * Содержит параметры для интеграции с Telegram.
         * Сохраняется в энергонезависимой памяти через FileData.
         *
         * @note TODO: Реализовать функционал Telegram бота
         */
        struct telegram_config_t
        {
            // TODO: Добавить поля для конфигурации Telegram бота
            // Например:
            // char bot_token[64] = "";
            // char chat_id[32] = "";
            // bool enabled = false;

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear();

            /**
             * @brief Вывод конфигурации в Serial
             */
            void trace() const;
        };

        /**
         * @brief Конфигурация MQTT сервера
         *
         * Содержит параметры для подключения к MQTT брокеру.
         * Сохраняется в энергонезависимой памяти через FileData.
         */
        struct mqtt_config_t
        {
            char broker_host[64] = "";                ///< Адрес брокера
            uint16_t broker_port = 1883;              ///< Порт брокера
            char username[32] = "";                   ///< Имя пользователя
            char password[64] = "";                   ///< Пароль
            char client_id[32] = "esp_mqtt";          ///< Идентификатор клиента
            bool enabled = false;                     ///< Флаг включения MQTT

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear();

            /**
             * @brief Вывод конфигурации в Serial
             */
            void trace() const;

            // Setters
            void set_broker_host(const String& value);
            void set_broker_port(uint16_t value);
            void set_username(const String& value);
            void set_password(const String& value);
            void set_client_id(const String& value);
            void set_enabled(bool value);

            // Getters
            String get_broker_host() const;
            uint16_t get_broker_port() const;
            String get_username() const;
            String get_password() const;
            String get_client_id() const;
            bool is_enabled() const;
        };

        /**
         * @brief Результат сканирования WiFi сети
         */
        struct scan_result_t
        {
            String ssid;                                // SSID сети
            int32_t rssi;                               // Уровень сигнала (dBm)
            String encryption;                          // Тип шифрования (WPA2, WPA, Open)
            uint8_t channel;                            // Канал
            bool connected = false;                     // Флаг: подключено к этой сети
        };

        /**
         * @brief Значение текущих настроек WiFi
         */
        namespace settings
        {
            /**
             * @brief Установить значения подключения к точками доступа по умолчанию и считать данные
             * @param default_cfg Конфигурация WiFi сервера по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_wifi_config(const etl::webui::server_config_t& default_cfg, bool reset_to_default = false);

            /**
             * @brief Установить значения подключения к точками доступа
             * @param cfg Конфигурация WiFi сервера
             */
            bool save_wifi_config(const etl::webui::server_config_t& cfg);

            /**
             * @brief Считать текущие значения подключения к точками доступа
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<etl::webui::server_config_t> load_wifi_config();

            /**
             * @brief Инициализация настроек интерфейса
             * @param default_cfg Конфигурация интерфейса по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_ui_config(const etl::webui::ui_config_t& default_cfg, bool reset_to_default = false);

            /**
             * @brief Сохранить настройки интерфейса
             * @param cfg Конфигурация интерфейса
             */
            bool save_ui_config(const etl::webui::ui_config_t& cfg);

            /**
             * @brief Загрузить настройки интерфейса
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<etl::webui::ui_config_t> load_ui_config();

            /**
             * @brief Инициализация настроек Telegram бота
             * @param default_cfg Конфигурация Telegram бота по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_telegram_config(const etl::webui::telegram_config_t& default_cfg, bool reset_to_default = false);

            /**
             * @brief Сохранить настройки Telegram бота
             * @param cfg Конфигурация Telegram бота
             */
            bool save_telegram_config(const etl::webui::telegram_config_t& cfg);

            /**
             * @brief Загрузить настройки Telegram бота
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<etl::webui::telegram_config_t> load_telegram_config();

            /**
             * @brief Инициализация настроек MQTT
             * @param default_cfg Конфигурация MQTT по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_mqtt_config(const etl::webui::mqtt_config_t& default_cfg, bool reset_to_default = false);

            /**
             * @brief Сохранить настройки MQTT
             * @param cfg Конфигурация MQTT
             */
            bool save_mqtt_config(const etl::webui::mqtt_config_t& cfg);

            /**
             * @brief Загрузить настройки MQTT
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<etl::webui::mqtt_config_t> load_mqtt_config();
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_settings: no implementation for this platform")
#endif
