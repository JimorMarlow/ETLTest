/**
 * @file etl_wifi_setup.cpp
 * @brief Реализация WiFi Setup Server для ESP8266
 *
 * Платформа: ESP8266 (NodeMCU v3)
 */

#if defined(ESP8266)

#include "etl_wifi_setup.h"
#include "etl_wifi_setup_html.h"

#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"

// :DEBUG: Для отладки падений исключаю запись на карту памяти
#define EXCLUDE_SETTINGS_DATA

namespace etl
{
    namespace wifi
    {
        // Константы для хранения настроек
        static const uint32_t WIFI_CONNECT_TIMEOUT = 10000;  // 10 секунд

        namespace settings
        {
            const String    data_path = "/settings/wifi.cfg";
            const uint16_t  data_update_delay = 0;  // 0ms - Immiaditly update
            server_config_t default_wifi_cfg;       // Значение по-умолчанию для сброса к заводским значениям

            #ifdef EXCLUDE_SETTINGS_DATA
            server_config_t temp_wifi_cfg;       // Значение по-умолчанию для сброса к заводским значениям
            #endif//EXCLUDE_SETTINGS_DATA

            /**
             * @brief Установить значения подключения к точками доступа по умолчанию и считать данные
             * @param cfg Конфигурация WiFi сервера по умолчанию
             */
            bool init_config(const etl::wifi::server_config_t& default_cfg)
            {
                #ifdef EXCLUDE_SETTINGS_DATA
                temp_wifi_cfg = default_cfg;
                return true;
                #endif//#ifdef EXCLUDE_SETTINGS_DATA 

                if(etl::little_fs::begin())
                {
                    // Создание директории для файла настроек
                    etl::little_fs::create_dir(settings::data_path); 
                }

                // Сохранение настроек в постоянной памяти
                etl::settings::data<etl::wifi::server_config_t> data (settings::data_path, settings::data_update_delay, default_cfg);
                return data.init();
            }  

            /**
             * @brief Установить значения подключения к точками доступа
             * @param cfg Конфигурация WiFi сервера
             */
            bool save_config(const server_config_t& cfg)
            {
                #ifdef EXCLUDE_SETTINGS_DATA
                temp_wifi_cfg = cfg;
                return true;
                #endif//#ifdef EXCLUDE_SETTINGS_DATA 

                etl::settings::data<etl::wifi::server_config_t> data (settings::data_path, settings::data_update_delay, default_wifi_cfg);
                data.init();
                data.set(cfg);
                return data.save();
            }  

            /**
             * @brief Считать текущие значения подключения к точками доступа
             */
            server_config_t load_config()
            {
            #ifdef EXCLUDE_SETTINGS_DATA
                return temp_wifi_cfg;
            #endif//#ifdef EXCLUDE_SETTINGS_DATA 
                server_config_t cfg = default_wifi_cfg; 
                etl::settings::data<etl::wifi::server_config_t> data (settings::data_path, settings::data_update_delay, default_wifi_cfg);
                if(data.init())
                {
                    cfg = data.get();
                    
                    // device info update from actual default
                    cfg.device_name = default_wifi_cfg.device_name;
                    cfg.device_description = default_wifi_cfg.device_description;
                    cfg.device_icon_svg = default_wifi_cfg.device_icon_svg;
                }
                
                return cfg;
            } 
        }

        server_setup::server_setup(const server_config_t& cfg)
            : m_config(cfg)
        {
        }

        server_setup::~server_setup()
        {
            stop();
        }

