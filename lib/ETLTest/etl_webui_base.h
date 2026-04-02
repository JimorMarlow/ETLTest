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
            virtual ~web_server_base_t() = default;

            /**
             * @brief Инициализация веб-сервера
             *
             * @param device_info Информация об устройстве
             * @return true при успешной инициализации
             */
            virtual bool begin(const device_info_t& device_info) = 0;

            /**
             * @brief Остановка веб-сервера
             *
             * Освобождает ресурсы, останавливает сервер.
             */
            virtual void stop() = 0;

            /**
             * @brief Основной цикл обработки
             *
             * Вызывать регулярно из loop() для обработки событий WiFi и HTTP запросов.
             */
            virtual void handle() = 0;

            /**
             * @brief Проверка инициализации
             * @return true если сервер инициализирован
             */
            virtual bool is_initialized() const = 0;

            /**
             * @brief Получить статус подключения
             * @return Статус подключения
             */
            virtual connection_status_t get_connection_status() const = 0;

            /**
             * @brief Проверка подключения к WiFi сети
             * @return true если подключено к внешней сети
             */
            virtual bool is_connected() const = 0;

            /**
             * @brief Получить IP адрес
             * @return IP адрес в формате String
             */
            virtual String get_ip_address() const = 0;

            /**
             * @brief Получить режим работы
             * @return "AP" если точка доступа, "STA" если клиент, "AP+STA" если оба режима
             */
            virtual String get_mode() const = 0;

            /**
             * @brief Получить имя хоста для mDNS
             * @return Имя хоста
             */
            virtual String get_hostname() const = 0;

            /**
             * @brief Получить порт веб-сервера
             * @return Порт
             */
            virtual uint16_t get_port() const = 0;

        protected:
            /**
             * @brief Защищённый конструктор для использования в наследниках
             * @param cfg Конфигурация WiFi сервера (опционально)
             */
            explicit web_server_base_t(const etl::optional<server_config_t>& cfg = {});

            /**
             * @brief Инициализация mDNS
             * @param hostname Имя хоста для mDNS
             * @return true при успешной инициализации
             */
            bool init_mdns(const String& hostname);

            etl::optional<server_config_t> m_config;                ///< Конфигурация WiFi (опционально)
            etl::optional<ui_config_t> m_ui_config;                 ///< Конфигурация интерфейса (опционально)
            device_info_t m_device_info;                            ///< Информация об устройстве
            bool m_initialized = false;                             ///< Флаг инициализации
            connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения
            etl::shared_ptr<etl_web_server_t> m_server;             ///< HTTP сервер
        };

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
