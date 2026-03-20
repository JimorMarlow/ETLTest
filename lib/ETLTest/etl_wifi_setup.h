#pragma once
/**
 * @file etl_wifi_setup.h
 * @brief WiFi Setup Server для первичной настройки WiFi подключения
 *
 * Платформа: ESP8266 (NodeMCU v3)
 *
 * Особенности:
 * - Режим точки доступа для настройки WiFi
 * - Сканирование доступных сетей
 * - Подключение к выбранной сети
 * - Сохранение настроек в энергонезависимой памяти
 * - Сброс к заводским настройкам
 * - Встроенный HTTP сервер с веб-интерфейсом
 * - mDNS для доступа по имени hostname.local
 *
 * @note Класс предоставляет серверную часть для настройки WiFi с веб-интерфейсом.
 *       Веб-страница доступна по http://hostname.local (где hostname из конфигурации)
 */

#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <etl/etl_memory.h>

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Конфигурация WiFi сервера
         *
         * Содержит параметры для точки доступа и внешней сети.
         */
        struct server_config_t
        {
            // Конфигурация сети
            String hostname = "espdevice";              // Имя хоста для mDNS
            String ap_ssid = "ESP_Device_AP";           // SSID точки доступа
            String ap_password = "password123";         // Пароль точки доступа
            String wifi_ssid = "";                       // SSID внешней сети (пусто = не подключено)
            String wifi_password = "";                   // Пароль внешней сети
            uint16_t port = 80;                         // Порт веб-сервера
            uint32_t update_interval = 500;             // Интервал обновления данных (мс)

            // Информация об устройстве (не сохраняется в постоянной памяти, берется каждый раз из значения по умолчанию, чтобы была актуальной)
            struct device_info_t {
                String name = "ESP Device v1.0.0";      // Название устройства
                String description = "Smart home device based on ESP8266/ESP32";  // Описание
                String icon_svg = "";                   // SVG иконка устройства (опционально)

                // Очистка информации об устройстве
                void clear() {
                    name = "";
                    description = "";
                    icon_svg = "";
                }

                // Оператор присвоения
                device_info_t& operator=(const device_info_t& other) {
                    if (this != &other) {
                        name = other.name;
                        description = other.description;
                        icon_svg = other.icon_svg;
                    }
                    return *this;
                }
            } device;

            void trace() const {
                Serial.println(F("=== server_config_t settings ==="));
                Serial.printf("hostname        = %s\n", hostname.c_str());
                Serial.printf("ap_ssid         = %s\n", ap_ssid.c_str());
                Serial.printf("ap_password     = %s\n", ap_password.c_str());
                Serial.printf("wifi_ssid       = %s\n", wifi_ssid.c_str());
                Serial.printf("wifi_password   = %s\n", wifi_password.c_str());
                Serial.printf("port            = %u\n", port);
                Serial.printf("update_interval = %u\n", update_interval);
                Serial.printf("--- device info ---\n");
                Serial.printf("name            = %s\n", device.name.c_str());
                Serial.printf("description     = %s\n", device.description.c_str());
                Serial.printf("icon_svg        = %s\n", device.icon_svg.c_str());
                Serial.println(F("========================"));
            }
        };

        /**
         * @brief Результат сканирования WiFi сети
         */
        struct scan_result_t
        {
            String ssid;                                // SSID сети
            int32_t rssi;                               // Уровень сигнала (dBm)
            String encryption;                          // Тип шифрования (WPA2, WPA, Open)
            uint8_t channel;                            // Канал
        };

        /**
         * @brief Статус подключения WiFi
         */
        enum class connection_status_t
        {
            disconnected,                               // Не подключено
            connecting,                                 // Подключение
            connected,                                  // Подключено
            error                                       // Ошибка
        };

        /**
         * @brief Значение текущих настроек WiFi
         */
        namespace settings
        {
            /**
             * @brief Установить значения подключения к точками доступа по умолчанию и считать данные
             * @param cfg Конфигурация WiFi сервера по умолчанию
             */
            bool init_config(const etl::wifi::server_config_t& default_cfg);  

            /**
             * @brief Установить значения подключения к точками доступа
             * @param cfg Конфигурация WiFi сервера
             */
            bool save_config(const etl::wifi::server_config_t& cfg);  

            /**
             * @brief Считать текущие значения подключения к точками доступа
             */
            etl::wifi::server_config_t load_config();  
        }

        /**
         * @brief Класс WiFi Setup Server
         *
         * Предоставляет функционал для первичной настройки WiFi подключения.
         * Работает в режиме точки доступа или подключается к внешней сети.
         */
        class server_setup
        {
        public:
            /**
             * @brief Конструктор
             * @param cfg Конфигурация WiFi сервера
             */
            explicit server_setup(const server_config_t& cfg = server_config_t());

            /**
             * @brief Деструктор
             *
             * Виртуальный деструктор для корректного наследования.
             * Вызывает stop() для освобождения ресурсов.
             */
            virtual ~server_setup();

            /**
             * @brief Инициализация WiFi сервера
             *
             * - Запуск в режиме точки доступа
             * - Настройка сети
             *
             * @return true при успешной инициализации
             */
            virtual bool begin();

            /**
             * @brief Остановка WiFi сервера
             *
             * - Отключение от WiFi сети
             * - Остановка точки доступа
             * - Сброс флага инициализации
             */
            virtual void stop();

            /**
             * @brief Основной цикл обработки
             *
             * Вызывать регулярно из loop() для обработки событий WiFi
             */
            virtual void handle();

            /**
             * @brief Проверка инициализации
             * @return true если сервер инициализирован
             */
            virtual bool is_initialized() const;

            /**
             * @brief Получить статус подключения
             * @return Статус подключения
             */
            virtual connection_status_t get_connection_status() const;

            /**
             * @brief Проверка подключения к WiFi сети
             * @return true если подключено к внешней сети
             */
            virtual bool is_connected() const;

            /**
             * @brief Получить IP адрес
             * @return IP адрес в формате String
             */
            virtual String get_ip_address() const;

            /**
             * @brief Получить режим работы
             * @return "AP" если точка доступа, "STA" если клиент, "AP+STA" если оба режима
             */
            virtual String get_mode() const;

            /**
             * @brief Сканирование доступных WiFi сетей
             * @param results Вектор для результатов сканирования
             * @return Количество найденных сетей
             */
            virtual int32_t scan_networks(std::vector<scan_result_t>& results);

            /**
             * @brief Подключение к WiFi сети
             * @param ssid SSID сети
             * @param password Пароль сети
             * @param timeout Таймаут подключения (мс, по умолчанию 10000)
             * @return true при успешном подключении
             */
            virtual bool connect_to_network(const String& ssid, const String& password, uint32_t timeout = 10000);

            /**
             * @brief Отключение от WiFi сети
             */
            virtual void disconnect();

            /**
             * @brief Сохранение настроек WiFi
             * @return true при успешном сохранении
             */
            virtual bool save_settings();

            /**
             * @brief Загрузка сохранённых настроек WiFi
             * @return true если настройки загружены успешно
             */
            virtual bool load_settings();

            /**
             * @brief Сброс настроек WiFi к заводским
             * @return true при успешном сбросе
             */
            virtual bool reset_settings();

            /**
             * @brief Установить конфигурацию сервера
             * @param cfg Новая конфигурация
             * @note Должно быть вызвано до begin() или после stop()
             */
            virtual void set_config(const server_config_t& cfg);

            /**
             * @brief Получить текущую конфигурацию
             * @return Конфигурация сервера
             */
            virtual const server_config_t& get_config() const { return m_config; }

            /**
             * @brief Перезагрузка устройства
             * @note Вызывает ESP.reset()
             */
            virtual void reboot();

            /**
             * @brief Обработка HTTP запросов сервера
             * @note Вызывать из loop() вместе с handle()
             */
            virtual void handle_client();

        protected:
            etl::shared_ptr<ESP8266WebServer> m_server; ///< HTTP сервер
            std::vector<scan_result_t> m_scan_cache;    ///< Кэш результатов сканирования
            uint32_t m_scan_timestamp = 0;              ///< Время последнего сканирования
            static const uint32_t SCAN_CACHE_TIME = 30000;  ///< Время кэширования сканирования (30 сек)

            /**
             * @brief Запуск HTTP сервера
             */
            virtual void start_http_server();

            /**
             * @brief Запуск точки доступа
             * @return true при успешном запуске
             */
            virtual bool start_ap();

            /**
             * @brief Подключение к внешней сети
             * @param timeout Таймаут подключения (мс)
             * @return true при успешном подключении
             */
            virtual bool connect_to_sta(uint32_t timeout);

            /**
             * @brief Обновление статуса подключения
             */
            virtual void update_connection_status();

            /**
             * @brief Получить тип шифрования из WiFi.encryptionType()
             * @param type Тип шифрования
             * @return Строковое представление типа шифрования
             */
            virtual String get_encryption_type(uint8_t type) const;

            /**
             * @brief Настройка HTTP роутинга
             */
            virtual void setup_http_routes();

            /**
             * @brief Обработчик главной страницы
             */
            virtual void handle_root();

            /**
             * @brief Обработчик API сканирования сетей
             */
            virtual void handle_api_scan();

            /**
             * @brief Обработчик API подключения
             */
            virtual void handle_api_connect();

            /**
             * @brief Обработчик API статуса
             */
            virtual void handle_api_status();

            /**
             * @brief Обработчик API конфигурации устройства
             */
            virtual void handle_api_config();

            /**
             * @brief Обработчик API сохранения настроек
             */
            virtual void handle_api_save();

            /**
             * @brief Обработчик API сброса настроек
             */
            virtual void handle_api_reset();

            /**
             * @brief Обработчик API настройки точки доступа
             */
            virtual void handle_api_ap_settings();

            /**
             * @brief Получить SVG иконку устройства
             * @return SVG строка или иконка по умолчанию
             */
            virtual String get_device_icon() const;

            /**
             * @brief Отправить ответ с результатами сканирования
             */
            virtual void send_scan_response();

            /**
             * @brief Отправить успешный ответ
             * @param message Сообщение
             * @param extra_data Дополнительные данные
             */
            virtual void send_success_response(const String& message, const String& extra_data = "");

            /**
             * @brief Отправить ответ с ошибкой
             * @param message Сообщение об ошибке
             */
            virtual void send_error_response(const String& message);

            server_config_t m_config;                   ///< Конфигурация
            bool m_initialized = false;                 ///< Флаг инициализации
            connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения
        };

    } // namespace wifi
} // namespace etl

