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
#include "etl/etl_settings.h"

// Алиас типа сервера для совместимости ESP8266 и ESP32
#if defined(ESP8266)
  using light_web_server_t = ESP8266WebServer;
#elif defined(ESP32)
  using light_web_server_t = WebServer;
#endif

#if defined(ESP8266) || defined(ESP32)

namespace light_control
{
    namespace data
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
            bool restore_power_on_start = true;  // Восстановление питания при старте

            /**
             * @brief Очистка настроек к значениям по умолчанию
             */
            void clear() {
                power = false;
                brightness = 100.0f;
                restore_power_on_start = true;
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

            void trace(const String& name) const {
                Serial.printf("[%s] power: %s; brightness: 🔆 %.1f;\n", name.c_str(), power ? "✅" : "🔳", brightness);
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

        // Глобальные данные для управления светом с сохранением настроек в постоянной памяти
        // записывается через 30 секунд для сохранения ресурса памяти
        etl::settings::app_data<kitchen_light_t>& app(); 

    } // namespace data

    namespace webui
    {
        /**
         * @brief Получить настройки устройства по умолчанию
         * @return device_info_t с информацией об устройстве
         */
        etl::webui::device_info_t get_light_control_device_info();

        /**
         * @brief Класс сервера управления лампой
         *
         * Предоставляет функционал для управления светодиодной лампой.
         * Работает в режиме точки доступа или подключается к внешней сети.
         *
         * Наследуется от web_server_base_t для поддержки полиморфизма.
         */
        class light_control_server : public etl::webui::web_server_base_t
        {
        public:
            /**
             * @brief Конструктор
             * @param cfg Конфигурация WiFi сервера (опционально, используется как fallback)
             */
            explicit light_control_server(const etl::optional<etl::webui::server_config_t>& cfg = {});

            /**
             * @brief Деструктор
             *
             * Виртуальный деструктор для корректного наследования.
             * Вызывает stop() для освобождения ресурсов.
             */
            virtual ~light_control_server() = default;

            /**
             * @brief Инициализация сервера контента
             *
             * Загружает настройки WiFi и UI из FS, пытается подключиться к WiFi,
             * если не удалось - запускает AP режим.
             *
             * @param device_info Информация об устройстве
             * @return true при успешной инициализации
             */
            virtual bool begin(const etl::webui::device_info_t& device_info) override;

            /**
             * @brief Установить настройки лампы
             * @param settings Настройки лампы
             */
            void set_light_settings(const data::kitchen_light_t& settings);

            /**
             * @brief Получить настройки лампы
             * @return Текущие настройки лампы
             */
            data::kitchen_light_t get_light_settings() const;

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
             * @brief Попытка подключения к WiFi из сохранённых настроек
             */
            void connect_from_saved_config();

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
             * @brief Обработчик API настроек интерфейса (только чтение)
             */
            virtual void handle_api_ui_config();

            /**
             * @brief Обработчик API текущего состояния лампы
             */
            virtual void handle_api_state();

            /**
             * @brief Обработчик API управления лампой
             */
            virtual void handle_api_control();

            /**
             * @brief Обработчик API конфигурации устройства (только чтение)
             */
            virtual void handle_api_config();

            /**
             * @brief Обработчик API настроек (кнопка Settings - переключение на server_setup)
             */
            virtual void handle_api_settings();

            /**
             * @brief Отправка состояния питания и яркости в Serial
             * @param power Состояние питания
             * @param brightness Яркость
             */
            void send_state_to_serial(bool power, float brightness);

            /**
             * @brief Остановка сервера с очисткой подписок
             */
            virtual void stop() override;

        private:
            bool m_subscribed = false;  ///< Флаг подписки на изменения настроек
            volatile bool m_update_UI = false; ///< Флаг указывающий, что нужно обновить UI по внешним данным
        };

    } // namespace webui
} // namespace light_control

#else
    #pragma message("light_webui: no implementation for this platform")
#endif
