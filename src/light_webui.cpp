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

        void light_control_server::start_http_server()
        {
#ifdef ESP8266
            Serial.println(F("[LightControl] Init (ESP8266, no shared_ptr)..."));
            Serial.print(F("[LightControl] Free heap before: "));
            Serial.println(ESP.getFreeHeap());
#endif

            Serial.println(F("[LightControl] Starting HTTP server..."));

            // Создание серверера через shared_ptr
            m_server = etl::make_shared<etl_web_server_t>(m_config.has_value() ? m_config->port : 80);

            // Настройка роутинга
            setup_http_routes();

            // Запуск сервера
            m_server->begin();

#ifdef ESP8266
            Serial.print(F("[LightControl] Free heap after server: "));
            Serial.println(ESP.getFreeHeap());
#endif

            Serial.print(F("[LightControl] HTTP server started on port "));
            Serial.println(m_config.has_value() ? m_config->port : 80);

            // mDNS - инициализация при каждой загрузке
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

            // Добавляем сервис http только один раз
            if (!mdns_service_added) {
                MDNS.addService("http", "tcp", m_config.has_value() ? m_config->port : 80);
                mdns_service_added = true;
            }
#ifdef ESP8266
            MDNS.update();
#endif

            Serial.println(F("[LightControl] mDNS service added and updated"));
        }

        void light_control_server::handle_root()
        {
            Serial.println(F("[LightControl] Serving root page..."));

            // Отправка HTML напрямую из PROGMEM
            m_server->sendHeader("Cache-Control", "no-cache");
            m_server->send_P(200, "text/html", LIGHT_WEBUI_HTML);

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

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void light_control_server::handle_api_device_info()
        {
            JsonDocument doc;
            doc["name"] = m_device_info.name;
            doc["description"] = m_device_info.description;
            doc["icon_svg"] = m_device_info.icon_svg;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
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
            m_server->send(200, "application/json", response);
        }

        void light_control_server::handle_api_state()
        {
            JsonDocument doc;
            doc["power"] = m_light_settings.power;
            doc["brightness"] = m_light_settings.brightness;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void light_control_server::handle_api_control()
        {
            Serial.println(F("[LightControl] API: /api/control"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    JsonDocument err;
                    err["success"] = false;
                    err["message"] = "Invalid JSON";
                    String response;
                    serializeJson(err, response);
                    m_server->send(400, "application/json", response);
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
                JsonDocument resp;
                resp["success"] = true;
                resp["power"] = m_light_settings.power;
                resp["brightness"] = m_light_settings.brightness;

                String response;
                serializeJson(resp, response);
                m_server->send(200, "application/json", response);
            } else {
                JsonDocument err;
                err["success"] = false;
                err["message"] = "No data provided";
                String response;
                serializeJson(err, response);
                m_server->send(400, "application/json", response);
            }
        }

        void light_control_server::handle_api_ui_settings()
        {
            Serial.println(F("[LightControl] API: /api/ui_settings"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
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

        void light_control_server::handle_api_scan()
        {
            Serial.println(F("[LightControl] API: /api/scan"));

            // Проверка кэша
            uint32_t current_time = millis();
            if (m_scan_cache.size() > 0 && (current_time - m_scan_timestamp) < SCAN_CACHE_TIME) {
                Serial.println(F("[LightControl] Returning cached scan results"));
                send_scan_response();
                return;
            }

            // Сканирование сетей
            m_scan_cache.clear();
            int32_t count = scan_networks(m_scan_cache);
            m_scan_timestamp = millis();

            Serial.printf("[LightControl] Scan completed: %d networks\n", count);

            send_scan_response();
        }

        void light_control_server::handle_api_connect()
        {
            Serial.println(F("[LightControl] API: /api/connect"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    send_error_response("Invalid JSON");
                    return;
                }

                String ssid = doc["ssid"].as<String>();
                String password = doc["password"].as<String>();

                if (ssid.length() == 0) {
                    send_error_response("SSID is required");
                    return;
                }

                Serial.print(F("[LightControl] Connecting to: "));
                Serial.println(ssid);

                // Сохраняем настройки
                if (!m_config.has_value()) {
                    m_config = server_config_t();
                }
                m_config->set_wifi_ssid(ssid);
                m_config->set_wifi_password(password);

                // Сохранение текущего режима для восстановления
                WiFiMode_t previous_mode = WiFi.getMode();

                // Установка режима AP+STA для подключения
                WiFi.mode(WIFI_AP_STA);

                // Подключение к сети с ожиданием
                WiFi.begin(m_config->get_wifi_ssid().c_str(), m_config->get_wifi_password().c_str());

                // Ожидание подключения (до 15 секунд)
                uint32_t start_time = millis();
                const uint32_t timeout = 15000;

                while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout) {
                    delay(500);
                    yield();
                    // Обработка HTTP запросов во время ожидания
                    if (m_server) {
                        m_server->handleClient();
                    }
                    Serial.print(F("."));
                }

                // Проверка результата подключения
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println(F("\n[LightControl] Connected successfully"));
                    Serial.print(F("[LightControl] IP address: "));
                    Serial.println(WiFi.localIP());

                    // Отправка успешного ответа с IP
                    JsonDocument response_doc;
                    response_doc["success"] = true;
                    response_doc["message"] = "Connected successfully";
                    response_doc["ip"] = WiFi.localIP().toString();
                    response_doc["ssid"] = ssid;

                    String response;
                    serializeJson(response_doc, response);
                    m_server->send(200, "application/json", response);

                    // Возврат в предыдущий режим (AP или AP+STA)
                    if (previous_mode == WIFI_AP) {
                        WiFi.mode(WIFI_AP_STA);
                    }

                    m_connection_status = connection_status_t::connected;
                } else {
                    Serial.println(F("\n[LightControl] Connection failed"));
                    m_connection_status = connection_status_t::error;

                    // Отправка ответа с ошибкой
                    JsonDocument response_doc;
                    response_doc["success"] = false;
                    response_doc["message"] = "Connection timeout";

                    String response;
                    serializeJson(response_doc, response);
                    m_server->send(200, "application/json", response);

                    // Возврат в предыдущий режим и перезапуск AP
                    if (previous_mode == WIFI_AP || previous_mode == WIFI_AP_STA) {
                        WiFi.mode(WIFI_AP);
                        WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                                    m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
                        Serial.println(F("[LightControl] AP restarted after connection failure"));
                    } else if (previous_mode == WIFI_OFF) {
                        WiFi.mode(WIFI_AP);
                        WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                                    m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
                        Serial.println(F("[LightControl] AP started after connection failure"));
                    } else {
                        WiFi.mode(previous_mode);
                    }
                }
            } else {
                send_error_response("No data provided");
            }
        }

        void light_control_server::handle_api_disconnect()
        {
            Serial.println(F("[LightControl] API: /api/disconnect"));

            // Сброс настроек WiFi
            if (m_config.has_value()) {
                m_config->set_wifi_ssid("");
                m_config->set_wifi_password("");
            }

            // Сначала отправляем успешный ответ клиенту
            JsonDocument response_doc;
            response_doc["success"] = true;
            response_doc["message"] = "Disconnected";

            String response;
            serializeJson(response_doc, response);
            m_server->send(200, "application/json", response);

            // Задержка для отправки ответа клиенту
            delay(100);
            yield();

            // Отключение от сети
            WiFi.disconnect(true);
            m_connection_status = connection_status_t::disconnected;

#ifdef ESP32
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                        m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
            Serial.println(F("[LightControl] Switched to AP+STA mode"));
#else
            WiFi.mode(WIFI_AP);
            WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                        m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
#endif

            Serial.println(F("[LightControl] Disconnected from WiFi, AP restarted"));
        }

        void light_control_server::handle_api_config()
        {
            JsonDocument doc;
            // Информация об устройстве (из m_device_info)
            doc["device_name"] = m_device_info.name;
            doc["device_description"] = m_device_info.description;
            doc["device_icon_svg"] = m_device_info.icon_svg;

            // WiFi конфигурация (из m_config)
            if (m_config.has_value()) {
                doc["hostname"] = m_config->get_hostname();
                doc["ap_ssid"] = m_config->get_ap_ssid();
                doc["ap_password"] = m_config->get_ap_password();
                doc["port"] = m_config->port;
                doc["wifi_ssid"] = m_config->get_wifi_ssid();
            }

            // Настройки интерфейса (из m_ui_config)
            doc["ui_config_initialized"] = m_ui_config.has_value();
            if (m_ui_config.has_value()) {
                doc["language"] = m_ui_config->get_language();
                doc["dark_theme"] = m_ui_config->is_dark_theme();
                doc["large_font"] = m_ui_config->is_large_font();
                doc["use_bold_values"] = m_ui_config->is_use_bold_values();
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void light_control_server::handle_api_save()
        {
            Serial.println(F("[LightControl] API: /api/save"));

            bool success = save_settings();

            if (success) {
                send_success_response("Settings saved");
            } else {
                send_error_response("Failed to save settings");
            }
        }

        void light_control_server::handle_api_reset()
        {
            Serial.println(F("[LightControl] API: /api/reset"));

            bool success = reset_settings();

            if (success) {
                send_success_response("Settings reset. Rebooting...");
                Serial.println(F("[LightControl] Rebooting in 2 seconds..."));
                delay(2000);
                reboot();
            } else {
                send_error_response("Failed to reset settings");
            }
        }

        void light_control_server::handle_api_ap_settings()
        {
            Serial.println(F("[LightControl] API: /api/ap_settings"));

            if (m_server->hasArg("plain")) {
                String body = m_server->arg("plain");
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, body);

                if (error) {
                    send_error_response("Invalid JSON");
                    return;
                }

                String ap_ssid = doc["ap_ssid"].as<String>();
                String ap_password = doc["ap_password"].as<String>();

                if (ap_ssid.length() == 0) {
                    send_error_response("AP SSID is required");
                    return;
                }

                if (ap_password.length() > 0 && ap_password.length() < 8) {
                    send_error_response("AP password must be at least 8 characters");
                    return;
                }

                // Применение настроек AP через setter'ы
                if (!m_config.has_value()) {
                    m_config = server_config_t();
                }
                m_config->set_ap_ssid(ap_ssid);
                m_config->set_ap_password(ap_password);

                // Сохранение настроек в постоянной памяти
                save_settings();

                // Сначала отправляем ответ клиенту
                send_success_response("AP settings applied", m_config->get_ap_ssid());

                // Небольшая задержка для отправки ответа
                delay(100);

                // Перезапуск точки доступа
                WiFi.softAPdisconnect(true);
                start_ap();

                Serial.println(F("[LightControl] AP restarted, client should reconnect"));
            } else {
                send_error_response("No data provided");
            }
        }

        void light_control_server::handle_api_settings()
        {
            Serial.println(F("[LightControl] API: /api/settings - scheduling settings callback"));

            // Отправляем успешный ответ клиенту
            send_success_response("Switching to settings server");

            // Запланировать callback — выполнится через N тиков
            schedule_settings_cb();
        }

        void light_control_server::setup_http_routes()
        {
            Serial.println(F("[LightControl] Setting up HTTP routes..."));

            // Главная страница
            m_server->on("/", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /"));
                handle_root();
            });

            // Favicon и Apple touch icons (возвращаем 204 No Content)
            m_server->on("/favicon.ico", HTTP_GET, [this]() {
                m_server->send(204);
            });
            m_server->on("/apple-touch-icon.png", HTTP_GET, [this]() {
                m_server->send(204);
            });
            m_server->on("/apple-touch-icon-precomposed.png", HTTP_GET, [this]() {
                m_server->send(204);
            });

            // API endpoints
            m_server->on("/api/scan", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/scan"));
                handle_api_scan();
            });
            m_server->on("/api/connect", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/connect"));
                handle_api_connect();
            });
            m_server->on("/api/disconnect", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/disconnect"));
                handle_api_disconnect();
            });
            m_server->on("/api/status", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/status"));
                handle_api_status();
            });
            m_server->on("/api/config", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/config"));
                handle_api_config();
            });
            m_server->on("/api/save", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/save"));
                handle_api_save();
            });
            m_server->on("/api/reset", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/reset"));
                handle_api_reset();
            });
            m_server->on("/api/ap_settings", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/ap_settings"));
                handle_api_ap_settings();
            });
            m_server->on("/api/ui_settings", HTTP_POST, [this]() {
                Serial.println(F("[LightControl] Request: /api/ui_settings"));
                handle_api_ui_settings();
            });

            // Специфичные API для управления лампой
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

            // Обработчик для остальных путей - 404
            m_server->onNotFound([this]() {
                Serial.print(F("[LightControl] Request 404: "));
                Serial.println(m_server->uri());
                m_server->send(404, "text/plain", "Not Found");
            });
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("light_webui: no implementation for this platform")
#endif