        bool server_setup::begin()
        {
            Serial.println(F("[WiFiSetup] Initializing..."));

            // Попытка загрузки сохранённых настроек
            if (load_settings()) {
                Serial.println(F("[WiFiSetup] Loaded saved settings"));

                // Если есть сохранённые настройки, пробуем подключиться
                if (m_config.wifi_ssid.length() > 0) {
                    Serial.print(F("[WiFiSetup] Connecting to saved network: "));
                    Serial.println(m_config.wifi_ssid);

                    if (connect_to_sta(WIFI_CONNECT_TIMEOUT)) {
                        Serial.println(F("[WiFiSetup] Connected to saved network"));
                        m_initialized = true;
                        start_http_server();
                        return true;
                    } else {
                        Serial.println(F("[WiFiSetup] Failed to connect to saved network"));
                    }
                }
            }

            // Запуск в режиме точки доступа
            Serial.println(F("[WiFiSetup] Starting AP mode..."));
            if (start_ap()) {
                Serial.println(F("[WiFiSetup] AP started successfully"));
                m_initialized = true;
                start_http_server();
                return true;
            }

            Serial.println(F("[WiFiSetup] Failed to start AP"));
            return false;
        }

        void server_setup::start_http_server()
        {
            Serial.println(F("[WiFiSetup] Starting HTTP server..."));

            // Создание сервера через shared_ptr
            m_server = etl::make_shared<ESP8266WebServer>(m_config.port);

            // Настройка роутинга
            setup_http_routes();

            // Запуск сервера
            m_server->begin();
            Serial.print(F("[WiFiSetup] HTTP server started on port "));
            Serial.println(m_config.port);

            // mDNS
            if (MDNS.begin(m_config.hostname.c_str())) {
                Serial.print(F("[WiFiSetup] mDNS: http://"));
                Serial.print(m_config.hostname);
                Serial.println(F(".local"));
                MDNS.addService("http", "tcp", m_config.port);
            } else {
                Serial.println(F("[WiFiSetup] mDNS failed"));
            }
        }

        void server_setup::stop()
        {
            if (!m_initialized) {
                return;
            }

            Serial.println(F("[WiFiSetup] Stopping..."));

            // Остановка mDNS
            MDNS.end();

            // Остановка HTTP сервера (shared_ptr автоматически освободит ресурсы)
            if (m_server) {
                m_server->stop();
                m_server.reset();
            }

            // Отключение от WiFi
            WiFi.disconnect(true);

            // Остановка точки доступа
            WiFi.softAPdisconnect(true);

            // Отключение WiFi
            WiFi.mode(WIFI_OFF);

            m_initialized = false;
            m_connection_status = connection_status_t::disconnected;

            Serial.println(F("[WiFiSetup] Stopped"));
        }

        void server_setup::handle()
        {
            if (!m_initialized) {
                return;
            }

            // Обновление статуса подключения
            update_connection_status();
            
            // Перезапуск HTTP сервера после подключения к STA (если нужно)
            static bool http_server_restarted = false;
            if (is_connected() && !http_server_restarted) {
                // Небольшая задержка перед перезапуском сервера
                static uint32_t restart_delay_start = 0;
                if (restart_delay_start == 0) {
                    restart_delay_start = millis();
                }
                
                if (millis() - restart_delay_start > 500) {
                    Serial.println(F("[WiFiSetup] Restarting HTTP server after STA connection..."));
                    
                    // Перезапуск сервера в режиме AP+STA
                    if (m_server) {
                        m_server->stop();
                        m_server.reset();
                    }
                    start_http_server();
                    
                    http_server_restarted = true;
                    restart_delay_start = 0;
                }
            }
            
            // Сброс флага при отключении
            if (!is_connected()) {
                http_server_restarted = false;
            }
        }

        void server_setup::handle_client()
        {
            if (m_server != nullptr) {
                MDNS.update();
                m_server->handleClient();
            }
        }

        bool server_setup::is_initialized() const
        {
            return m_initialized;
        }

        connection_status_t server_setup::get_connection_status() const
        {
            return m_connection_status;
        }

        bool server_setup::is_connected() const
        {
            return WiFi.status() == WL_CONNECTED;
        }

        String server_setup::get_ip_address() const
        {
            if (WiFi.status() == WL_CONNECTED) {
                return WiFi.localIP().toString();
            }
            return WiFi.softAPIP().toString();
        }

