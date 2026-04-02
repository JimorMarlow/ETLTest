/**
 * @file etl_webui_settings.cpp
 * @brief Реализация конфигурационных структур для WebUI
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui_settings.h"
#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"

namespace etl
{
    namespace webui
    {
        namespace settings
        {
            // WiFi настройки
            const String    wifi_data_path = "/settings/wifi.cfg";
            const uint16_t  wifi_data_update_delay = 0;  // 0ms - Immediately update
            server_config_t default_wifi_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::server_config_t>> wifi_cfg;

            // UI настройки
            const String    ui_data_path = "/settings/ui.cfg";
            const uint16_t  ui_data_update_delay = 0;  // 0ms - Immediately update
            ui_config_t     default_ui_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::ui_config_t>> ui_cfg;

            // Telegram настройки
            const String    telegram_data_path = "/settings/telegram.cfg";
            const uint16_t  telegram_data_update_delay = 0;  // 0ms - Immediately update
            telegram_config_t default_telegram_cfg;          // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::telegram_config_t>> telegram_cfg;

            // MQTT настройки
            const String    mqtt_data_path = "/settings/mqtt.cfg";
            const uint16_t  mqtt_data_update_delay = 0;  // 0ms - Immediately update
            mqtt_config_t   default_mqtt_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::mqtt_config_t>> mqtt_cfg;

            // ============================================================================
            // WiFi настройки
            // ============================================================================

            /**
             * @brief Установить значения подключения к точками доступа по умолчанию и считать данные
             * @param cfg Конфигурация WiFi сервера по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_wifi_config(const etl::webui::server_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_wifi_config()"));

                if(etl::little_fs::begin())
                {
                    // Создание директории для файла настроек
                    etl::little_fs::create_dir(settings::wifi_data_path);
                }

                // Сохранение настроек в постоянной памяти
                if(!wifi_cfg)
                {
                    wifi_cfg = etl::make_shared<etl::settings::data<etl::webui::server_config_t>>(settings::wifi_data_path, settings::wifi_data_update_delay, default_cfg);
                    bool result = wifi_cfg->init();
                    Serial.print(F("[wifi::settings] init_wifi_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting to default ..."));
                        auto loaded_cfg = wifi_cfg->get();
                        Serial.println(F("[wifi::settings] loaded from memory:"));
                        loaded_cfg.trace();

                        // Выполняем сброс: устанавливаем значения по умолчанию и сохраняем
                        wifi_cfg->set(default_cfg);
                        bool reset_result = wifi_cfg->save();

                        Serial.print(F("[wifi::settings] reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                        if(reset_result)
                        {
                            default_cfg.trace();
                        }
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_wifi_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Установить значения подключения к точками доступа
             * @param cfg Конфигурация WiFi сервера
             */
            bool save_wifi_config(const server_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_wifi_config()"));

                if(wifi_cfg)
                {
                    wifi_cfg->set(cfg);
                    bool result = wifi_cfg->save();
                    Serial.print(F("[wifi::settings] save_wifi_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_wifi_config() error: wifi_cfg not inited"));
                return false;
            }

            /**
             * @brief Считать текущие значения подключения к точками доступа
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<server_config_t> load_wifi_config()
            {
                Serial.println(F("[wifi::settings] load_wifi_config()"));

                if(wifi_cfg)
                {
                    server_config_t cfg = wifi_cfg->get();
                    Serial.println(F("[wifi::settings] load_wifi_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_wifi_config(): wifi_cfg not inited, returning empty optional"));
                    return {};
                }
            }

            // ============================================================================
            // UI настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек интерфейса
             * @param default_cfg Конфигурация интерфейса по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_ui_config(const etl::webui::ui_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_ui_config()"));

                if(etl::little_fs::begin())
                {
                    // Создание директории для файла настроек
                    etl::little_fs::create_dir(settings::ui_data_path);
                }

                // Сохранение настроек в постоянной памяти
                if(!ui_cfg)
                {
                    ui_cfg = etl::make_shared<etl::settings::data<etl::webui::ui_config_t>>(settings::ui_data_path, settings::ui_data_update_delay, default_cfg);
                    bool result = ui_cfg->init();
                    Serial.print(F("[wifi::settings] init_ui_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting UI to default ..."));
                        auto loaded_cfg = ui_cfg->get();
                        Serial.println(F("[wifi::settings] UI loaded from memory:"));
                        loaded_cfg.trace();

                        // Выполняем сброс: устанавливаем значения по умолчанию и сохраняем
                        ui_cfg->set(default_cfg);
                        bool reset_result = ui_cfg->save();

                        Serial.print(F("[wifi::settings] UI reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                        if(reset_result)
                        {
                            default_cfg.trace();
                        }
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_ui_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки интерфейса
             * @param cfg Конфигурация интерфейса
             */
            bool save_ui_config(const ui_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_ui_config()"));

                if(ui_cfg)
                {
                    ui_cfg->set(cfg);
                    bool result = ui_cfg->save();
                    Serial.print(F("[wifi::settings] save_ui_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_ui_config() error: ui_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки интерфейса
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<ui_config_t> load_ui_config()
            {
                Serial.println(F("[wifi::settings] load_ui_config()"));

                if(ui_cfg)
                {
                    ui_config_t cfg = ui_cfg->get();
                    Serial.println(F("[wifi::settings] load_ui_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_ui_config(): ui_cfg not inited, returning empty optional"));
                    etl::optional<ui_config_t> empty;
                    return empty;
                }
            }

            // ============================================================================
            // Telegram настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек Telegram бота
             * @param default_cfg Конфигурация Telegram бота по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_telegram_config(const etl::webui::telegram_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_telegram_config()"));

                if(etl::little_fs::begin())
                {
                    etl::little_fs::create_dir(settings::telegram_data_path);
                }

                if(!telegram_cfg)
                {
                    telegram_cfg = etl::make_shared<etl::settings::data<etl::webui::telegram_config_t>>(settings::telegram_data_path, settings::telegram_data_update_delay, default_cfg);
                    bool result = telegram_cfg->init();
                    Serial.print(F("[wifi::settings] init_telegram_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting Telegram to default ..."));
                        telegram_cfg->set(default_cfg);
                        bool reset_result = telegram_cfg->save();
                        Serial.print(F("[wifi::settings] Telegram reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_telegram_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки Telegram бота
             * @param cfg Конфигурация Telegram бота
             */
            bool save_telegram_config(const telegram_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_telegram_config()"));

                if(telegram_cfg)
                {
                    telegram_cfg->set(cfg);
                    bool result = telegram_cfg->save();
                    Serial.print(F("[wifi::settings] save_telegram_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_telegram_config() error: telegram_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки Telegram бота
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<telegram_config_t> load_telegram_config()
            {
                Serial.println(F("[wifi::settings] load_telegram_config()"));

                if(telegram_cfg)
                {
                    telegram_config_t cfg = telegram_cfg->get();
                    Serial.println(F("[wifi::settings] load_telegram_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_telegram_config(): telegram_cfg not inited, returning empty optional"));
                    return {};
                }
            }

            // ============================================================================
            // MQTT настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек MQTT
             * @param default_cfg Конфигурация MQTT по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_mqtt_config(const etl::webui::mqtt_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_mqtt_config()"));

                if(etl::little_fs::begin())
                {
                    etl::little_fs::create_dir(settings::mqtt_data_path);
                }

                if(!mqtt_cfg)
                {
                    mqtt_cfg = etl::make_shared<etl::settings::data<etl::webui::mqtt_config_t>>(settings::mqtt_data_path, settings::mqtt_data_update_delay, default_cfg);
                    bool result = mqtt_cfg->init();
                    Serial.print(F("[wifi::settings] init_mqtt_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting MQTT to default ..."));
                        mqtt_cfg->set(default_cfg);
                        bool reset_result = mqtt_cfg->save();
                        Serial.print(F("[wifi::settings] MQTT reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_mqtt_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки MQTT
             * @param cfg Конфигурация MQTT
             */
            bool save_mqtt_config(const mqtt_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_mqtt_config()"));

                if(mqtt_cfg)
                {
                    mqtt_cfg->set(cfg);
                    bool result = mqtt_cfg->save();
                    Serial.print(F("[wifi::settings] save_mqtt_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_mqtt_config() error: mqtt_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки MQTT
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<mqtt_config_t> load_mqtt_config()
            {
                Serial.println(F("[wifi::settings] load_mqtt_config()"));

                if(mqtt_cfg)
                {
                    mqtt_config_t cfg = mqtt_cfg->get();
                    Serial.println(F("[wifi::settings] load_mqtt_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_mqtt_config(): mqtt_cfg not inited, returning empty optional"));
                    return {};
                }
            }
        } // namespace settings

        // ============================================================================
        // Реализация server_config_t
        // ============================================================================

        void server_config_t::clear()
        {
            memset(hostname, 0, WIFI_CONFIG_HOSTNAME_SIZE);
            memset(ap_ssid, 0, WIFI_CONFIG_SSID_SIZE);
            memset(ap_password, 0, WIFI_CONFIG_PASSWORD_SIZE);
            memset(wifi_ssid, 0, WIFI_CONFIG_SSID_SIZE);
            memset(wifi_password, 0, WIFI_CONFIG_PASSWORD_SIZE);

            // Установка значений по умолчанию
            strncpy(hostname, "espdevice", WIFI_CONFIG_HOSTNAME_SIZE - 1);
            hostname[WIFI_CONFIG_HOSTNAME_SIZE - 1] = '\0';
            strncpy(ap_ssid, "ESP_Device_AP", WIFI_CONFIG_SSID_SIZE - 1);
            ap_ssid[WIFI_CONFIG_SSID_SIZE - 1] = '\0';
            strncpy(ap_password, "password123", WIFI_CONFIG_PASSWORD_SIZE - 1);
            ap_password[WIFI_CONFIG_PASSWORD_SIZE - 1] = '\0';

            port = 80;
            update_interval = 500;
        }

        void server_config_t::trace() const
        {
            Serial.println(F("=== server_config_t settings ==="));
            Serial.printf("hostname        = %s\n", hostname);
            Serial.printf("ap_ssid         = %s\n", ap_ssid);
            Serial.printf("ap_password     = %s\n", ap_password);
            Serial.printf("wifi_ssid       = %s\n", wifi_ssid);
            Serial.printf("wifi_password   = %s\n", wifi_password);
            Serial.printf("port            = %u\n", port);
            Serial.printf("update_interval = %u\n", update_interval);
            Serial.println(F("========================"));
        }

        // Setters
        void server_config_t::set_hostname(const String& value)
        {
            memset(hostname, 0, WIFI_CONFIG_HOSTNAME_SIZE);
            strncpy(hostname, value.c_str(), WIFI_CONFIG_HOSTNAME_SIZE - 1);
            hostname[WIFI_CONFIG_HOSTNAME_SIZE - 1] = '\0';
        }

        void server_config_t::set_ap_ssid(const String& value)
        {
            memset(ap_ssid, 0, WIFI_CONFIG_SSID_SIZE);
            strncpy(ap_ssid, value.c_str(), WIFI_CONFIG_SSID_SIZE - 1);
            ap_ssid[WIFI_CONFIG_SSID_SIZE - 1] = '\0';
        }

        void server_config_t::set_ap_password(const String& value)
        {
            memset(ap_password, 0, WIFI_CONFIG_PASSWORD_SIZE);
            strncpy(ap_password, value.c_str(), WIFI_CONFIG_PASSWORD_SIZE - 1);
            ap_password[WIFI_CONFIG_PASSWORD_SIZE - 1] = '\0';
        }

        void server_config_t::set_wifi_ssid(const String& value)
        {
            memset(wifi_ssid, 0, WIFI_CONFIG_SSID_SIZE);
            strncpy(wifi_ssid, value.c_str(), WIFI_CONFIG_SSID_SIZE - 1);
            wifi_ssid[WIFI_CONFIG_SSID_SIZE - 1] = '\0';
        }

        void server_config_t::set_wifi_password(const String& value)
        {
            memset(wifi_password, 0, WIFI_CONFIG_PASSWORD_SIZE);
            strncpy(wifi_password, value.c_str(), WIFI_CONFIG_PASSWORD_SIZE - 1);
            wifi_password[WIFI_CONFIG_PASSWORD_SIZE - 1] = '\0';
        }

        // Getters
        String server_config_t::get_hostname() const
        {
            return String(hostname);
        }

        String server_config_t::get_ap_ssid() const
        {
            return String(ap_ssid);
        }

        String server_config_t::get_ap_password() const
        {
            return String(ap_password);
        }

        String server_config_t::get_wifi_ssid() const
        {
            return String(wifi_ssid);
        }

        String server_config_t::get_wifi_password() const
        {
            return String(wifi_password);
        }

        // ============================================================================
        // Реализация ui_config_t
        // ============================================================================

        void ui_config_t::clear()
        {
            memset(language, 0, WIFI_CONFIG_LANGUAGE_SIZE);
            strncpy(language, "en", WIFI_CONFIG_LANGUAGE_SIZE - 1);
            language[WIFI_CONFIG_LANGUAGE_SIZE - 1] = '\0';

            dark_theme = false;
            large_font = false;
            use_bold_values = false;
        }

        void ui_config_t::trace() const
        {
            Serial.println(F("=== ui_config_t settings ==="));
            Serial.printf("language        = %s\n", language);
            Serial.printf("dark_theme      = %s\n", dark_theme ? "✅" : "⬜");
            Serial.printf("large_font      = %s\n", large_font ? "✅" : "⬜");
            Serial.printf("use_bold_values = %s\n", use_bold_values ? "✅" : "⬜");
            Serial.println(F("========================"));
        }

        // Setters
        void ui_config_t::set_language(const String& value)
        {
            memset(language, 0, WIFI_CONFIG_LANGUAGE_SIZE);
            strncpy(language, value.c_str(), WIFI_CONFIG_LANGUAGE_SIZE - 1);
            language[WIFI_CONFIG_LANGUAGE_SIZE - 1] = '\0';
        }

        void ui_config_t::set_dark_theme(bool value)
        {
            dark_theme = value;
        }

        void ui_config_t::set_large_font(bool value)
        {
            large_font = value;
        }

        void ui_config_t::set_use_bold_values(bool value)
        {
            use_bold_values = value;
        }

        // Getters
        String ui_config_t::get_language() const
        {
            return String(language);
        }

        bool ui_config_t::is_dark_theme() const
        {
            return dark_theme;
        }

        bool ui_config_t::is_large_font() const
        {
            return large_font;
        }

        bool ui_config_t::is_use_bold_values() const
        {
            return use_bold_values;
        }

        // ============================================================================
        // Реализация telegram_config_t
        // ============================================================================

        void telegram_config_t::clear()
        {
            // TODO: Реализовать очистку полей при добавлении
        }

        void telegram_config_t::trace() const
        {
            Serial.println(F("=== telegram_config_t settings ==="));
            Serial.println(F("TODO: Not implemented yet"));
            Serial.println(F("========================"));
        }

        // ============================================================================
        // Реализация mqtt_config_t
        // ============================================================================

        void mqtt_config_t::clear()
        {
            // TODO: Реализовать очистку полей при добавлении
        }

        void mqtt_config_t::trace() const
        {
            Serial.println(F("=== mqtt_config_t settings ==="));
            Serial.println(F("TODO: Not implemented yet"));
            Serial.println(F("========================"));
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_settings: no implementation for this platform")
#endif
