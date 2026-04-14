/**
 * @file etl_wifi.cpp
 * @brief Реализация менеджера WiFi подключений
 *
 * Платформа: ESP8266 (NodeMCU v3, D1 Mini Lite), ESP32 (C3, WROOM-32U)
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_wifi.h"
#include "etl/etl_settings.h"
#include "etl/etl_littlefs.h"

// Алиас для типа режима WiFi (разный для ESP8266 и ESP32)
#if defined(ESP8266)
    using wifi_mode_type_t = WiFiMode_t;
#elif defined(ESP32)
    using wifi_mode_type_t = wifi_mode_t;
#endif

namespace etl
{
    namespace wifi
    {
        // ============================================================================
        // Конструктор/деструктор
        // ============================================================================

        manager::manager(const etl::webui::server_config_t& config)
            : m_config(config)
        {
            Serial.println(F("[etl::wifi::manager] Constructor"));
        }

        manager::~manager()
        {
            stop();
            Serial.println(F("[etl::wifi::manager] Destructor"));
        }

        // ============================================================================
        // begin() / stop()
        // ============================================================================

        bool manager::begin()
        {
            Serial.println(F("[etl::wifi::manager] begin()"));

            // Загрузка настроек из FS
            auto loaded_config = etl::webui::settings::load_wifi_config();
            if (loaded_config.has_value())
            {
                m_config = loaded_config.value();
                Serial.println(F("[etl::wifi::manager] WiFi config loaded from FS"));
            }
            else
            {
                Serial.println(F("[etl::wifi::manager] No saved WiFi config, using defaults"));
            }

            // Попытка подключиться к сохранённой сети
            if (strlen(m_config.wifi_ssid) > 0)
            {
                Serial.printf("[etl::wifi::manager] Trying to connect to saved network: %s\n", m_config.wifi_ssid);

                if (connect_to_sta(10000))
                {
                    // Успешно подключились к STA - переключаем в AP+STA
                    WiFi.mode(WIFI_AP_STA);
                    m_mode = mode_t::ap_sta;
                    m_status = status_t::ap_sta_mode;

                    // Инициализация mDNS
                    init_mdns(m_config.hostname);

                    notify_status_change(m_status);
                    return true;
                }
            }

            // Не удалось подключиться - запускаем AP
            Serial.println(F("[etl::wifi::manager] Failed to connect to STA, starting AP mode"));
            if (start_ap())
            {
                m_mode = mode_t::ap;
                m_status = status_t::ap_mode;
                notify_status_change(m_status);
                return true;
            }

            m_status = status_t::error;
            notify_status_change(m_status);
            return false;
        }

        void manager::stop()
        {
            Serial.println(F("[etl::wifi::manager] stop()"));

            // Остановка mDNS
            stop_mdns();

            // Отключение от WiFi
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);

            m_status = status_t::disconnected;
            m_mode = mode_t::none;
            m_reconnect_attempts = 0;
            m_reconnect_pending = false;
        }

        // ============================================================================
        // tick()
        // ============================================================================

        void manager::tick()
        {
            // Обновление статуса
            update_status();

            // Проверка необходимости переподключения
            if (m_reconnect_pending && (m_status == status_t::disconnected || m_status == status_t::error))
            {
                uint32_t now = millis();
                if (now - m_last_reconnect_time >= RECONNECT_DELAY_MS)
                {
                    attempt_reconnect();
                }
            }

            // Мониторинг разрыва STA соединения
            if (m_mode == mode_t::sta || m_mode == mode_t::ap_sta)
            {
                if (WiFi.status() != WL_CONNECTED && m_status != status_t::connecting)
                {
                    // Соединение потеряно
                    Serial.println(F("[etl::wifi::manager] WiFi connection lost"));
                    m_status = status_t::disconnected;
                    notify_status_change(m_status);

                    // Начинаем переподключение
                    m_reconnect_attempts = 0;
                    m_reconnect_pending = true;
                    attempt_reconnect();
                }
            }
        }

        // ============================================================================
        // connect() / disconnect()
        // ============================================================================

        bool manager::connect(const String& ssid, const String& password, uint32_t timeout)
        {
            Serial.printf("[etl::wifi::manager] connect() to SSID: %s\n", ssid.c_str());

            // Обновляем конфигурацию
            m_config.set_wifi_ssid(ssid);
            m_config.set_wifi_password(password);

            // Сохраняем предыдущий режим
            wifi_mode_type_t prev_mode = WiFi.getMode();

            // Отключаемся если уже подключены
            if (WiFi.status() == WL_CONNECTED)
            {
                WiFi.disconnect(false);
                delay(100);
            }

            // Переключаем в режим STA или AP+STA
            if (prev_mode == WIFI_AP)
            {
                // Если были в AP - переходим в AP+STA для одновременной работы
                WiFi.mode(WIFI_AP_STA);
                m_mode = mode_t::ap_sta;
            }
            else
            {
                WiFi.mode(WIFI_STA);
                m_mode = mode_t::sta;
            }

            // Начинаем подключение
            WiFi.begin(ssid.c_str(), password.c_str());
            m_status = status_t::connecting;
            notify_status_change(m_status);

            // Ждём подключения
            uint32_t start_time = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout)
            {
                delay(100);
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                m_status = (m_mode == mode_t::ap_sta) ? status_t::ap_sta_mode : status_t::connected_sta;
                m_reconnect_attempts = 0;
                m_reconnect_pending = false;

                // Инициализация mDNS
                init_mdns(m_config.hostname);

                Serial.printf("[etl::wifi::manager] Connected, IP: %s\n", WiFi.localIP().toString().c_str());
                notify_status_change(m_status);
                return true;
            }

            // Не удалось подключиться
            Serial.println(F("[etl::wifi::manager] Connection timeout"));
            m_status = status_t::error;
            m_reconnect_pending = true;
            m_last_reconnect_time = millis();
            notify_status_change(m_status);
            return false;
        }

        void manager::disconnect()
        {
            Serial.println(F("[etl::wifi::manager] disconnect()"));

            stop_mdns();
            WiFi.disconnect(false);

            m_status = status_t::disconnected;
            m_reconnect_attempts = 0;
            m_reconnect_pending = false;
            notify_status_change(m_status);
        }

        // ============================================================================
        // start_ap()
        // ============================================================================

        bool manager::start_ap(const String& ssid /*= ""*/, const String& password /*= ""*/)
        {
            String ap_ssid = (ssid.length() > 0) ? ssid : m_config.ap_ssid;
            String ap_pass = (password.length() > 0) ? password : m_config.ap_password;

            Serial.printf("[etl::wifi::manager] start_ap() SSID: %s\n", ap_ssid.c_str());

            // Отключаемся если были подключены к STA
            if (WiFi.status() == WL_CONNECTED)
            {
                WiFi.disconnect(false);
                delay(100);
            }

            // Переключаем в режим AP
            WiFi.mode(WIFI_AP);
            WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());

            delay(100);

            IPAddress ip = WiFi.softAPIP();
            Serial.printf("[etl::wifi::manager] AP started, IP: %s\n", ip.toString().c_str());

            m_mode = mode_t::ap;
            m_status = status_t::ap_mode;
            notify_status_change(m_status);

            // Инициализация mDNS
            init_mdns(m_config.hostname);

            return true;
        }

        // ============================================================================
        // set_mode()
        // ============================================================================

        bool manager::set_mode(mode_t mode)
        {
            Serial.printf("[etl::wifi::manager] set_mode() %d\n", static_cast<uint8_t>(mode));

            wifi_mode_type_t wifi_mode;
            switch (mode)
            {
                case mode_t::none:
                    wifi_mode = WIFI_OFF;
                    break;
                case mode_t::ap:
                    wifi_mode = WIFI_AP;
                    break;
                case mode_t::sta:
                    wifi_mode = WIFI_STA;
                    break;
                case mode_t::ap_sta:
                    wifi_mode = WIFI_AP_STA;
                    break;
                default:
                    return false;
            }

            WiFi.mode(wifi_mode);
            m_mode = mode;

            // Обновляем статус
            update_status();
            notify_status_change(m_status);

            return true;
        }

        // ============================================================================
        // Геттеры
        // ============================================================================

        status_t manager::get_status() const
        {
            return m_status;
        }

        mode_t manager::get_mode() const
        {
            return m_mode;
        }

        String manager::get_ip_address() const
        {
            if (m_status == status_t::connected_sta || m_status == status_t::ap_sta_mode)
            {
                return WiFi.localIP().toString();
            }
            else if (m_status == status_t::ap_mode)
            {
                return WiFi.softAPIP().toString();
            }
            return String("0.0.0.0");
        }

        bool manager::is_connected() const
        {
            return (WiFi.status() == WL_CONNECTED);
        }

        String manager::get_hostname() const
        {
            return String(m_config.hostname);
        }

        const etl::webui::server_config_t& manager::get_config() const
        {
            return m_config;
        }

        // ============================================================================
        // scan_networks()
        // ============================================================================

        int32_t manager::scan_networks(etl::vector<scan_result_t>& results)
        {
            uint32_t now = millis();

            // Проверяем кэш
            if (now - m_scan_timestamp < SCAN_CACHE_TIME && !m_scan_cache.empty())
            {
                results = m_scan_cache;
                return results.size();
            }

            // Определяем текущий режим для восстановления после сканирования
            wifi_mode_type_t current_mode = WiFi.getMode();

            // Для сканирования нужен STA режим
            if (current_mode != WIFI_STA && current_mode != WIFI_AP_STA)
            {
                WiFi.mode(WIFI_STA);
                delay(100);
            }

            // Сканируем
            int32_t count = WiFi.scanNetworks();

            if (count > 0)
            {
                m_scan_cache.clear();
                for (int32_t i = 0; i < count; ++i)
                {
                    scan_result_t result;
                    result.ssid = WiFi.SSID(i);
                    result.rssi = WiFi.RSSI(i);
                    result.channel = WiFi.channel(i);
                    result.encryption = get_encryption_type(WiFi.encryptionType(i));
                    result.connected = (result.ssid == WiFi.SSID());

                    m_scan_cache.push_back(result);
                }

                // Сортировка по RSSI (сильный сигнал первый)
                // etl::vector не имеет std::sort, сортируем вручную
                for (size_t i = 0; i < m_scan_cache.size(); ++i)
                {
                    for (size_t j = i + 1; j < m_scan_cache.size(); ++j)
                    {
                        if (m_scan_cache[j].rssi > m_scan_cache[i].rssi)
                        {
                            scan_result_t temp = m_scan_cache[i];
                            m_scan_cache[i] = m_scan_cache[j];
                            m_scan_cache[j] = temp;
                        }
                    }
                }
            }

            // Восстанавливаем режим
            WiFi.mode(current_mode);

            m_scan_timestamp = now;
            results = m_scan_cache;
            return results.size();
        }

        // ============================================================================
        // Callback'и
        // ============================================================================

        void manager::add_status_callback(status_callback_t cb)
        {
            if (cb)
            {
                // Проверяем дубликаты
                for (const auto& existing_cb : m_status_callbacks)
                {
                    if (existing_cb == cb) return;
                }
                m_status_callbacks.push_back(cb);
                Serial.println(F("[etl::wifi::manager] Status callback added"));
            }
        }

        void manager::remove_status_callback(status_callback_t cb)
        {
            if (cb)
            {
                // etl::vector не имеет std::remove, удаляем вручную
                size_t write_idx = 0;
                for (size_t read_idx = 0; read_idx < m_status_callbacks.size(); ++read_idx)
                {
                    if (m_status_callbacks[read_idx] != cb)
                    {
                        if (write_idx != read_idx)
                        {
                            m_status_callbacks[write_idx] = m_status_callbacks[read_idx];
                        }
                        write_idx++;
                    }
                }
                m_status_callbacks.resize(write_idx);
                Serial.println(F("[etl::wifi::manager] Status callback removed"));
            }
        }

        // ============================================================================
        // update_config()
        // ============================================================================

        void manager::update_config(const etl::webui::server_config_t& config)
        {
            Serial.println(F("[etl::wifi::manager] update_config()"));
            m_config = config;
        }

        void manager::apply_settings_changes()
        {
            Serial.println(F("[etl::wifi::manager] apply_settings_changes()"));

            // Загружаем новые настройки
            auto new_config = etl::webui::settings::load_wifi_config();
            if (new_config.has_value())
            {
                // Сохраняем текущий SSID для сравнения
                String old_ssid = m_config.wifi_ssid;
                String new_ssid = new_config.value().wifi_ssid;

                update_config(new_config.value());

                // Если изменилась сеть - переподключаемся
                if (old_ssid != new_ssid && strlen(new_ssid.c_str()) > 0)
                {
                    Serial.printf("[etl::wifi::manager] WiFi SSID changed, reconnecting to: %s\n", new_ssid.c_str());
                    disconnect();
                    connect(new_ssid, new_config.value().wifi_password);
                }
                // Если изменились настройки AP - перезапускаем AP
                else if (m_status == status_t::ap_mode || m_status == status_t::ap_sta_mode)
                {
                    Serial.println(F("[etl::wifi::manager] Restarting AP with new settings"));
                    start_ap();
                }
            }
        }

        // ============================================================================
        // mDNS
        // ============================================================================

        bool manager::init_mdns(const String& hostname)
        {
            if (m_mdns_initialized)
            {
                MDNS.end();
                m_mdns_initialized = false;
            }

            Serial.printf("[etl::wifi::manager] init_mdns() hostname: %s\n", hostname.c_str());

            if (MDNS.begin(hostname.c_str()))
            {
                MDNS.addService("http", "tcp", m_config.port);
                m_mdns_initialized = true;
                Serial.println(F("[etl::wifi::manager] mDNS initialized"));
                return true;
            }

            Serial.println(F("[etl::wifi::manager] mDNS initialization failed"));
            return false;
        }

        void manager::stop_mdns()
        {
            if (m_mdns_initialized)
            {
                MDNS.end();
                m_mdns_initialized = false;
                Serial.println(F("[etl::wifi::manager] mDNS stopped"));
            }
        }

        // ============================================================================
        // Protected методы
        // ============================================================================

        bool manager::connect_to_sta(uint32_t timeout)
        {
            Serial.printf("[etl::wifi::manager] connect_to_sta() SSID: %s, timeout: %u\n",
                          m_config.wifi_ssid, timeout);

            WiFi.mode(WIFI_STA);
            WiFi.begin(m_config.wifi_ssid, m_config.wifi_password);

            uint32_t start_time = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < timeout)
            {
                delay(100);
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.printf("[etl::wifi::manager] Connected to STA, IP: %s\n",
                              WiFi.localIP().toString().c_str());
                return true;
            }

            Serial.println(F("[etl::wifi::manager] Failed to connect to STA"));
            return false;
        }

        void manager::update_status()
        {
            status_t new_status = m_status;

            if (WiFi.status() == WL_CONNECTED)
            {
                if (WiFi.getMode() == WIFI_AP_STA)
                {
                    new_status = status_t::ap_sta_mode;
                }
                else
                {
                    new_status = status_t::connected_sta;
                }
            }
            else if (WiFi.getMode() == WIFI_AP)
            {
                new_status = status_t::ap_mode;
            }
            else if (WiFi.getMode() == WIFI_OFF)
            {
                new_status = status_t::disconnected;
            }

            if (new_status != m_status)
            {
                m_status = new_status;
                notify_status_change(m_status);
            }
        }

        void manager::notify_status_change(status_t new_status)
        {
            for (const auto& cb : m_status_callbacks)
            {
                if (cb)
                {
                    cb(new_status);
                }
            }
        }

        String manager::get_encryption_type(uint8_t type) const
        {
#if defined(ESP8266)
            switch (type)
            {
                case ENC_TYPE_NONE: return "Open";
                case ENC_TYPE_WEP: return "WEP";
                case ENC_TYPE_TKIP: return "WPA";
                case ENC_TYPE_CCMP: return "WPA2";
                case ENC_TYPE_AUTO: return "WPA/WPA2";
                default: return "Unknown";
            }
#elif defined(ESP32)
            switch (type)
            {
                case WIFI_AUTH_OPEN: return "Open";
                case WIFI_AUTH_WEP: return "WEP";
                case WIFI_AUTH_WPA_PSK: return "WPA";
                case WIFI_AUTH_WPA2_PSK: return "WPA2";
                case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
                case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
                default: return "Unknown";
            }
#endif
        }

        void manager::attempt_reconnect()
        {
            if (m_reconnect_attempts >= MAX_RECONNECT_ATTEMPTS)
            {
                Serial.printf("[etl::wifi::manager] Max reconnect attempts (%u) reached, switching to AP\n",
                              MAX_RECONNECT_ATTEMPTS);

                // Переходим в AP режим
                start_ap();
                m_mode = mode_t::ap;
                m_status = status_t::ap_mode;
                m_reconnect_attempts = 0;
                m_reconnect_pending = false;
                notify_status_change(m_status);
                return;
            }

            m_reconnect_attempts++;
            m_last_reconnect_time = millis();

            Serial.printf("[etl::wifi::manager] Reconnect attempt %u/%u\n",
                          m_reconnect_attempts, MAX_RECONNECT_ATTEMPTS);

            m_status = status_t::connecting;
            WiFi.reconnect();
            notify_status_change(m_status);

            // Ждём немного результата
            uint32_t start_time = millis();
            while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < 5000)
            {
                delay(100);
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                m_status = (m_mode == mode_t::ap_sta) ? status_t::ap_sta_mode : status_t::connected_sta;
                m_reconnect_attempts = 0;
                m_reconnect_pending = false;
                notify_status_change(m_status);
                Serial.println(F("[etl::wifi::manager] Reconnected successfully"));
            }
            else
            {
                m_status = status_t::disconnected;
                m_reconnect_pending = true;
                notify_status_change(m_status);
            }
        }

    } // namespace wifi
} // namespace etl

#else
    #pragma message("etl_wifi: no implementation for this platform")
#endif
