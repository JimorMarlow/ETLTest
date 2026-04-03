#pragma once
/**
 * @file light_webui.h
 * @brief Сервер контента для управления светодиодной лампой
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 *
 * Особенности:
 * - Управление питанием и яркостью лампы
 * - Веб-интерфейс для управления с телефона
 * - Поддержка настроек интерфейса (язык, тема, шрифт)
 * - Сохранение настроек в энергонезависимой памяти
 *
 * @note Наследуется от web_server_base_t для поддержки полиморфизма.
 *       Для хранения настроек устройства используется kitchen_light_t.
 */

#include "etl_webui_base.h"
#include "version.h"

// Алиас типа сервера для совместимости ESP8266 и ESP32
#if defined(ESP8266)
  using light_web_server_t = ESP8266WebServer;
#elif defined(ESP32)
  using light_web_server_t = WebServer;
#endif

#if defined(ESP8266) || defined(ESP32)

namespace etl
{
    namespace webui
    {
        /**
         * @brief Настройки устройства (светодиодная лампа)
         *
         * Содержит параметры для управления лампой.
         * Сохраняется в энергонезависимой памяти через FileData.
         */
        struct kitchen_light_t
        {
            bool power = false;                   // Состояние питания (вкл/выкл)
            float brightness = 100.0f;           // Яркость [1..100], по умолчанию 100
            bool restore_power_on_start = false;  // Восстановление питания при старте

            /**
             * @brief Очистка настроек к значениям по умолчанию
             */
            void clear() {
                power = false;
                brightness = 100.0f;
                restore_power_on_start = false;
            }

            /**
             * @brief Вывод настроек в Serial
             */
            void trace() const {
                Serial.println(F("=== kitchen_light_t settings ==="));
                Serial.printf("power               = %s\n", power ? "ON" : "OFF");
                Serial.printf("brightness          = %.1f\n", brightness);
                Serial.printf("restore_power_start = %s\n", restore_power_on_start ? "YES" : "NO");
            }

            /**
             * @brief Оператор присвоения
             * @param other Другой объект kitchen_light_t
             * @return Ссылка на текущий объект
             */
            kitchen_light_t& operator=(const kitchen_light_t& other) {
                if (this != &other) {
                    power = other.power;
                    brightness = other.brightness;
                    restore_power_on_start = other.restore_power_on_start;
                }
                return *this;
            }
        };

        /**
         * @brief Класс сервера управления лампой
         *
         * Предоставляет функционал для управления светодиодной лампой.
         * Работает в режиме точки доступа или подключается к внешней сети.
         *
         * Наследуется от web_server_base_t для поддержки полиморфизма.
         */
        class light_control_server : public web_server_base_t
        {
        public:
            /**
             * @brief Конструктор
             * @param cfg Конфигурация WiFi сервера (опционально)
             */
            explicit light_control_server(const etl::optional<server_config_t>& cfg = {});

            /**
             * @brief Деструктор
             *
             * Виртуальный деструктор для корректного наследования.
             * Вызывает stop() для освобождения ресурсов.
             */
            virtual ~light_control_server() = default;

            /**
             * @brief Получить настройки устройства по умолчанию
             * @return device_info_t с информацией об устройстве
             */
            static device_info_t get_light_control_device_info();

            /**
             * @brief Установить настройки лампы
             * @param settings Настройки лампы
             */
            void set_light_settings(const kitchen_light_t& settings);

            /**
             * @brief Получить настройки лампы
             * @return Текущие настройки лампы
             */
            kitchen_light_t get_light_settings() const;

            /**
             * @brief Установить состояние питания
             * @param power Состояние питания (true - включено, false - выключено)
             */
            void set_power(bool power);

            /**
             * @brief Получить состояние питания
             * @return true если питание включено
             */
            bool get_power() const;

            /**
             * @brief Установить яркость
             * @param brightness Яркость [1..100]
             */
            void set_brightness(float brightness);

            /**
             * @brief Получить яркость
             * @return Текущая яркость [1..100]
             */
            float get_brightness() const;

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
             * @brief Обработчик API статуса
             */
            virtual void handle_api_status();

            /**
             * @brief Обработчик API информации об устройстве
             */
            virtual void handle_api_device_info();

            /**
             * @brief Обработчик API настроек интерфейса
             */
            virtual void handle_api_ui_config();

            /**
             * @brief Обработчик API текущего состояния
             */
            virtual void handle_api_state();

            /**
             * @brief Обработчик API управления
             */
            virtual void handle_api_control();

            /**
             * @brief Обработчик API настроек интерфейса (GET/POST)
             */
            virtual void handle_api_ui_settings();

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
             * @brief Отправка состояния питания и яркости в Serial
             * @param power Состояние питания
             * @param brightness Яркость
             */
            void send_state_to_serial(bool power, float brightness);

        private:
            kitchen_light_t m_light_settings;  ///< Настройки лампы
        };

    } // namespace webui
} // namespace etl

#else
    #pragma message("light_webui: no implementation for this platform")
#endif
