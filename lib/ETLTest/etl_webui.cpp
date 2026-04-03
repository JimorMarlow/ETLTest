/**
 * @file etl_wifi_setup.cpp
 * @brief Реализация WiFi Setup Server
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui.h"
#include "etl_wifi_setup_html.h"
#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"

namespace etl
{
    namespace webui
    {
        // ============================================================================
        // Реализация server_setup
        // ============================================================================

        server_setup::server_setup(const etl::optional<server_config_t>& cfg)
            : web_server_base_t(cfg)
        {
        }

        void server_setup::start_http_server()
        {
            Serial.println(F("[WiFiSetup] Starting HTTP server..."));

            // Создание сервера через shared_ptr
            m_server = etl::make_shared<etl_web_server_t>(m_config.has_value() ? m_config->port : 80);

            // Настройка роутинга
            setup_http_routes();

            // Запуск сервера
            m_server->begin();
            Serial.print(F("[WiFiSetup] HTTP server started on port "));
            Serial.println(m_config.has_value() ? m_config->port : 80);

            // mDNS - инициализация при каждой загрузке
            // Статические переменные сохраняют состояние в течение сессии
            static bool mdns_initialized = false;
            static bool mdns_service_added = false;

            if (!mdns_initialized) {
                // Первый запуск после загрузки
                Serial.print(F("[WiFiSetup] Initializing mDNS: "));
                if (MDNS.begin(m_config.has_value() ? m_config->get_hostname().c_str() : "espdevice")) {
                    Serial.print(F("[WiFiSetup] mDNS: http://"));
                    Serial.print(m_config.has_value() ? m_config->get_hostname() : "espdevice");
                    Serial.println(F(".local"));
                    mdns_initialized = true;
                } else {
                    Serial.println(F("[WiFiSetup] mDNS failed"));
                }
            } else {
                // mDNS уже инициализирован в этой сессии
                Serial.print(F("[WiFiSetup] mDNS already running: http://"));
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

            Serial.println(F("[WiFiSetup] mDNS service added and updated"));
        }

        // ============================================================================
        // Реализация server_setup (специфичная)
        // ============================================================================

        void server_setup::handle_root()
        {
            Serial.println(F("[WiFiSetup] Serving root page..."));

            // Отправка HTML напрямую из PROGMEM
            m_server->sendHeader("Cache-Control", "no-cache");
            m_server->send_P(200, "text/html", HTML_TEMPLATE);

            Serial.println(F("[WiFiSetup] Page sent"));
        }

        void server_setup::handle_api_scan()
        {
            Serial.println(F("[WiFiSetup] API: /api/scan"));

            // Проверка кэша
            uint32_t current_time = millis();
            if (m_scan_cache.size() > 0 && (current_time - m_scan_timestamp) < SCAN_CACHE_TIME) {
                Serial.println(F("[WiFiSetup] Returning cached scan results"));
                send_scan_response();
                return;
            }

            // Сканирование сетей
            m_scan_cache.clear();
            int32_t count = scan_networks(m_scan_cache);
            m_scan_timestamp = millis();

            Serial.printf("[WiFiSetup] Scan completed: %d networks\n", count);

            send_scan_response();
        }

        void server_setup::handle_api_connect()
        {
            Serial.println(F("[WiFiSetup] API: /api/connect"));

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

                Serial.print(F("[WiFiSetup] Connecting to: "));
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
                // Это позволит точке доступа продолжать работу во время подключения к STA
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
                    Serial.println(F("\n[WiFiSetup] Connected successfully"));
                    Serial.print(F("[WiFiSetup] IP address: "));
                    Serial.println(WiFi.localIP());
                    Serial.print(F("[WiFiSetup] Subnet Mask: "));
                    Serial.println(WiFi.subnetMask());
                    Serial.print(F("[WiFiSetup] Gateway IP: "));
                    Serial.println(WiFi.gatewayIP());
                    Serial.print(F("[WiFiSetup] DNS IP: "));
                    Serial.println(WiFi.dnsIP());

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
                    // Если был WIFI_OFF или WIFI_STA, оставляем как есть

                    m_connection_status = connection_status_t::connected;
                } else {
                    Serial.println(F("\n[WiFiSetup] Connection failed"));
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
                        Serial.println(F("[WiFiSetup] AP restarted after connection failure"));
                    } else if (previous_mode == WIFI_OFF) {
                        // Если WiFi был выключен, включаем AP для продолжения настройки
                        WiFi.mode(WIFI_AP);
                        WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                                    m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
                        Serial.println(F("[WiFiSetup] AP started after connection failure"));
                    } else {
                        // previous_mode == WIFI_STA - остаемся в STA
                        WiFi.mode(previous_mode);
                    }
                }
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::handle_api_disconnect()
        {
            Serial.println(F("[WiFiSetup] API: /api/disconnect"));

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
            // На ESP32 остаёмся в режиме AP+STA для стабильности
            // Точка доступа уже работает в этом режиме
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                        m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");

            Serial.println(F("[WiFiSetup] Switched to AP+STA mode"));
#else
            // На ESP8266 просто переключаемся в режим AP
            // HTTP сервер продолжает работать
            WiFi.mode(WIFI_AP);
            WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                        m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
#endif

            Serial.println(F("[WiFiSetup] Disconnected from WiFi, AP restarted"));
        }

        void server_setup::handle_api_status()
        {
            JsonDocument doc;
            doc["connected"] = is_connected();
            doc["ssid"] = m_config.has_value() ? m_config->get_wifi_ssid() : "";
            doc["ip"] = get_ip_address();
#ifdef ESP8266
            doc["rssi"] = WiFi.RSSI();
#elif defined(ESP32)
            doc["rssi"] = WiFi.RSSI();  // На ESP32 возвращает RSSI текущего подключения
#endif
            doc["mode"] = get_mode();

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::handle_api_config()
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
            // Флаг ui_config_initialized указывает, были ли инициализированы настройки интерфейса
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

        void server_setup::handle_api_save()
        {
            Serial.println(F("[WiFiSetup] API: /api/save"));

            bool success = save_settings();

            if (success) {
                send_success_response("Settings saved. Switching to content server...");
                // Переключаемся на сервер контента с новыми настройками
                m_pending_content_cb = true;
                m_pending_cb_counter = PENDING_CB_TICKS;
            } else {
                send_error_response("Failed to save settings");
            }
        }

        void server_setup::handle_api_reset()
        {
            Serial.println(F("[WiFiSetup] API: /api/reset"));

            bool success = reset_settings();

            if (success) {
                // Отправляем ответ клиенту
                send_success_response("Settings reset. Switching to settings server...");
                // Менеджер выполнит сброс и запустит сервер настроек заново
                m_pending_factory_reset_cb = true;
                m_pending_cb_counter = PENDING_CB_TICKS;
            } else {
                send_error_response("Failed to reset settings");
            }
        }

        void server_setup::handle_api_back()
        {
            Serial.println(F("[WiFiSetup] API: /api/back - scheduling content callback"));

            // Отправляем успешный ответ клиенту
            send_success_response("Switching to content server");

            // Устанавливаем отложенный флаг — callback выполнится через N тиков
            m_pending_content_cb = true;
            m_pending_cb_counter = PENDING_CB_TICKS;
        }

        void server_setup::handle_api_ap_settings()
        {
            Serial.println(F("[WiFiSetup] API: /api/ap_settings"));

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

                Serial.println(F("[WiFiSetup] AP restarted, client should reconnect"));
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::handle_api_ui_settings()
        {
            Serial.println(F("[WiFiSetup] API: /api/ui_settings"));

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
                Serial.println(F("[WiFiSetup] UI settings updated"));
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::setup_http_routes()
        {
            Serial.println(F("[WiFiSetup] Setting up HTTP routes..."));

            // Главная страница
            m_server->on("/", HTTP_GET, [this]() {
                Serial.println(F("[WiFiSetup] Request: /"));
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
                Serial.println(F("[WiFiSetup] Request: /api/scan"));
                handle_api_scan();
            });
            m_server->on("/api/connect", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/connect"));
                handle_api_connect();
            });
            m_server->on("/api/disconnect", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/disconnect"));
                handle_api_disconnect();
            });
            m_server->on("/api/status", HTTP_GET, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/status"));
                handle_api_status();
            });
            m_server->on("/api/config", HTTP_GET, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/config"));
                handle_api_config();
            });
            m_server->on("/api/save", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/save"));
                handle_api_save();
            });
            m_server->on("/api/reset", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/reset"));
                handle_api_reset();
            });
            m_server->on("/api/back", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/back"));
                handle_api_back();
            });
            m_server->on("/api/ap_settings", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/ap_settings"));
                handle_api_ap_settings();
            });
            m_server->on("/api/ui_settings", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/ui_settings"));
                handle_api_ui_settings();
            });

            // Обработчик для остальных путей - 404
            m_server->onNotFound([this]() {
                Serial.print(F("[WiFiSetup] Request 404: "));
                Serial.println(m_server->uri());
                m_server->send(404, "text/plain", "Not Found");
            });
        }

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_wifi_setup: no implementation for this platform")
    // Пустой файл по умолчанию
#endif
