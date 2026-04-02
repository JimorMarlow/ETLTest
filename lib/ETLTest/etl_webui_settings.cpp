/**
 * @file etl_webui_settings.cpp
 * @brief Реализация конфигурационных структур для WebUI
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui_settings.h"

namespace etl
{
    namespace webui
    {
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
