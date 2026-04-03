/**
 * @file etl_webui_base.cpp
 * @brief Реализация базового класса для веб-серверов WebUI
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui_base.h"
#include "etl_webui.h"  // Для server_setup

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

        web_server_base_t::~web_server_base_t()
        {
            stop();
        }

        void web_server_base_t::set_on_settings_callback(on_settings_callback_t cb)
        {
            m_on_settings_cb = cb;
        }

        void web_server_base_t::set_on_content_callback(on_content_callback_t cb)
        {
            m_on_content_cb = cb;
        }

        void web_server_base_t::set_on_factory_reset_callback(on_factory_reset_t cb)
        {
            m_on_factory_reset_cb = cb;
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

        bool web_server_base_t::is_initialized() const
        {
            return m_initialized;
        }

         connection_status_t web_server_base_t::get_connection_status() const
        {
            return m_connection_status;
        }

        bool web_server_base_t::is_connected() const
        {
            return WiFi.status() == WL_CONNECTED;
        }

        String web_server_base_t::get_ip_address() const
        {
            if (WiFi.status() == WL_CONNECTED) {
                return WiFi.localIP().toString();
            }
            return WiFi.softAPIP().toString();
        }

        String web_server_base_t::get_mode() const
        {
            // Проверяем активные интерфейсы напрямую
            WiFiMode_t mode = WiFi.getMode();
            bool ap_active = (mode == WIFI_AP || mode == WIFI_AP_STA);
            bool sta_connected = (WiFi.status() == WL_CONNECTED);

            if (ap_active && sta_connected) return "AP+STA";
            if (sta_connected) return "STA";
            if (ap_active) return "AP";
            return "OFF";
        }

        String web_server_base_t::get_hostname() const
        {
            return m_config.has_value() ? String(m_config->hostname) : "espdevice";
        }

        uint16_t web_server_base_t::get_port() const
        {
            return m_config.has_value() ? m_config->port : 80;
        }

        bool web_server_base_t::begin(const device_info_t& device_info)
        {
            Serial.println(F("[WiFiSetup] Initializing..."));

            // Сохранение информации об устройстве
            m_device_info = device_info;

            // Попытка загрузки сохранённых настроек
            if (load_settings()) {
                Serial.println(F("[WiFiSetup] Loaded saved settings"));

                // Если есть сохранённые настройки, пробуем подключиться
                if (m_config.has_value() && m_config->wifi_ssid[0] != '\0') {
                    Serial.print(F("[WiFiSetup] Connecting to saved network: "));
                    Serial.println(m_config->get_wifi_ssid());

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

        void web_server_base_t::stop()
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

        void web_server_base_t::tick()
        {
            // Если сервер не инициализирован, ничего не делаем
            if (!m_initialized) {
                return;
            }

            handle();
            handle_client();
        }

        void web_server_base_t::handle()
        {
            if (!m_initialized) {
                return;
            }

            // Обновление статуса подключения
            update_connection_status();

            // Перезапуск HTTP сервера после подключения к STA (если нужно)
            static bool http_server_restarted = false;
            static uint32_t connection_time = 0;

            if (is_connected() && !http_server_restarted) {
                // Запоминаем время подключения для задержки
                if (connection_time == 0) {
                    connection_time = millis();
                }

                // Ждем 5 секунд после подключения перед перезапуском сервера
                // Это даст время клиенту получить ответ и завершить текущие запросы
                // Клиент успеет получить ответ и перерисовать UI
                if (millis() - connection_time > 5000) {
                    Serial.println(F("[WiFiSetup] Restarting HTTP server after STA connection..."));

                    // Перезапуск сервера в режиме AP+STA
                    if (m_server) {
                        m_server->stop();
                        m_server.reset();
                    }
                    start_http_server();

                    http_server_restarted = true;
                    connection_time = 0;
                }
            }

            // Сброс флага при отключении
            if (!is_connected()) {
                http_server_restarted = false;
                connection_time = 0;
            }
        }

        void web_server_base_t::handle_client()
        {
            if (m_server) {
#ifdef ESP8266
                MDNS.update();
#endif
                m_server->handleClient();
            }

            // Обработка отложенных callback'ов (после завершения обработки запроса)
            if (m_pending_settings_cb) {
                m_pending_settings_cb = false;
                if (m_on_settings_cb) {
                    Serial.println(F("[WebUI] Executing pending settings callback"));
                    m_on_settings_cb();
                }
            }
            if (m_pending_content_cb) {
                m_pending_content_cb = false;
                if (m_on_content_cb) {
                    Serial.println(F("[WebUI] Executing pending content callback"));
                    m_on_content_cb();
                }
            }
            if (m_pending_factory_reset_cb) {
                m_pending_factory_reset_cb = false;
                if (m_on_factory_reset_cb) {
                    Serial.println(F("[WebUI] Executing pending factory reset callback"));
                    m_on_factory_reset_cb();
                }
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

        int32_t web_server_base_t::scan_networks(std::vector<scan_result_t>& results)
        {
            Serial.println(F("[WiFiSetup] Scanning networks..."));

            results.clear();

            // Сохранение текущего режима
            WiFiMode_t current_mode = WiFi.getMode();

            // Для сканирования нужен режим STA, но если мы в AP, переключаемся в AP+STA
            if (current_mode == WIFI_AP) {
                WiFi.mode(WIFI_AP_STA);
                delay(50);  // Ждём переключения режима
            } else if (current_mode == WIFI_STA) {
                // Уже в STA, ничего не делаем
            }
            // Если уже AP_STA или OFF, оставляем как есть

            // Асинхронное сканирование с ожиданием
            int32_t count = WiFi.scanNetworks(false, true);  // async=false, show_hidden=true

            // Небольшая задержка для завершения сканирования
            delay(100);

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

        bool web_server_base_t::connect_to_network(const String& ssid, const String& password, uint32_t timeout)
        {
            Serial.print(F("[WiFiSetup] Connecting to network: "));
            Serial.println(ssid);

            // Сохранение настроек через setter'ы
            if (!m_config.has_value()) {
                m_config = server_config_t();
            }
            m_config->set_wifi_ssid(ssid);
            m_config->set_wifi_password(password);

            return connect_to_sta(timeout);
        }

        void web_server_base_t::connect_to_network_async(const String& ssid, const String& password)
        {
            Serial.print(F("[WiFiSetup] Starting async connection to: "));
            Serial.println(ssid);

            // Сохранение настроек через setter'ы
            if (!m_config.has_value()) {
                m_config = server_config_t();
            }
            m_config->set_wifi_ssid(ssid);
            m_config->set_wifi_password(password);

            // Установка режима STA для подключения
            WiFi.mode(WIFI_STA);

            // Начинаем подключение (не ждём завершения)
            WiFi.begin(m_config->get_wifi_ssid().c_str(), m_config->get_wifi_password().c_str());

            Serial.println(F("[WiFiSetup] Async connection started"));
        }

        String web_server_base_t::get_device_icon() const
        {
            if (m_device_info.icon_svg.length() > 0) {
                return m_device_info.icon_svg;
            }

            // Иконка умного устройства по умолчанию
            return F("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 512 512\"><path d=\"M 256 0 C 114.6 0 0 114.6 0 256 S 114.6 512 256 512 S 512 397.4 512 256 S 397.4 0 256 0 Z M 256 480 C 132.3 480 32 379.7 32 256 S 132.3 32 256 32 S 480 132.3 480 256 S 379.7 480 256 480 Z\" fill=\"#007AFF\"/><path d=\"M 256 128 C 203.1 128 160 171.1 160 224 V 320 C 160 353.1 186.9 380 220 380 H 292 C 325.1 380 352 353.1 352 320 V 224 C 352 171.1 308.9 128 256 128 Z M 320 320 C 320 335.4 307.4 348 292 348 H 220 C 204.6 348 192 335.4 192 320 V 224 C 192 188.8 220.8 160 256 160 S 320 188.8 320 224 V 320 Z\" fill=\"#007AFF\"/><circle cx=\"256\" cy=\"256\" r=\"48\" fill=\"#007AFF\"/></svg>");
        }

        void web_server_base_t::update_connection_status()
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

        String web_server_base_t::get_encryption_type(uint8_t type) const
        {
#ifdef ESP8266
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
#elif defined(ESP32)
            switch (type) {
                case WIFI_AUTH_OPEN:
                    return "Open";
                case WIFI_AUTH_WEP:
                    return "WEP";
                case WIFI_AUTH_WPA_PSK:
                    return "WPA";
                case WIFI_AUTH_WPA2_PSK:
                    return "WPA2";
                case WIFI_AUTH_WPA_WPA2_PSK:
                    return "WPA/WPA2";
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    return "WPA2-Enterprise";
                default:
                    return "Unknown";
            }
#endif
        }

        bool web_server_base_t::start_ap()
        {
            Serial.print(F("[WiFiSetup] Starting AP: "));
            Serial.println(m_config.has_value() ? m_config->get_ap_ssid() : "ESP_Device_AP");

            // Установка режима AP
            WiFi.mode(WIFI_AP);

            // Запуск точки доступа
            if (!WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                             m_config.has_value() ? m_config->get_ap_password().c_str() : "password123")) {
                Serial.println(F("[WiFiSetup] Failed to start AP"));
                return false;
            }

            Serial.print(F("[WiFiSetup] AP IP address: "));
            Serial.println(WiFi.softAPIP());

            m_connection_status = connection_status_t::disconnected;
            return true;
        }

        bool web_server_base_t::connect_to_sta(uint32_t timeout)
        {
            if (!m_config.has_value() || m_config->wifi_ssid[0] == '\0') {
                Serial.println(F("[WiFiSetup] No SSID configured"));
                return false;
            }

            Serial.print(F("[WiFiSetup] Connecting to "));
            Serial.println(m_config->get_wifi_ssid());

            // Сохранение текущего режима
            WiFiMode_t previous_mode = WiFi.getMode();

            // Установка режима STA для подключения
            WiFi.mode(WIFI_STA);

            // Подключение к сети
            WiFi.begin(m_config->get_wifi_ssid().c_str(), m_config->get_wifi_password().c_str());

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

                // Переключение в режим AP+STA для одновременной работы AP и STA
                // Это нужно для работы HTTP сервера в режиме точки доступа
                WiFi.mode(WIFI_AP_STA);

                // Запуск точки доступа, если она не была активна
                if (previous_mode != WIFI_AP && previous_mode != WIFI_AP_STA) {
                    WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                                m_config.has_value() ? m_config->get_ap_password().c_str() : "password123");
                }

                m_connection_status = connection_status_t::connected;
                return true;
            }

            Serial.println(F("\n[WiFiSetup] Connection timeout"));
            m_connection_status = connection_status_t::error;

            // Возврат в предыдущий режим
            WiFi.mode(previous_mode);
            if (previous_mode == WIFI_AP || previous_mode == WIFI_AP_STA) {
                if (!WiFi.softAP(m_config.has_value() ? m_config->get_ap_ssid().c_str() : "ESP_Device_AP",
                                 m_config.has_value() ? m_config->get_ap_password().c_str() : "password123")) {
                    Serial.println(F("[WiFiSetup] Failed to restart AP"));
                } else {
                    Serial.println(F("[WiFiSetup] AP restarted"));
                }
            }

            return false;
        }

        void web_server_base_t::send_scan_response()
        {
            JsonDocument doc;
            JsonArray networks = doc["networks"].to<JsonArray>();

            for (const auto& network : m_scan_cache) {
                JsonObject net = networks.add<JsonObject>();
                net["ssid"] = network.ssid;
                net["rssi"] = network.rssi;
                net["encryption"] = network.encryption;
                net["channel"] = network.channel;
                // Помечаем текущую подключенную сеть
                net["connected"] = (network.ssid == (m_config.has_value() ? m_config->get_wifi_ssid() : "")) && is_connected();
            }

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        void web_server_base_t::send_success_response(const String& message, const String& extra_data)
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

        void web_server_base_t::send_error_response(const String& message)
        {
            JsonDocument doc;
            doc["success"] = false;
            doc["message"] = message;

            String response;
            serializeJson(doc, response);
            m_server->send(200, "application/json", response);
        }

        // ============================================================================
        // Реализация web_manager
        // ============================================================================

        web_manager::web_manager(const device_info_t& device_info)
            : m_device_info(device_info)
        {
        }

        web_manager::~web_manager()
        {
            // Остановка сервера при уничтожении менеджера
            if (m_server) {
                m_server->stop();
                m_server.reset();
            }
        }

        void web_manager::start_content()
        {
            Serial.println(F("[WebManager] Starting content server..."));

            // Остановка текущего сервера — сначала обнуляем указатель,
            // чтобы сервер не мог быть использован во время остановки
            etl::shared_ptr<web_server_base_t> old_server;
            if (m_server) {
                Serial.println(F("[WebManager] Stopping current server..."));
                old_server = m_server;
                m_server.reset();  // Обнуляем ДО остановки
            }

            // Останавливаем старый сервер (без m_server менеджер уже безопасен)
            if (old_server) {
                old_server->stop();
            }

            // Создание и запуск сервера контента
            // Callback'и настраиваются внутри on_create_content()
            m_server = on_create_content();
            if (m_server) {
                if (m_server->begin(m_device_info)) {
                    Serial.println(F("[WebManager] Content server started"));
                } else {
                    Serial.println(F("[WebManager] Failed to start content server"));
                    m_server.reset();
                }
            } else {
                Serial.println(F("[WebManager] on_create_content returned null"));
            }
        }

        void web_manager::start_settings()
        {
            Serial.println(F("[WebManager] Starting settings server..."));

            // Остановка текущего сервера — сначала обнуляем указатель
            etl::shared_ptr<web_server_base_t> old_server;
            if (m_server) {
                Serial.println(F("[WebManager] Stopping current server..."));
                old_server = m_server;
                m_server.reset();  // Обнуляем ДО остановки
            }

            // Останавливаем старый сервер
            if (old_server) {
                old_server->stop();
            }

            // Создание и запуск сервера настроек
            // Callback'и настраиваются внутри on_create_settings()
            m_server = on_create_settings();
            if (m_server) {
                if (m_server->begin(m_device_info)) {
                    Serial.println(F("[WebManager] Settings server started"));
                } else {
                    Serial.println(F("[WebManager] Failed to start settings server"));
                    m_server.reset();
                }
            } else {
                Serial.println(F("[WebManager] on_create_settings returned null"));
            }
        }

        void web_manager::toggle()
        {
            Serial.println(F("[WebManager] Toggle servers..."));

            // Проверяем тип текущего сервера для переключения
            // Если сервера нет или это не server_setup, запускаем настройки
            bool is_settings = false;
            if (m_server) {
                // Проверяем, является ли текущий сервер сервером настроек
                // Это можно определить по наличию m_initialized и типу сервера
                // Простой способ: если сервер инициализирован и это server_setup
                is_settings = (m_server->get_mode() == "AP");  // Сервер настроек обычно в AP режиме
            }

            if (is_settings) {
                start_content();
            } else {
                start_settings();
            }
        }

        void web_manager::tick()
        {
            // Проверяем наличие сервера перед вызовом tick()
            if (m_server) {
                m_server->tick();
            }
        }

        etl::shared_ptr<web_server_base_t> web_manager::on_create_settings()
        {
            // Создание сервера настроек по умолчанию
            auto web_config = settings::load_wifi_config();
            auto server = etl::make_shared<etl::webui::server_setup>(
                web_config.has_value() ? web_config.value() : server_config_t()
            );
            return server;
        }

        bool web_manager::trace_connection() const
        {
            auto server = get_server();
            if(server && server->is_initialized()) {
                // Определяем тип сервера
                const char* server_type = "Content";
                // Если сервер работает в режиме AP - это сервер настроек
                if (server->get_mode() == "AP") {
                    server_type = "Setup";
                }

                const String& ip_addr = server->get_ip_address();
                const String& hostname_cfg = server->get_wifi_config().has_value() ? 
                                             server->get_wifi_config()->get_hostname() : "espdevice";
                const String& mode = server->get_mode();

                Serial.println(F("\n=== WiFi Server Info ==="));
                Serial.print  (F("Server:   ")); Serial.println(server_type);
                Serial.print  (F("Mode:     ")); Serial.println(mode);
                Serial.print  (F("IP Addr:  ")); Serial.println(ip_addr.length() > 0 ? ip_addr : F("(AP IP: 192.168.4.1)"));
                Serial.print  (F("Hostname: http://"));
                if (mode == "AP") {
                    Serial.println(F("192.168.4.1"));
                } else {
                    Serial.print(hostname_cfg);
                    Serial.println(F(".local"));
                }
                Serial.print  (F("mDNS:     http://")); Serial.print  (hostname_cfg); Serial.println(F(".local"));
                Serial.println(F("=========================\n"));
                return true;
            }
            else {
                Serial.println(F("[ERROR] WiFi server initialization failed!"));
                return false;
            }
        }
    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
