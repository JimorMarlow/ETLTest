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

// Размеры буферов для строк в server_config_t
#define WIFI_CONFIG_HOSTNAME_SIZE     32
#define WIFI_CONFIG_SSID_SIZE         32
#define WIFI_CONFIG_PASSWORD_SIZE     64

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Информация об устройстве
         * 
         * НЕ сохраняется в постоянной памяти, передаётся отдельно при запуске сервера.
         * Использует String для поддержки произвольных размеров (особенно для SVG иконки).
         */
        struct device_info_t
        {
            String name = "ESP Device v1.0.0";          // Название устройства
            String description = "Smart home device based on ESP8266/ESP32";  // Описание
            String icon_svg = "";                       // SVG иконка устройства (опционально)

            /**
             * @brief Очистка информации об устройстве
             */
            void clear() {
                name.clear();
                description.clear();
                icon_svg.clear();
            }

            /**
             * @brief Оператор присвоения
             * @param other Другой объект device_info_t
             * @return Ссылка на текущий объект
             */
            device_info_t& operator=(const device_info_t& other) {
                if (this != &other) {
                    name = other.name;
                    description = other.description;
                    icon_svg = other.icon_svg;
                }
                return *this;
            }

            /**
             * @brief Вывод информации об устройстве в Serial
             */
            void trace() const {
                Serial.println(F("--- device info ---"));
                Serial.printf("name            = %s\n", name.c_str());
                Serial.printf("description     = %s\n", description.c_str());
                Serial.printf("icon_svg        = %s\n", icon_svg.c_str());
            }
        };

        /**
         * @brief Конфигурация WiFi сервера
         *
         * Содержит параметры для точки доступа и внешней сети.
         * Сохраняется в энергонезависимой памяти через FileData.
         * Использует фиксированные массивы char для корректного бинарного сохранения.
         */
        struct server_config_t
        {
            // Конфигурация сети
            char hostname[WIFI_CONFIG_HOSTNAME_SIZE] = "espdevice";
            char ap_ssid[WIFI_CONFIG_SSID_SIZE] = "ESP_Device_AP";
            char ap_password[WIFI_CONFIG_PASSWORD_SIZE] = "password123";
            char wifi_ssid[WIFI_CONFIG_SSID_SIZE] = "";
            char wifi_password[WIFI_CONFIG_PASSWORD_SIZE] = "";
            uint16_t port = 80;                         // Порт веб-сервера
            uint32_t update_interval = 500;             // Интервал обновления данных (мс)

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear();

            /**
             * @brief Вывод конфигурации в Serial
             */
            void trace() const;

            // Setters
            void set_hostname(const String& value);
            void set_ap_ssid(const String& value);
            void set_ap_password(const String& value);
            void set_wifi_ssid(const String& value);
            void set_wifi_password(const String& value);

            // Getters
            String get_hostname() const;
            String get_ap_ssid() const;
            String get_ap_password() const;
            String get_wifi_ssid() const;
            String get_wifi_password() const;
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
            bool connected = false;                     // Флаг: подключено к этой сети
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
             * @param default_cfg Конфигурация WiFi сервера по умолчанию
             * @param reset_to_default Установить значения по умолчанию и перезаписать данные при старте
             */
            bool init_config(const etl::wifi::server_config_t& default_cfg, bool reset_to_default = false);

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
             * @param device_info Информация об устройстве (не сохраняется в FS)
             * @return true при успешной инициализации
             */
            virtual bool begin(const device_info_t& device_info);

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
             * @brief Начать подключение к WiFi сети (асинхронно, без ожидания)
             * @param ssid SSID сети
             * @param password Пароль сети
             */
            virtual void connect_to_network_async(const String& ssid, const String& password);

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
             * @brief Установить информацию об устройстве
             * @param info Информация об устройстве
             */
            virtual void set_device_info(const device_info_t& info);

            /**
             * @brief Получить информацию об устройстве
             * @return Информация об устройстве
             */
            virtual const device_info_t& get_device_info() const { return m_device_info; }

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
             * @brief Обработчик API отключения
             */
            virtual void handle_api_disconnect();

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

            server_config_t m_config;                   ///< Конфигурация WiFi
            device_info_t m_device_info;                ///< Информация об устройстве
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
 * etl::wifi::device_info_t device_info;
 *
 * void setup() {
 *     Serial.begin(115200);
 *
 *     // Настройка конфигурации WiFi
 *     wifi_config.set_hostname("moonshine");
 *     wifi_config.set_ap_ssid("Moonshine_AP");
 *     wifi_config.set_ap_password("moonshine123");
 *
 *     // Настройка информации об устройстве
 *     device_info.name = "Moonshine v1.2.13";
 *     device_info.description = "Устройство для контроля температуры";
 *     // Опционально: кастомная SVG иконка
 *     // device_info.icon_svg = "<svg>...</svg>";
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
 *     if (wifi_server.begin(device_info)) {
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
