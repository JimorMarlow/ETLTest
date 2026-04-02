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

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
