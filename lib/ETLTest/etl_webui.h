#pragma once
/**
 * @file etl_webui.h
 * @brief WiFi Setup Server для первичной настройки WiFi подключения
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
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

#include "etl_webui_base.h"

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
         * @brief Класс WiFi Setup Server
         *
         * Предоставляет функционал для первичной настройки WiFi подключения.
         * Работает в режиме точки доступа или подключается к внешней сети.
         * 
         * Наследуется от web_server_base_t для поддержки полиморфизма.
         */
        class server_setup : public web_server_base_t
        {
        public:
            /**
             * @brief Конструктор
             * @param cfg Конфигурация WiFi сервера (опционально)
             */
            explicit server_setup(const etl::optional<server_config_t>& cfg = {});

            /**
             * @brief Деструктор
             *
             * Виртуальный деструктор для корректного наследования.
             * Вызывает stop() для освобождения ресурсов.
             */
            virtual ~server_setup() = default;

        protected:
            
            /**
             * @brief Запуск HTTP сервера
             */
            virtual void start_http_server() override;

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
             * @brief Обработчик API сохранения настроек интерфейса
             */
            virtual void handle_api_ui_settings();

        };

    } // namespace webui
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
 * etl::webui::server_setup wifi_server;
 * etl::webui::server_config_t wifi_config;
 * etl::webui::device_info_t device_info;
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
 *     std::vector<etl::webui::scan_result_t> networks;
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