        String server_setup::get_mode() const
        {
            WiFiMode_t mode = WiFi.getMode();
            switch (mode) {
                case WIFI_STA: return "STA";
                case WIFI_AP: return "AP";
                case WIFI_AP_STA: return "AP+STA";
                default: return "UNKNOWN";
            }
        }

        int32_t server_setup::scan_networks(std::vector<scan_result_t>& results)
        {
            Serial.println(F("[WiFiSetup] Scanning networks..."));

            results.clear();

            // Переключение в режим STA для сканирования
            WiFiMode_t current_mode = WiFi.getMode();
            WiFi.mode(WIFI_STA);

            int32_t count = WiFi.scanNetworks();

            // Возврат предыдущего режима
            if (current_mode == WIFI_AP) {
                WiFi.mode(WIFI_AP);
            } else if (current_mode == WIFI_AP_STA) {
                WiFi.mode(WIFI_AP_STA);
            }

            if (count == 0) {
                Serial.println(F("[WiFiSetup] No networks found"));
                return 0;
            }

            Serial.printf("[WiFiSetup] Found %d networks\n", count);

            for (int32_t i = 0; i < count; ++i) {
                scan_result_t result;
                result.ssid = WiFi.SSID(i);
                result.rssi = WiFi.RSSI(i);
                result.encryption = get_encryption_type(WiFi.encryptionType(i));
                result.channel = WiFi.channel(i);

                results.push_back(result);

                Serial.printf("[WiFiSetup] Network %d: %s (RSSI: %d, Encryption: %s)\n",
                              i + 1, result.ssid.c_str(), result.rssi, result.encryption.c_str());
            }

            // Сортировка по уровню сигнала (убывание)
            std::sort(results.begin(), results.end(),
                      [](const scan_result_t& a, const scan_result_t& b) {
                          return a.rssi > b.rssi;
                      });

            WiFi.scanDelete();
            return count;
        }

        bool server_setup::connect_to_network(const String& ssid, const String& password, uint32_t timeout)
        {
            Serial.print(F("[WiFiSetup] Connecting to network: "));
            Serial.println(ssid);

            // Сохранение настроек
            m_config.wifi_ssid = ssid;
            m_config.wifi_password = password;

            return connect_to_sta(timeout);
        }

        void server_setup::disconnect()
        {
            Serial.println(F("[WiFiSetup] Disconnecting..."));

            WiFi.disconnect(true);
            m_connection_status = connection_status_t::disconnected;
        }

        bool server_setup::save_settings()
        {
            Serial.println(F("[WiFiSetup] Saving settings..."));
            m_config.trace();
            bool result = settings::save_config(m_config);
            Serial.println(result ? "OK" : "FAILED");
            return result;
        }

        bool server_setup::load_settings()
        {
            // Применение настроек
            m_config = settings::load_config();
            Serial.println(F("[WiFiSetup] Settings loaded"));
            m_config.trace();
            return true;
        }

        bool server_setup::reset_settings()
        {
            Serial.println(F("[WiFiSetup] Resetting settings..."));
            // Сброс конфигурации к значениям по умолчанию
            m_config = settings::default_wifi_cfg;

            Serial.println(F("[WiFiSetup] Settings reset"));
            return settings::save_config(m_config);
        }

        void server_setup::set_config(const server_config_t& cfg)
        {
            m_config = cfg;
        }

        void server_setup::reboot()
        {
            Serial.println(F("[WiFiSetup] Rebooting..."));
            delay(100);
            ESP.reset();
        }

        bool server_setup::start_ap()
        {
            Serial.print(F("[WiFiSetup] Starting AP: "));
            Serial.println(m_config.ap_ssid);

            // Установка режима AP
            WiFi.mode(WIFI_AP);

            // Запуск точки доступа
            if (!WiFi.softAP(m_config.ap_ssid.c_str(), m_config.ap_password.c_str())) {
                Serial.println(F("[WiFiSetup] Failed to start AP"));
                return false;
            }

            Serial.print(F("[WiFiSetup] AP IP address: "));
            Serial.println(WiFi.softAPIP());

            m_connection_status = connection_status_t::disconnected;
            return true;
        }