#else
    #pragma message("etl_wifi_setup: no implementation for this platform")
    // Пустой файл по умолчанию
#endif

/**
 * @brief Пример использования в main.cpp:
 *
 * @code
 * #include "etl_wifi_setup.h"
 *
 * // Глобальный экземпляр
 * etl::wifi::server_setup wifi_server;
 * etl::wifi::server_config_t wifi_config;
 *
 * void setup() {
 *     Serial.begin(115200);
 *
 *     // Настройка конфигурации
 *     wifi_config.hostname = "moonshine";
 *     wifi_config.ap_ssid = "Moonshine_AP";
 *     wifi_config.ap_password = "moonshine123";
 *
 *     // Настройка веб-интерфейса
 *     wifi_config.device_name = "Moonshine v1.2.13";
 *     wifi_config.device_description = "Устройство для контроля температуры";
 *     // Опционально: кастомная SVG иконка
 *     // wifi_config.device_icon_svg = "<svg>...</svg>";
 *
 *     // Попытка загрузки сохранённых настроек
 *     if (wifi_server.load_settings()) {
 *         // Настройки загружены, пробуем подключиться
 *         wifi_server.set_config(wifi_config);
 *     } else {
 *         // Настроек нет, используем конфигурацию по умолчанию
 *         wifi_server.set_config(wifi_config);
 *     }
 *
 *     // Инициализация WiFi сервера
 *     if (wifi_server.begin()) {
 *         Serial.println("WiFi setup server started");
 *         Serial.print("IP: ");
 *         Serial.println(wifi_server.get_ip_address());
 *     }
 * }
 *
 * void loop() {
 *     // Обработка событий WiFi
 *     wifi_server.handle();
 *
 *     // Обработка HTTP запросов
 *     wifi_server.handle_client();
 *
 *     // Проверка статуса подключения
 *     if (wifi_server.is_connected()) {
 *         Serial.println("Connected to WiFi");
 *         Serial.print("IP: ");
 *         Serial.println(wifi_server.get_ip_address());
 *     }
 *
 *     // ... остальная логика
 * }
 *
 * // Пример сканирования сетей
 * void scan_wifi_networks() {
 *     std::vector<etl::wifi::scan_result_t> networks;
 *     int32_t count = wifi_server.scan_networks(networks);
 *
 *     Serial.printf("Found %d networks\n", count);
 *     for (const auto& network : networks) {
 *         Serial.printf("SSID: %s, RSSI: %d, Encryption: %s\n",
 *                       network.ssid.c_str(), network.rssi, network.encryption.c_str());
 *     }
 * }
 *
 * // Пример подключения к сети
 * void connect_to_wifi() {
 *     if (wifi_server.connect_to_network("MyWiFi", "mypassword")) {
 *         Serial.println("Connected successfully");
 *         wifi_server.save_settings();  // Сохранить настройки
 *     } else {
 *         Serial.println("Connection failed");
 *     }
 * }
 *
 * // Пример сброса настроек
 * void factory_reset() {
 *     wifi_server.reset_settings();
 *     wifi_server.reboot();  // Перезагрузка для применения сброса
 * }
 * @endcode
 */
