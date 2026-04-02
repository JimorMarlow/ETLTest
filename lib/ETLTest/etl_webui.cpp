/**
 * @file etl_wifi_setup.cpp
 * @brief Реализация WiFi Setup Server
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_webui.h"
#include "etl_wifi_setup_html.h"
// #include "etl_webui_base.cpp"

#include "etl/etl_littlefs.h"
#include "etl/etl_settings.h"

namespace etl
{
    namespace webui
    {
        // Константы для хранения настроек
        static const uint32_t WIFI_CONNECT_TIMEOUT = 10000;  // 10 секунд

        namespace settings
        {
            // WiFi настройки
            const String    wifi_data_path = "/settings/wifi.cfg";
            const uint16_t  wifi_data_update_delay = 0;  // 0ms - Immediately update
            server_config_t default_wifi_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::server_config_t>> wifi_cfg;

            // UI настройки
            const String    ui_data_path = "/settings/ui.cfg";
            const uint16_t  ui_data_update_delay = 0;  // 0ms - Immediately update
            ui_config_t     default_ui_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::ui_config_t>> ui_cfg;

            // Telegram настройки
            const String    telegram_data_path = "/settings/telegram.cfg";
            const uint16_t  telegram_data_update_delay = 0;  // 0ms - Immediately update
            telegram_config_t default_telegram_cfg;          // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::telegram_config_t>> telegram_cfg;

            // MQTT настройки
            const String    mqtt_data_path = "/settings/mqtt.cfg";
            const uint16_t  mqtt_data_update_delay = 0;  // 0ms - Immediately update
            mqtt_config_t   default_mqtt_cfg;            // Значение по-умолчанию для сброса к заводским значениям
            etl::shared_ptr<etl::settings::data<etl::webui::mqtt_config_t>> mqtt_cfg;

            // ============================================================================
            // WiFi настройки
            // ============================================================================

            /**
             * @brief Установить значения подключения к точками доступа по умолчанию и считать данные
             * @param cfg Конфигурация WiFi сервера по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_wifi_config(const etl::webui::server_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_wifi_config()"));

                if(etl::little_fs::begin())
                {
                    // Создание директории для файла настроек
                    etl::little_fs::create_dir(settings::wifi_data_path);
                }

                // Сохранение настроек в постоянной памяти
                if(!wifi_cfg)
                {
                    wifi_cfg = etl::make_shared<etl::settings::data<etl::webui::server_config_t>>(settings::wifi_data_path, settings::wifi_data_update_delay, default_cfg);
                    bool result = wifi_cfg->init();
                    Serial.print(F("[wifi::settings] init_wifi_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting to default ..."));
                        auto loaded_cfg = wifi_cfg->get();
                        Serial.println(F("[wifi::settings] loaded from memory:"));
                        loaded_cfg.trace();

                        // Выполняем сброс: устанавливаем значения по умолчанию и сохраняем
                        wifi_cfg->set(default_cfg);
                        bool reset_result = wifi_cfg->save();

                        Serial.print(F("[wifi::settings] reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                        if(reset_result)
                        {
                            default_cfg.trace();
                        }
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_wifi_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Установить значения подключения к точками доступа
             * @param cfg Конфигурация WiFi сервера
             */
            bool save_wifi_config(const server_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_wifi_config()"));

                if(wifi_cfg)
                {
                    wifi_cfg->set(cfg);
                    bool result = wifi_cfg->save();
                    Serial.print(F("[wifi::settings] save_wifi_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_wifi_config() error: wifi_cfg not inited"));
                return false;
            }

            /**
             * @brief Считать текущие значения подключения к точками доступа
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<server_config_t> load_wifi_config()
            {
                Serial.println(F("[wifi::settings] load_wifi_config()"));

                if(wifi_cfg)
                {
                    server_config_t cfg = wifi_cfg->get();
                    Serial.println(F("[wifi::settings] load_wifi_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_wifi_config(): wifi_cfg not inited, returning empty optional"));
                    return {};
                }
            }

            // ============================================================================
            // UI настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек интерфейса
             * @param default_cfg Конфигурация интерфейса по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_ui_config(const etl::webui::ui_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_ui_config()"));

                if(etl::little_fs::begin())
                {
                    // Создание директории для файла настроек
                    etl::little_fs::create_dir(settings::ui_data_path);
                }

                // Сохранение настроек в постоянной памяти
                if(!ui_cfg)
                {
                    ui_cfg = etl::make_shared<etl::settings::data<etl::webui::ui_config_t>>(settings::ui_data_path, settings::ui_data_update_delay, default_cfg);
                    bool result = ui_cfg->init();
                    Serial.print(F("[wifi::settings] init_ui_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting UI to default ..."));
                        auto loaded_cfg = ui_cfg->get();
                        Serial.println(F("[wifi::settings] UI loaded from memory:"));
                        loaded_cfg.trace();

                        // Выполняем сброс: устанавливаем значения по умолчанию и сохраняем
                        ui_cfg->set(default_cfg);
                        bool reset_result = ui_cfg->save();

                        Serial.print(F("[wifi::settings] UI reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                        if(reset_result)
                        {
                            default_cfg.trace();
                        }
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_ui_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки интерфейса
             * @param cfg Конфигурация интерфейса
             */
            bool save_ui_config(const ui_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_ui_config()"));

                if(ui_cfg)
                {
                    ui_cfg->set(cfg);
                    bool result = ui_cfg->save();
                    Serial.print(F("[wifi::settings] save_ui_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_ui_config() error: ui_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки интерфейса
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<ui_config_t> load_ui_config()
            {
                Serial.println(F("[wifi::settings] load_ui_config()"));

                if(ui_cfg)
                {
                    ui_config_t cfg = ui_cfg->get();
                    Serial.println(F("[wifi::settings] load_ui_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_ui_config(): ui_cfg not inited, returning empty optional"));
                    etl::optional<ui_config_t> empty;
                    return empty;
                }
            }

            // ============================================================================
            // Telegram настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек Telegram бота
             * @param default_cfg Конфигурация Telegram бота по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_telegram_config(const etl::webui::telegram_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_telegram_config()"));

                if(etl::little_fs::begin())
                {
                    etl::little_fs::create_dir(settings::telegram_data_path);
                }

                if(!telegram_cfg)
                {
                    telegram_cfg = etl::make_shared<etl::settings::data<etl::webui::telegram_config_t>>(settings::telegram_data_path, settings::telegram_data_update_delay, default_cfg);
                    bool result = telegram_cfg->init();
                    Serial.print(F("[wifi::settings] init_telegram_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting Telegram to default ..."));
                        telegram_cfg->set(default_cfg);
                        bool reset_result = telegram_cfg->save();
                        Serial.print(F("[wifi::settings] Telegram reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_telegram_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки Telegram бота
             * @param cfg Конфигурация Telegram бота
             */
            bool save_telegram_config(const telegram_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_telegram_config()"));

                if(telegram_cfg)
                {
                    telegram_cfg->set(cfg);
                    bool result = telegram_cfg->save();
                    Serial.print(F("[wifi::settings] save_telegram_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_telegram_config() error: telegram_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки Telegram бота
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<telegram_config_t> load_telegram_config()
            {
                Serial.println(F("[wifi::settings] load_telegram_config()"));

                if(telegram_cfg)
                {
                    telegram_config_t cfg = telegram_cfg->get();
                    Serial.println(F("[wifi::settings] load_telegram_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_telegram_config(): telegram_cfg not inited, returning empty optional"));
                    return {};
                }
            }

            // ============================================================================
            // MQTT настройки
            // ============================================================================

            /**
             * @brief Инициализация настроек MQTT
             * @param default_cfg Конфигурация MQTT по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_mqtt_config(const etl::webui::mqtt_config_t& default_cfg, bool reset_to_default /*= false*/)
            {
                Serial.println(F("[wifi::settings] init_mqtt_config()"));

                if(etl::little_fs::begin())
                {
                    etl::little_fs::create_dir(settings::mqtt_data_path);
                }

                if(!mqtt_cfg)
                {
                    mqtt_cfg = etl::make_shared<etl::settings::data<etl::webui::mqtt_config_t>>(settings::mqtt_data_path, settings::mqtt_data_update_delay, default_cfg);
                    bool result = mqtt_cfg->init();
                    Serial.print(F("[wifi::settings] init_mqtt_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));

                    if(result && reset_to_default)
                    {
                        Serial.println(F("[wifi::settings] resetting MQTT to default ..."));
                        mqtt_cfg->set(default_cfg);
                        bool reset_result = mqtt_cfg->save();
                        Serial.print(F("[wifi::settings] MQTT reset to default: "));
                        Serial.println(reset_result ? F("OK") : F("FAILED"));
                    }

                    return result;
                }

                Serial.print(F("[wifi::settings] init_mqtt_config() result: ALREADY INITED"));
                return true;
            }

            /**
             * @brief Сохранить настройки MQTT
             * @param cfg Конфигурация MQTT
             */
            bool save_mqtt_config(const mqtt_config_t& cfg)
            {
                Serial.println(F("[wifi::settings] save_mqtt_config()"));

                if(mqtt_cfg)
                {
                    mqtt_cfg->set(cfg);
                    bool result = mqtt_cfg->save();
                    Serial.print(F("[wifi::settings] save_mqtt_config() result: "));
                    Serial.println(result ? F("OK") : F("FAILED"));
                    return result;
                }

                Serial.print(F("[wifi::settings] save_mqtt_config() error: mqtt_cfg not inited"));
                return false;
            }

            /**
             * @brief Загрузить настройки MQTT
             * @return etl::optional с конфигом, если он был инициализирован, или пустой optional
             */
            etl::optional<mqtt_config_t> load_mqtt_config()
            {
                Serial.println(F("[wifi::settings] load_mqtt_config()"));

                if(mqtt_cfg)
                {
                    mqtt_config_t cfg = mqtt_cfg->get();
                    Serial.println(F("[wifi::settings] load_mqtt_config() loaded from FS"));
                    cfg.trace();
                    return cfg;
                }
                else
                {
                    Serial.println(F("[wifi::settings] load_mqtt_config(): mqtt_cfg not inited, returning empty optional"));
                    return {};
                }
            }
        }

        // ============================================================================
        // Реализация server_setup
        // ============================================================================

        server_setup::server_setup(const etl::optional<server_config_t>& cfg)
            : web_server_base_t(cfg)
        {
        }

        server_setup::~server_setup()
        {
            stop();
        }

        String server_setup::get_hostname() const
        {
            return m_config.has_value() ? String(m_config->hostname) : "espdevice";
        }

        uint16_t server_setup::get_port() const
        {
            return m_config.has_value() ? m_config->port : 80;
        }

        bool server_setup::begin(const device_info_t& device_info)
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

        void server_setup::handle_client()
        {
            if (m_server != nullptr) {
#ifdef ESP8266
                MDNS.update();
#endif
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
            // Проверяем активные интерфейсы напрямую
            WiFiMode_t mode = WiFi.getMode();
            bool ap_active = (mode == WIFI_AP || mode == WIFI_AP_STA);
            bool sta_connected = (WiFi.status() == WL_CONNECTED);

            if (ap_active && sta_connected) return "AP+STA";
            if (sta_connected) return "STA";
            if (ap_active) return "AP";
            return "OFF";
        }

        int32_t server_setup::scan_networks(std::vector<scan_result_t>& results)
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

        bool server_setup::connect_to_network(const String& ssid, const String& password, uint32_t timeout)
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

        void server_setup::connect_to_network_async(const String& ssid, const String& password)
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

        void server_setup::disconnect()
        {
            Serial.println(F("[WiFiSetup] Disconnecting..."));

            WiFi.disconnect(true);
            m_connection_status = connection_status_t::disconnected;
        }

        bool server_setup::save_settings()
        {
            Serial.println(F("[WiFiSetup] Saving settings..."));
            
            bool wifi_saved = false;
            bool ui_saved = false;
            
            // Сохранение WiFi настроек
            if (m_config.has_value()) {
                m_config->trace();
                wifi_saved = settings::save_wifi_config(*m_config);
                Serial.print(F("[WiFiSetup] WiFi settings saved: "));
                Serial.println(wifi_saved ? F("OK") : F("FAILED"));
            } else {
                Serial.println(F("[WiFiSetup] No WiFi config to save"));
            }
            
            // Сохранение UI настроек
            if (m_ui_config.has_value()) {
                m_ui_config->trace();
                ui_saved = settings::save_ui_config(*m_ui_config);
                Serial.print(F("[WiFiSetup] UI settings saved: "));
                Serial.println(ui_saved ? F("OK") : F("FAILED"));
            } else {
                Serial.println(F("[WiFiSetup] No UI config to save"));
            }
            
            return wifi_saved || ui_saved;
        }

        bool server_setup::load_settings()
        {
            Serial.println(F("[WiFiSetup] Loading settings..."));
            
            // Загрузка WiFi настроек
            if (auto wifi_cfg = settings::load_wifi_config(); wifi_cfg.has_value()) 
            {
                m_config = wifi_cfg;
                Serial.println(F("[WiFiSetup] WiFi settings loaded"));
            } else {
                Serial.println(F("[WiFiSetup] No WiFi settings found"));
            }
            
            // Загрузка UI настроек
            if (auto ui_cfg = settings::load_ui_config(); ui_cfg.has_value()) 
            {
                m_ui_config = ui_cfg;
                Serial.println(F("[WiFiSetup] UI settings loaded"));
            } else {
                Serial.println(F("[WiFiSetup] No UI settings found"));
            }
            
            return true;
        }

        bool server_setup::reset_settings()
        {
            Serial.println(F("[WiFiSetup] Resetting settings..."));
            
            // Сброс WiFi конфигурации к значениям по умолчанию
            if (m_config.has_value()) {
                m_config->clear();
                settings::save_wifi_config(*m_config);
                Serial.println(F("[WiFiSetup] WiFi settings reset"));
            }
            
            // Сброс UI конфигурации к значениям по умолчанию
            if (m_ui_config.has_value()) {
                m_ui_config->clear();
                settings::save_ui_config(*m_ui_config);
                Serial.println(F("[WiFiSetup] UI settings reset"));
            }
            
            return true;
        }

        void server_setup::set_config(const etl::optional<server_config_t>& cfg)
        {
            m_config = cfg;
        }

        etl::optional<ui_config_t> server_setup::get_ui_config() const
        {
            return m_ui_config;
        }

        void server_setup::set_device_info(const device_info_t& info)
        {
            m_device_info = info;
        }

        void server_setup::reboot()
        {
            Serial.println(F("[WiFiSetup] Rebooting..."));
            delay(100);
#ifdef ESP8266
            ESP.reset();
#elif defined(ESP32)
            ESP.restart();
#endif
        }

        bool server_setup::start_ap()
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

        bool server_setup::connect_to_sta(uint32_t timeout)
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

        String server_setup::get_device_icon() const
        {
            if (m_device_info.icon_svg.length() > 0) {
                return m_device_info.icon_svg;
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
            int32_t count = scan_networks(m_scan_cache);
            m_scan_timestamp = millis();

            Serial.printf("[WiFiSetup] Scan completed: %d networks\n", count);

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
                // Помечаем текущую подключенную сеть
                net["connected"] = (network.ssid == (m_config.has_value() ? m_config->get_wifi_ssid() : "")) && is_connected();
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

// Подключение реализации настроек
#include "etl_webui_settings.cpp"

#else
    #pragma message("etl_wifi_setup: no implementation for this platform")
    // Пустой файл по умолчанию
#endif
