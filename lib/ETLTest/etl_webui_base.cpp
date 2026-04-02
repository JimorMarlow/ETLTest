/**
 * @file etl_webui_base.cpp
 * @brief Реализация базового класса для веб-серверов WebUI
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui_base.h"

#if defined(ESP8266)
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <ESPmDNS.h>
#endif

namespace etl
{
    namespace webui
    {
        // ============================================================================
        // Реализация web_server_base_t
        // ============================================================================

        web_server_base_t::web_server_base_t(const etl::optional<server_config_t>& cfg)
            : m_config(cfg)
        {
        }

        bool web_server_base_t::init_mdns(const String& hostname)
        {
            Serial.println(F("[WebUI] Initializing mDNS..."));

            // Статические переменные сохраняют состояние в течение сессии
            static bool mdns_initialized = false;
            static bool mdns_service_added = false;

            if (!mdns_initialized) {
                // Первый запуск после загрузки
                Serial.print(F("[WebUI] mDNS: "));
                if (MDNS.begin(hostname.c_str())) {
                    Serial.print(F("[WebUI] mDNS: http://"));
                    Serial.print(hostname);
                    Serial.println(F(".local"));

                    mdns_initialized = true;
                    return true;
                } else {
                    Serial.println(F("[WebUI] mDNS initialization failed"));
                    return false;
                }
            }

            // mDNS уже инициализирован, проверяем, нужно ли добавлять сервис
            if (!mdns_service_added) {
                Serial.println(F("[WebUI] mDNS service already added"));
                mdns_service_added = true;
            }

            return true;
        }

        void web_server_base_t::tick()
        {
            handle();
            handle_client();
        }

        void web_server_base_t::handle_client()
        {
            if (m_server) {
                m_server->handleClient();
            }
        }

        void web_server_base_t::reboot()
        {
            Serial.println(F("[WebUI] Rebooting..."));
#if defined(ESP8266)
            ESP.reset();
#elif defined(ESP32)
            ESP.restart();
#endif
        }

        void web_server_base_t::set_device_info(const device_info_t& info)
        {
            m_device_info = info;
        }

        const device_info_t& web_server_base_t::get_device_info() const 
        { 
            return m_device_info; 
        }

        etl::optional<ui_config_t> web_server_base_t::get_ui_config() const
        {
            return m_ui_config;
        }

        void web_server_base_t::disconnect()
        {
            Serial.println(F("[WebUI] Disconnecting..."));
#if defined(ESP8266)
            WiFi.disconnect(true);
#elif defined(ESP32)
            WiFi.disconnect(true);
#endif
            m_connection_status = connection_status_t::disconnected;
        }

        bool web_server_base_t::save_settings()
        {
            Serial.println(F("[WebUI] Saving settings..."));

            bool wifi_saved = false;
            bool ui_saved = false;

            // Сохранение WiFi настроек
            if (m_config.has_value()) {
                m_config->trace();
                wifi_saved = settings::save_wifi_config(*m_config);
                Serial.print(F("[WebUI] WiFi settings saved: "));
                Serial.println(wifi_saved ? F("OK") : F("FAILED"));
            } else {
                Serial.println(F("[WebUI] No WiFi config to save"));
            }

            // Сохранение UI настроек
            if (m_ui_config.has_value()) {
                m_ui_config->trace();
                ui_saved = settings::save_ui_config(*m_ui_config);
                Serial.print(F("[WebUI] UI settings saved: "));
                Serial.println(ui_saved ? F("OK") : F("FAILED"));
            } else {
                Serial.println(F("[WebUI] No UI config to save"));
            }

            return wifi_saved || ui_saved;
        }

        bool web_server_base_t::load_settings()
        {
            Serial.println(F("[WebUI] Loading settings..."));

            // Загрузка WiFi настроек
            if (auto wifi_cfg = settings::load_wifi_config(); wifi_cfg.has_value())
            {
                m_config = wifi_cfg;
                Serial.println(F("[WebUI] WiFi settings loaded"));
            } else {
                Serial.println(F("[WebUI] No WiFi settings found"));
            }

            // Загрузка UI настроек
            if (auto ui_cfg = settings::load_ui_config(); ui_cfg.has_value())
            {
                m_ui_config = ui_cfg;
                Serial.println(F("[WebUI] UI settings loaded"));
            } else {
                Serial.println(F("[WebUI] No UI settings found"));
            }

            return true;
        }

        bool web_server_base_t::reset_settings()
        {
            Serial.println(F("[WebUI] Resetting settings..."));

            // Сброс WiFi конфигурации к значениям по умолчанию
            if (m_config.has_value()) {
                m_config->clear();
                settings::save_wifi_config(*m_config);
                Serial.println(F("[WebUI] WiFi settings reset"));
            }

            // Сброс UI конфигурации к значениям по умолчанию
            if (m_ui_config.has_value()) {
                m_ui_config->clear();
                settings::save_ui_config(*m_ui_config);
                Serial.println(F("[WebUI] UI settings reset"));
            }

            return true;
        }

        void web_server_base_t::set_config(const etl::optional<server_config_t>& cfg)
        {
            m_config = cfg;
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