        bool server_setup::connect_to_sta(uint32_t timeout)
        {
            if (m_config.wifi_ssid.length() == 0) {
                Serial.println(F("[WiFiSetup] No SSID configured"));
                return false;
            }

            Serial.print(F("[WiFiSetup] Connecting to "));
            Serial.println(m_config.wifi_ssid);

            // Сохранение текущего режима
            WiFiMode_t previous_mode = WiFi.getMode();

            // Установка режима STA для подключения
            WiFi.mode(WIFI_STA);

            // Подключение к сети
            WiFi.begin(m_config.wifi_ssid.c_str(), m_config.wifi_password.c_str());

            // Ожидание подключения
            uint32_t start_time = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout) {
                delay(500);
                Serial.print(F("."));
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(F("\n[WiFiSetup] Connected"));
                Serial.print(F("[WiFiSetup] IP address: "));
                Serial.println(WiFi.localIP());

                // Возврат в предыдущий режим (AP или AP+STA) для продолжения работы сервера
                // Переключение в AP+STA будет сделано в handle() после отправки ответа
                if (previous_mode == WIFI_AP) {
                    WiFi.mode(WIFI_AP_STA);
                } else {
                    WiFi.mode(previous_mode);
                }
                
                m_connection_status = connection_status_t::connected;
                return true;
            }

            Serial.println(F("\n[WiFiSetup] Connection timeout"));
            m_connection_status = connection_status_t::error;

            // Возврат в предыдущий режим
            WiFi.mode(previous_mode);
            if (previous_mode == WIFI_AP || previous_mode == WIFI_AP_STA) {
                if (!WiFi.softAP(m_config.ap_ssid.c_str(), m_config.ap_password.c_str())) {
                    Serial.println(F("[WiFiSetup] Failed to restart AP"));
                } else {
                    Serial.println(F("[WiFiSetup] AP restarted"));
                }
            }

            return false;
        }

        void server_setup::update_connection_status()
        {
            wl_status_t status = WiFi.status();

            switch (status) {
                case WL_CONNECTED:
                    m_connection_status = connection_status_t::connected;
                    break;

                case WL_DISCONNECTED:
                case WL_IDLE_STATUS:
                    m_connection_status = connection_status_t::disconnected;
                    break;

                case WL_CONNECT_FAILED:
                case WL_CONNECTION_LOST:
                    m_connection_status = connection_status_t::error;
                    break;

                default:
                    break;
            }
        }

        String server_setup::get_encryption_type(uint8_t type) const
        {
            switch (type) {
                case ENC_TYPE_NONE:
                    return "Open";
                case ENC_TYPE_WEP:
                    return "WEP";
                case ENC_TYPE_TKIP:
                    return "WPA";
                case ENC_TYPE_CCMP:
                    return "WPA2";
                case ENC_TYPE_AUTO:
                    return "WPA/WPA2";
                default:
                    return "Unknown";
            }
        }

        String server_setup::get_device_icon() const
        {
            if (m_config.device_icon_svg.length() > 0) {
                return m_config.device_icon_svg;
            }

            // Иконка умного устройства по умолчанию
            return F("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512 512\"><path d=\"M 256 0 C 114.6 0 0 114.6 0 256 S 114.6 512 256 512 S 512 397.4 512 256 S 397.4 0 256 0 Z M 256 480 C 132.3 480 32 379.7 32 256 S 132.3 32 256 32 S 480 132.3 480 256 S 379.7 480 256 480 Z\" fill=\"#007AFF\"/><path d=\"M 256 128 C 203.1 128 160 171.1 160 224 V 320 C 160 353.1 186.9 380 220 380 H 292 C 325.1 380 352 353.1 352 320 V 224 C 352 171.1 308.9 128 256 128 Z M 320 320 C 320 335.4 307.4 348 292 348 H 220 C 204.6 348 192 335.4 192 320 V 224 C 192 188.8 220.8 160 256 160 S 320 188.8 320 224 V 320 Z\" fill=\"#007AFF\"/><circle cx=\"256\" cy=\"256\" r=\"48\" fill=\"#007AFF\"/></svg>");
        }

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
            scan_networks(m_scan_cache);
            m_scan_timestamp = current_time;

