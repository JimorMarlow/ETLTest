#pragma once
/**
 * @file etl_webui_base.h
 * @brief Базовый класс для веб-серверов WebUI
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 *
 * Особенности:
 * - Базовый класс для всех веб-серверов в системе
 * - Предоставляет общий интерфейс для управления серверами
 * - Поддержка умных указателей etl::shared_ptr для полиморфизма
 */

// Для включения нужной wi-fi библиотеки
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#else
  #pragma message("ERROR: no Wi-Fi lib specified")
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include "etl_webui_settings.h"

// Алиас типа сервера для совместимости ESP8266 и ESP32
#if defined(ESP8266)
  using etl_web_server_t = ESP8266WebServer;
#elif defined(ESP32)
  using etl_web_server_t = WebServer;
#endif

#if defined(ESP8266) || defined(ESP32)

namespace etl
{
    namespace webui
    {
        // device_info_t и connection_status_t определены в etl_webui_settings.h

        /**
         * @brief Базовый класс для веб-серверов
         *
         * Предоставляет общий интерфейс для всех веб-серверов в системе.
         * Используется для полиморфного управления через умные указатели.
         *
         * Пример использования:
         * @code
         * etl::shared_ptr<etl::webui::web_server_base_t> server;
         * server = etl::make_shared<etl::webui::server_setup>(config);
         * server->begin(device_info);
         * @endcode
         */
        class web_server_base_t
        {
        public:
            /**
             * @brief Виртуальный деструктор
             *
             * Обеспечивает корректное уничтожение объектов-наследников
             * при удалении через указатель на базовый класс.
             */
            virtual ~web_server_base_t();

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
             * @brief Запуск HTTP сервера
             */
            virtual void start_http_server() = 0;

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
             * @brief Получить имя хоста для mDNS
             * @return Имя хоста
             */
            virtual String get_hostname() const;

            /**
             * @brief Получить порт веб-сервера
             * @return Порт
             */
            virtual uint16_t get_port() const;

            /**
             * @brief Основной цикл обработки (объединяет handle() и handle_client())
             * 
             * Вызывать регулярно из loop() для обработки событий WiFi и HTTP запросов.
             */
            virtual void tick();

            /**
             * @brief Основной цикл обработки
             *
             * Вызывать регулярно из loop() для обработки событий WiFi и HTTP запросов.
             */
            virtual void handle();

            /**
             * @brief Обработка HTTP запросов сервера
             */
            virtual void handle_client();

            /**
             * @brief Сохранение настроек
             * @return true при успешном сохранении
             */
            virtual bool save_settings();

            /**
             * @brief Загрузка сохранённых настроек
             * @return true если настройки загружены успешно
             */
            virtual bool load_settings();

            /**
             * @brief Сброс настроек к заводским
             * @return true при успешном сбросе
             */
            virtual bool reset_settings();

            /**
             * @brief Перезагрузка устройства
             */
            virtual void reboot();

            /**
             * @brief Установить информацию об устройстве
             * @param info Информация об устройстве
             */
            virtual void set_device_info(const device_info_t& info);

            /**
             * @brief Получить информацию об устройстве
             * @return Информация об устройстве
             */
            virtual const device_info_t& get_device_info() const;

            /**
             * @brief Получить текущую конфигурацию интерфейса
             * @return Конфигурация интерфейса (опционально)
             */
            virtual etl::optional<ui_config_t> get_ui_config() const;

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
             * @brief Установить конфигурацию сервера
             * @param cfg Новая конфигурация
             */
            virtual void set_config(const etl::optional<server_config_t>& cfg);

            /**
             * @brief Получить текущую конфигурацию WiFi
             * @return Конфигурация сервера (опционально)
             */
            virtual const etl::optional<server_config_t>& get_wifi_config() const { return m_config; }

            /**
             * @brief Получить SVG иконку устройства
             * @return SVG строка или иконка по умолчанию
             */
            virtual String get_device_icon() const;

        protected:
            /**
             * @brief Защищённый конструктор для использования в наследниках
             * @param cfg Конфигурация WiFi сервера (опционально)
             */
            explicit web_server_base_t(const etl::optional<server_config_t>& cfg = {});

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
             * @brief Инициализация mDNS
             * @param hostname Имя хоста для mDNS
             * @return true при успешной инициализации
             */
            bool init_mdns(const String& hostname);

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

        protected:  
            // Data section
            etl::optional<server_config_t> m_config;                ///< Конфигурация WiFi (опционально)
            etl::optional<ui_config_t> m_ui_config;                 ///< Конфигурация интерфейса (опционально)
            device_info_t m_device_info;                            ///< Информация об устройстве
            bool m_initialized = false;                             ///< Флаг инициализации
            connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения
            etl::shared_ptr<etl_web_server_t> m_server;             ///< HTTP сервер
            std::vector<scan_result_t> m_scan_cache;                ///< Кэш результатов сканирования
            uint32_t m_scan_timestamp = 0;                          ///< Время последнего сканирования

            // Константы для хранения настроек
            static const uint32_t WIFI_CONNECT_TIMEOUT = 10000;     ///< Время ожидания подключени к wifi 10 секунд
            static const uint32_t SCAN_CACHE_TIME = 30000;          ///< Время кэширования сканирования (30 сек)

        };

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
