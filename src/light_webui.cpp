/**
 * @file light_webui.cpp
 * @brief Реализация сервера управления светодиодной лампой
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 *
 * Сервер контента - ТОЛЬКО ЧТЕНИЕ настроек, управление лампой и показ статуса.
 * Настройки WiFi меняются через server_setup.
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"
#include "light_webui.h"
#include "light_webui_html.h"
#include "light_mqtt.h"

namespace light_control
{
    namespace data
    {
        // настройки управленя светом
        const String    _path = "/settings/light_control.cfg";
        const uint16_t  _update_delay = 30000;  // 0ms - Immediately update, 30s - данные обновятся, если не было изменений в течении 30с
        const String    _trace_name = "light_control::data";    // Имя для трассировки в Serial
        
        // Определяем глобальный экземпляр данных
        etl::settings::app_data<kitchen_light_t>& app()
        {
            static etl::settings::app_data<kitchen_light_t> app_data(_path, _update_delay, _trace_name);   
            return app_data;
        }

    } // namespace data

    namespace webui
    {
        // ============================================================================
        // Реализация light_control_server
        // ============================================================================

        etl::webui::device_info_t get_light_control_device_info()
        {
            etl::webui::device_info_t info;
            info.name = "Рабочая зона";
            info.description = String("v") + APP_VERSION_STRING;
            info.icon_svg = LIGHT_DEVICE_ICON_SVG;
            return info;
        }

        light_control_server::light_control_server(const etl::optional<etl::webui::server_config_t>& cfg)
            : etl::webui::web_server_base_t(cfg)
        {
        }

        void light_control_server::set_light_settings(const data::kitchen_light_t& settings)
        {
            data::app().set(settings, etl::settings::sender_id::webui);
        }

        data::kitchen_light_t light_control_server::get_light_settings() const
        {
            if(auto settings = data::app().get(); settings) {
                return *settings;
            }
            // Возвращаем значения по умолчанию, если данные не инициализированы
            return data::kitchen_light_t{};
        }

        void light_control_server::set_power(bool power)
        {
            if(auto current = data::app().get(); current) {
                data::kitchen_light_t updated = *current;
                updated.power = power;
                data::app().set(updated, etl::settings::sender_id::webui);
                send_state_to_serial(updated.power, updated.brightness);
            }
        }

        bool light_control_server::get_power() const
        {
            if(auto current = data::app().get(); current) {
                return current->power;
            }
            return false; // Значение по умолчанию
        }

        void light_control_server::set_brightness(float brightness)
        {
            if(auto current = data::app().get(); current) {
                data::kitchen_light_t updated = *current;
                // Ограничение диапазона [1..100]
                updated.brightness = constrain(brightness, 1.0f, 100.0f);
                data::app().set(updated, etl::settings::sender_id::webui);
                send_state_to_serial(updated.power, updated.brightness);
            }
        }

        float light_control_server::get_brightness() const
        {
            if(auto current = data::app().get(); current) {
                return current->brightness;
            }
            return 100.0f; // Значение по умолчанию
        }

        void light_control_server::send_state_to_serial(bool power, float brightness)
        {
            Serial.print(F("[LightControl] State: power="));
            Serial.print(power ? "ON" : "OFF");
            Serial.print(F(", brightness="));
            Serial.println(brightness, 1);
        }

        void light_control_server::stop()
        {
            Serial.println(F("[LightControl] stop() - cleaning up subscriptions"));
            
            // Отписка от изменений настроек
            if(m_subscribed) {
                data::app().unsubscribe(etl::settings::sender_id::webui);
                m_subscribed = false;
                Serial.println(F("[LightControl] Unsubscribed from settings changes"));
            }
            
            // Вызов базовой реализации
            etl::webui::web_server_base_t::stop();
        }

        // ============================================================================
        // Инициализация сервера контента
        // ============================================================================

        bool light_control_server::begin(const etl::webui::device_info_t& device_info)
        {
            Serial.println(F("[LightControl] begin()..."));

            // Сохраняем информацию об устройстве
            m_device_info = device_info;

            // Загружаем настройки WiFi из FS (только чтение)
            auto saved_wifi = etl::webui::settings::load_wifi_config();
            if (saved_wifi.has_value()) {
                m_config = saved_wifi;
                Serial.println(F("[LightControl] WiFi settings loaded"));
            }

            // Загружаем настройки UI из FS (только чтение)
            auto saved_ui = etl::webui::settings::load_ui_config();
            if (saved_ui.has_value()) {
                m_ui_config = saved_ui;
                Serial.println(F("[LightControl] UI settings loaded"));
            }

            // Подписка на изменения настроек от других источников (не webui)
            // Если изменения пришли не от webui, обновляем данные в веб-интерфейсе
            m_subscribed = data::app().subscribe(
                etl::settings::sender_id::webui,
                [this](etl::settings::sender_id source) {
                    Serial.printf("[LightControl] Settings changed by source: %d (webui will be updated)\n", static_cast<uint8_t>(source));
                    // Данные изменились, веб-интерфейс получит их при следующем запросе через handle_api_state()
                    // Дополнительных действий не требуется, т.к. данные хранятся глобально
                    if(auto value = data::app().get(); value)
                    {
                        Serial.printf("[LightControl] will set to webui: power=%s, brightness,🔆=%d\n", value->power ? "✅" : "🔳", int(value->brightness));
                        // Если изменение пришло НЕ от webui — запоминаем время для блокировки "эха"
                        if (source != etl::settings::sender_id::webui) {
                            m_update_UI = true; // обновить интерфейс и не слать обратно в сервер изменения до таймаута, чтобы избежать кольца
                        }                        
                    }
                }
            );

            if(m_subscribed) {
                Serial.println(F("[LightControl] Subscribed to settings changes"));
            } else {
                Serial.println(F("[LightControl] Failed to subscribe to settings changes"));
            }

            // Попытка подключения к WiFi
            connect_from_saved_config();

            // Запуск HTTP сервера
            m_initialized = true;
            start_http_server();

            return true;
        }

        // ============================================================================
        // Подключение к WiFi из сохранённых настроек (только чтение)
        // ============================================================================

        void light_control_server::connect_from_saved_config()
        {
            Serial.println(F("[LightControl] Loading WiFi config from settings..."));

            // Считываем сохранённые настройки
            auto saved_cfg = etl::webui::settings::load_wifi_config();
            if (saved_cfg.has_value() && saved_cfg->wifi_ssid[0] != '\0') {
                m_config = saved_cfg;
                Serial.print(F("[LightControl] Found saved WiFi config, SSID: "));
                Serial.println(m_config->get_wifi_ssid());

#ifdef ESP8266
                Serial.print(F("[LightControl] Free heap before connect: "));
                Serial.println(ESP.getFreeHeap());
#endif

                // Попытка подключения
                Serial.print(F("[LightControl] Connecting to "));
                Serial.print(m_config->get_wifi_ssid());
                Serial.print(F("..."));

                WiFi.mode(WIFI_STA);
                WiFi.begin(m_config->get_wifi_ssid().c_str(), m_config->get_wifi_password().c_str());

                uint32_t start = millis();
                const uint32_t timeout = 10000;

                while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
                    delay(500);
                    Serial.print(F("."));
                }

                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println(F(" OK"));
                    Serial.print(F("[LightControl] IP: "));
                    Serial.println(WiFi.localIP());
                    m_connection_status = etl::webui::connection_status_t::connected;
                } else {
                    Serial.println(F(" FAILED"));
                    m_connection_status = etl::webui::connection_status_t::error;
                }

#ifdef ESP8266
                Serial.print(F("[LightControl] Free heap after connect: "));
                Serial.println(ESP.getFreeHeap());
#endif
            } else {
                Serial.println(F("[LightControl] No saved WiFi config, using AP mode"));
            }

            // Если не подключились - запускаем AP
            if (!is_connected()) {
                WiFi.mode(WIFI_AP);
                if (start_ap()) {
                    Serial.println(F("[LightControl] AP mode started"));
                }
            }
        }

        // ============================================================================
        // HTTP обработчики
        // ============================================================================

        void light_control_server::start_http_server()
        {
#ifdef ESP8266
            Serial.println(F("[LightControl] Init (ESP8266)..."));
            Serial.print(F("[LightControl] Free heap before: "));
            Serial.println(ESP.getFreeHeap());
#endif

            Serial.println(F("[LightControl] Starting HTTP server..."));

            // Создание сервера через shared_ptr
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

            // MQTT менеджер для управления светом
            if (auto light_mqtt_mgr = etl::mqtt::get_light_mqtt_mgr(); light_mqtt_mgr && light_mqtt_mgr->is_connected()) {
                doc["mqtt"] = "connected";
            }
            else {
                doc["mqtt"] = "error";
            }
            Serial.print(F("[light_control_server::handle_api_status] "));
            Serial.println(doc["mqtt"].as<const char*>());

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
            m_server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            m_server->sendHeader("Pragma", "no-cache");
            m_server->sendHeader("Expires", "0");

            JsonDocument doc;
            if(auto current = data::app().get(); current) {
                bool update = m_update_UI; m_update_UI = false;    // reset after sending                
                doc["power"] = current->power;
                doc["brightness"] = current->brightness;
                doc["update"] = update;
                // Отладка: лог в Serial
                Serial.printf("[LightControl] /api/state: power=%d, brightness=%.1f, update=%s\n", current->power, current->brightness, update ? "true" : "false");
            } else {
                // Значения по умолчанию
                doc["power"] = false;
                doc["brightness"] = 100.0f;
                doc["update"] = false;
            }

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

                // Получаем текущие настройки
                data::kitchen_light_t updated_settings;
                if(auto current = data::app().get(); current) {
                    updated_settings = *current;
                }

                // Обновление состояния питания
                if (doc["power"].is<bool>()) {
                    updated_settings.power = doc["power"].as<bool>();
                }

                // Обновление яркости
                if (doc["brightness"].is<float>()) {
                    updated_settings.brightness = constrain(doc["brightness"].as<float>(), 1.0f, 100.0f);
                } else if (doc["brightness"].is<int>()) {
                    updated_settings.brightness = constrain(static_cast<float>(doc["brightness"].as<int>()), 1.0f, 100.0f);
                }

                // Сохраняем обновлённые настройки с идентификатором webui
                data::app().set(updated_settings, etl::settings::sender_id::webui);

                // Отправка подтверждения
                JsonDocument resp;
                resp["success"] = true;
                resp["power"] = updated_settings.power;
                resp["brightness"] = updated_settings.brightness;

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

        void light_control_server::handle_api_config()
        {
            JsonDocument doc;
            // Информация об устройстве (из m_device_info)
            doc["device_name"] = m_device_info.name;
            doc["device_description"] = m_device_info.description;
            doc["device_icon_svg"] = m_device_info.icon_svg;

            // WiFi конфигурация (из m_config, только чтение)
            if (m_config.has_value()) {
                doc["hostname"] = m_config->get_hostname();
                doc["ap_ssid"] = m_config->get_ap_ssid();
                doc["ap_password"] = m_config->get_ap_password();
                doc["port"] = m_config->port;
                doc["wifi_ssid"] = m_config->get_wifi_ssid();
            }

            // Настройки интерфейса (из m_ui_config, только чтение)
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

            // API endpoints (только чтение + управление лампой)
            m_server->on("/api/status", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/status"));
                handle_api_status();
            });
            m_server->on("/api/config", HTTP_GET, [this]() {
                Serial.println(F("[LightControl] Request: /api/config"));
                handle_api_config();
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

            // Обработчик для остальных путей - 404
            m_server->onNotFound([this]() {
                Serial.print(F("[LightControl] Request 404: "));
                Serial.println(m_server->uri());
                m_server->send(404, "text/plain", "Not Found");
            });
        }

    } // namespace webui
} // namespace light_control

#else
    #pragma message("light_webui: no implementation for this platform")
#endif