            send_scan_response();
        }

        void server_setup::send_scan_response()
        {
            JsonDocument doc;
            JsonArray networks = doc["networks"].to<JsonArray>();

            for (const auto& network : m_scan_cache) {
                JsonObject net = networks.add<JsonObject>();
                net["ssid"] = network.ssid;
                net["rssi"] = network.rssi;
                net["encryption"] = network.encryption;
                net["channel"] = network.channel;
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
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

                // Подключение к сети (с увеличенным таймаутом для стабильности)
                bool success = connect_to_network(ssid, password, 20000);  // 20 секунд

                if (success) {
                    send_success_response("Connected", get_ip_address());
                } else {
                    send_error_response("Connection failed");
                }
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::handle_api_status()
        {
            JsonDocument doc;
            doc["connected"] = is_connected();
            doc["ssid"] = m_config.wifi_ssid;
            doc["ip"] = get_ip_address();
            doc["rssi"] = WiFi.RSSI();
            doc["mode"] = get_mode();

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::handle_api_config()
        {
            JsonDocument doc;
            doc["device_name"] = m_config.device_name;
            doc["device_description"] = m_config.device_description;
            doc["device_icon_svg"] = m_config.device_icon_svg;
            doc["hostname"] = m_config.hostname;
            doc["ap_ssid"] = m_config.ap_ssid;
            doc["port"] = m_config.port;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::handle_api_save()
        {
            Serial.println(F("[WiFiSetup] API: /api/save"));

            bool success = save_settings();

            if (success) {
                send_success_response("Settings saved");
            } else {
                send_error_response("Failed to save settings");
            }
        }

        void server_setup::handle_api_reset()
        {
            Serial.println(F("[WiFiSetup] API: /api/reset"));

            bool success = reset_settings();

            if (success) {
                send_success_response("Settings reset. Rebooting...");
                Serial.println(F("[WiFiSetup] Rebooting in 2 seconds..."));
                delay(2000);  // Дать время на отправку ответа клиенту
                reboot();
            } else {
                send_error_response("Failed to reset settings");
            }
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

                // Применение настроек AP
                m_config.ap_ssid = ap_ssid;
                m_config.ap_password = ap_password;

                // Перезапуск точки доступа
                WiFi.softAPdisconnect(true);
                start_ap();

                send_success_response("AP settings applied", m_config.ap_ssid);
            } else {
                send_error_response("No data provided");
            }
        }

        void server_setup::send_success_response(const String& message, const String& extra_data)
        {
            JsonDocument doc;
            doc["success"] = true;
            doc["message"] = message;
            if (extra_data.length() > 0) {
                doc["data"] = extra_data;
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void server_setup::send_error_response(const String& message)
        {
            JsonDocument doc;
            doc["success"] = false;
            doc["message"] = message;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
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
            m_server->on("/api/ap_settings", HTTP_POST, [this]() {
                Serial.println(F("[WiFiSetup] Request: /api/ap_settings"));
                handle_api_ap_settings();
            });

            // Обработчик для остальных путей - 404
            m_server->onNotFound([this]() {
                Serial.print(F("[WiFiSetup] Request 404: "));
                Serial.println(m_server->uri());
                m_server->send(404, "text/plain", "Not Found");
            });
        }

    } // namespace wifi
} // namespace etl

#else
    #pragma message("etl_wifi_setup: no implementation for this platform")
    // Пустой файл по умолчанию
#endif
