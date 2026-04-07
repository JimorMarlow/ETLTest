/**
 * @file light_webui.cpp
 * @brief Реализация сервера управления светодиодной лампой
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "light_webui.h"
#include "light_webui_html.h"
#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"

namespace etl
{
    namespace webui
    {
        // ============================================================================
        // Реализация light_control_server
        // ============================================================================

        device_info_t get_light_control_device_info()
        {
            device_info_t info;
            info.name = "Рабочая зона";
            info.description = "Кухня, v";
            info.description += APP_VERSION_STRING;
            info.icon_svg = LIGHT_DEVICE_ICON_SVG;
            return info;
        }

        light_control_server::light_control_server(const etl::optional<server_config_t>& cfg)
            : web_server_base_t(cfg)
        {
        }

        void light_control_server::set_light_settings(const kitchen_light_t& settings)
        {
            m_light_settings = settings;
        }

        kitchen_light_t light_control_server::get_light_settings() const
        {
            return m_light_settings;
        }

        void light_control_server::set_power(bool power)
        {
            m_light_settings.power = power;
            send_state_to_serial(m_light_settings.power, m_light_settings.brightness);
        }

        bool light_control_server::get_power() const
        {
            return m_light_settings.power;
        }

        void light_control_server::set_brightness(float brightness)
        {
            // Ограничение диапазона [1..100]
            m_light_settings.brightness = constrain(brightness, 1.0f, 100.0f);
            send_state_to_serial(m_light_settings.power, m_light_settings.brightness);
        }

        float light_control_server::get_brightness() const
        {
            return m_light_settings.brightness;
        }

        void light_control_server::send_state_to_serial(bool power, float brightness)
        {
            Serial.print(F("[LightControl] State: power="));
            Serial.print(power ? "ON" : "OFF");
            Serial.print(F(", brightness="));
            Serial.println(brightness, 1);
        }

        // ============================================================================
        // HTTP обработчики
        // ============================================================================

// === ESP8266: begin() — НЕ вызывает web_server_base_t::begin() и НЕ создаёт m_server
#ifdef ESP8266
        bool light_control_server::begin(const device_info_t& device_info)
        {
            Serial.println(F("[LightControl] Init (ESP8266, no shared_ptr)..."));
            Serial.print(F("[LightControl] Free heap before: "));
            Serial.println(ESP.getFreeHeap());

            m_device_info = device_info;
            if (load_settings()) Serial.println(F("[LightControl] Settings loaded"));

            if (start_ap()) {
                Serial.println(F("[LightControl] AP started"));
                Serial.print(F("[LightControl] Free heap after AP: "));
                Serial.println(ESP.getFreeHeap());

                m_http_server.begin(); // член класса — БЕЗ отдельной heap-аллокации!
                Serial.print(F("[LightControl] Free heap after server: "));
                Serial.println(ESP.getFreeHeap());

                setup_http_routes();
                m_initialized = true;
                Serial.println(F("[LightControl] Server started (ESP8266)"));
                return true;
            }
            return false;
        }

        void light_control_server::handle_client()
        {
            m_http_server.handleClient();
            if (m_pending_cb_counter > 0) {
                m_pending_cb_counter--;
                if (m_pending_cb_counter == 0) {
                    if (m_pending_settings_cb) { m_pending_settings_cb = false; if (m_on_settings_cb) m_on_settings_cb(); }
                    if (m_pending_content_cb) { m_pending_content_cb = false; if (m_on_content_cb) m_on_content_cb(); }
                    if (m_pending_factory_reset_cb) { m_pending_factory_reset_cb = false; if (m_on_factory_reset_cb) m_on_factory_reset_cb(); }
                }
            }
        }
#endif // ESP8266

// === ESP32: start_http_server через shared_ptr (памяти достаточно)
#ifndef ESP8266
        void light_control_server::start_http_server()
        {
            Serial.println(F("[LightControl] Starting HTTP server..."));
            Serial.print(F("[LightControl] Free heap: "));
            Serial.println(ESP.getFreeHeap());

            m_server = etl::make_shared<etl_web_server_t>(m_config.has_value() ? m_config->port : 80);
            setup_http_routes();
            m_server->begin();
            Serial.println(F("[LightControl] HTTP server started on port "));
            Serial.println(m_config.has_value() ? m_config->port : 80);

            // mDNS — только на ESP32
            static bool mdns_initialized = false;
            static bool mdns_service_added = false;

            if (!mdns_initialized) {
                Serial.print(F("[LightControl] Initializing mDNS: "));
                if (MDNS.begin(m_config.has_value() ? m_config->get_hostname().c_str() : "espdevice")) {
                    Serial.print(F("[LightControl] mDNS: http://"));
                    Serial.print(m_config.has_value() ? m_config->get_hostname() : "espdevice");
                    Serial.println(F(".local"));
                    mdns_initialized = true;
                } else {
                    Serial.println(F("[LightControl] mDNS failed"));
                }
            } else {
                Serial.print(F("[LightControl] mDNS already running: http://"));
                Serial.print(m_config.has_value() ? m_config->get_hostname() : "espdevice");
                Serial.println(F(".local"));
            }

            if (!mdns_service_added) {
                MDNS.addService("http", "tcp", m_config.has_value() ? m_config->port : 80);
                mdns_service_added = true;
            }
            Serial.println(F("[LightControl] mDNS service added and updated"));
        }
#endif // ESP32 (закрывает #ifndef ESP8266)

        void light_control_server::handle_root()
        {
            Serial.println(F("[LightControl] Serving root page..."));

#ifdef ESP8266
            // HTML в PROGMEM — отправляем напрямую из flash
            m_http_server.send_P(200, "text/html", LIGHT_WEBUI_HTML);
#else
            _sendHeader("Cache-Control", "no-cache");
            _send_P(200, "text/html", LIGHT_WEBUI_HTML);
#endif

            Serial.println(F("[LightControl] Page sent"));
        }

        void light_control_server::handle_api_status()
        {
            JsonDocument doc;
            doc["connected"] = is_connected();
            doc["ssid"] = m_config.has_value() ? m_config->get_wifi_ssid() : "";
            doc["ip"] = get_ip_address();
#ifdef ESP8266
            doc["rssi"] = WiFi.RSSI();
#elif defined(ESP32)
            doc["rssi"] = WiFi.RSSI();
#endif
            doc["mode"] = get_mode();

            // WiFi статус текстом
            String mode = get_mode();
            if (mode == "AP") {
                doc["wifi"] = "ap";
            } else if (is_connected()) {
                doc["wifi"] = "sta";
            } else {
                doc["wifi"] = "error";
            }

            // MQTT статус (заглушка - пока нет подключения)
            doc["mqtt"] = "disconnected";

            // Telegram статус (заглушка - пока нет подключения)
            doc["telegram"] = "disconnected";

            String response;
            serializeJson(doc, response);
            _send(200, "application/json", response);
        }

        void light_control_server::handle_api_device_info()
        {
            JsonDocument doc;
            doc["name"] = m_device_info.name;
            doc["description"] = m_device_info.description;
            doc["icon_svg"] = m_device_info.icon_svg;

            String response;
            serializeJson(doc, response);
            _send(200, "application/json", response);
        }

        void light_control_server::handle_api_ui_config()
        {
            JsonDocument doc;
            if (m_ui_config.has_value()) {
                doc["language"] = m_ui_config->get_language();
                doc["dark_theme"] = m_ui_config->is_dark_theme();
                doc["large_font"] = m_ui_config->is_large_font();
                doc["use_bold_values"] = m_ui_config->is_use_bold_values();
            } else {
                doc["language"] = "en";
                doc["dark_theme"] = false;
                doc["large_font"] = false;
                doc["use_bold_values"] = false;
            }

            String response;
            serializeJson(doc, response);
            _send(200, "application/json", response);
        }

        void light_control_server::handle_api_state()
        {
#ifdef ESP8266
            String resp;
            resp.reserve(50);
            resp = "{\"power\":";
            resp += m_light_settings.power ? "true" : "false";
            resp += ",\"brightness\":";
            resp += String(m_light_settings.brightness, 1);
            resp += "}";
            _send(200, "application/json", resp);
#else
            JsonDocument doc;
            doc["power"] = m_light_settings.power;
            doc["brightness"] = m_light_settings.brightness;
            String response;
            serializeJson(doc, response);
            _send(200, "application/json", response);
#endif
        }

        void light_control_server::handle_api_control()
        {
#ifdef ESP8266
            // Debounce — не чаще 50ms
            uint32_t now = millis();
            if (now - m_last_control_time < CONTROL_DEBOUNCE_MS) {
                _send(200, "application/json", F("{\"success\":true,\"debounced\":true}"));
                return;
            }
            m_last_control_time = now;
            
            Serial.print(F("[LightControl] Heap before control: "));
            Serial.println(ESP.getFreeHeap());
#endif

            Serial.println(F("[LightControl] API: /api/control"));

            if (_hasArg("plain")) {
                String body = _arg("plain");
#ifdef ESP8266
                // StaticJsonDocument — без аллокаций в куче
                StaticJsonDocument<256> doc;
#else
                JsonDocument doc;
#endif
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    _send(400, "application/json", F("{\"success\":false,\"message\":\"Invalid JSON\"}"));
                    return;
                }

                // Обновление состояния питания
                if (doc["power"].is<bool>()) {
                    set_power(doc["power"].as<bool>());
                }

                // Обновление яркости
                if (doc["brightness"].is<float>()) {
                    set_brightness(doc["brightness"].as<float>());
                } else if (doc["brightness"].is<int>()) {
                    set_brightness(static_cast<float>(doc["brightness"].as<int>()));
                }

                // Отправка подтверждения
#ifdef ESP8266
                // Формируем ответ без промежуточного String
                String resp;
                resp.reserve(60);
                resp = "{\"success\":true,\"power\":";
                resp += m_light_settings.power ? "true" : "false";
                resp += ",\"brightness\":";
                resp += String(m_light_settings.brightness, 1);
                resp += "}";
                _send(200, "application/json", resp);
#else
                JsonDocument resp;
                resp["success"] = true;
                resp["power"] = m_light_settings.power;
                resp["brightness"] = m_light_settings.brightness;
                String response;
                serializeJson(resp, response);
                _send(200, "application/json", response);
#endif
            } else {
                _send(400, "application/json", F("{\"success\":false,\"message\":\"No data\"}"));
            }
        }

        void light_control_server::handle_api_ui_settings()
        {
            Serial.println(F("[LightControl] API: /api/ui_settings"));

            if (_hasArg("plain")) {
                String body = _arg("plain");
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    send_error_response("Invalid JSON");
                    return;
                }

                // Применение настроек интерфейса через setter'ы
                if (!m_ui_config.has_value()) {
                    m_ui_config = ui_config_t();
                }

                if (doc["language"].is<const char*>()) {
                    m_ui_config->set_language(doc["language"].as<String>());
                }
                if (doc["dark_theme"].is<bool>()) {
                    m_ui_config->set_dark_theme(doc["dark_theme"].as<bool>());
                }
                if (doc["large_font"].is<bool>()) {
                    m_ui_config->set_large_font(doc["large_font"].as<bool>());
                }
                if (doc["use_bold_values"].is<bool>()) {
                    m_ui_config->set_use_bold_values(doc["use_bold_values"].as<bool>());
                }

                // Сохранение настроек интерфейса
                settings::save_ui_config(*m_ui_config);

                send_success_response("UI settings updated");
                Serial.println(F("[LightControl] UI settings updated"));
            } else {
                send_error_response("No data provided");
            }
        }

        void light_control_server::handle_api_config()
        {
            JsonDocument doc;
            doc["device_name"] = m_device_info.name;
            doc["device_description"] = m_device_info.description;
            if (m_config.has_value()) {
                doc["hostname"] = m_config->get_hostname();
                doc["ap_ssid"] = m_config->get_ap_ssid();
                doc["port"] = m_config->port;
                doc["wifi_ssid"] = m_config->get_wifi_ssid();
            }
            if (m_ui_config.has_value()) {
                doc["language"] = m_ui_config->get_language();
                doc["dark_theme"] = m_ui_config->is_dark_theme();
                doc["large_font"] = m_ui_config->is_large_font();
                doc["use_bold_values"] = m_ui_config->is_use_bold_values();
            }
            String response;
            serializeJson(doc, response);
            _send(200, "application/json", response);
        }

        void light_control_server::handle_api_settings()
        {
            Serial.println(F("[LightControl] API: /api/settings - scheduling settings callback"));

            // Отправляем успешный ответ клиенту
            send_success_response("Switching to settings server");

            // Запланировать callback — выполнится через N тиков
            schedule_settings_cb();
        }

        // ============================================================================
        // Статические callback-обёртки для ESP8266 (экономия RAM)
        // ============================================================================
#ifdef ESP8266
        // Глобальный указатель на текущий сервер для статических callback'ов
        static light_control_server* s_self_server = nullptr;

        // Единый диспетчер — вызывается ONCE на каждый запрос
        void _cb_dispatch(MinimalHttpServer& srv, const char* uri, bool is_post, const char* body, size_t body_len) {
            if (strcmp(uri, "/favicon.ico") == 0 || strcmp(uri, "/apple-touch-icon.png") == 0 ||
                strcmp(uri, "/apple-touch-icon-precomposed.png") == 0) {
                srv.send(204, "text/plain", "");
                return;
            }

            if (!is_post) {
                if (strcmp(uri, "/") == 0) s_self_server->handle_root();
                else if (strcmp(uri, "/api/status") == 0) s_self_server->handle_api_status();
                else if (strcmp(uri, "/api/config") == 0) s_self_server->handle_api_config();
                else if (strcmp(uri, "/api/device_info") == 0) s_self_server->handle_api_device_info();
                else if (strcmp(uri, "/api/ui_config") == 0) s_self_server->handle_api_ui_config();
                else if (strcmp(uri, "/api/state") == 0) s_self_server->handle_api_state();
                else srv.send(404, "text/plain", "Not Found");
            } else {
                if (strcmp(uri, "/api/ui_settings") == 0) s_self_server->handle_api_ui_settings();
                else if (strcmp(uri, "/api/control") == 0) s_self_server->handle_api_control();
                else if (strcmp(uri, "/api/settings") == 0) s_self_server->handle_api_settings();
                else srv.send(404, "text/plain", "Not Found");
            }
        }
#endif // ESP8266

        void light_control_server::setup_http_routes()
        {
            Serial.println(F("[LightControl] Setting up HTTP routes..."));

#ifdef ESP8266
            // ESP8266: ОДИН dispatch — БЕЗ отдельных heap-аллокаций
            s_self_server = this;
            m_http_server.onDispatch(_cb_dispatch);
#else
            // ESP32: лямбды с захватом (памяти достаточно), но без scan/save/reset/ap_settings
            m_server->on("/", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /"));
                handle_root();
            });
            m_server->on("/favicon.ico", HTTP_GET, [this]() { m_server->send(204); });
            m_server->on("/apple-touch-icon.png", HTTP_GET, [this]() { m_server->send(204); });
            m_server->on("/apple-touch-icon-precomposed.png", HTTP_GET, [this]() { m_server->send(204); });
            m_server->on("/api/status", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/status"));
                handle_api_status();
            });
            m_server->on("/api/config", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/config"));
                handle_api_config();
            });
            m_server->on("/api/ui_settings", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/ui_settings"));
                handle_api_ui_settings();
            });
            m_server->on("/api/device_info", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/device_info"));
                handle_api_device_info();
            });
            m_server->on("/api/ui_config", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/ui_config"));
                handle_api_ui_config();
            });
            m_server->on("/api/state", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/state"));
                handle_api_state();
            });
            m_server->on("/api/control", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/control"));
                handle_api_control();
            });
            m_server->on("/api/settings", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/settings"));
                handle_api_settings();
            });
            m_server->onNotFound([this]() {
                const String& uri = m_server->uri();
                if (uri == "/favicon.ico" || uri == "/apple-touch-icon.png" || uri == "/apple-touch-icon-precomposed.png") {
                    m_server->send(204);
                } else {
                    m_server->send(404, "text/plain", "Not Found");
                }
            });
#endif // ESP8266/ESP32
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("light_webui: no implementation for this platform")
#endif


