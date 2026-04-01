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
#include <etl/etl_memory.h>
#include <etl/etl_optional.h>

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
         * @brief Статус подключения к WiFi
         */
        enum class connection_status_t : uint8_t
        {
            disconnected,     // Не подключено
            connecting,       // В процессе подключения
            connected,        // Подключено к WiFi
            ap_mode,          // Режим точки доступа
            error             // Ошибка подключения
        };

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
             */
            web_server_base_t() = default;

            bool m_initialized = false;                 ///< Флаг инициализации
            connection_status_t m_connection_status = connection_status_t::disconnected;  ///< Статус подключения
        };

    } // namespace webui
} // namespace etl

#else
    #pragma message("etl_webui_base: no implementation for this platform")
#endif
