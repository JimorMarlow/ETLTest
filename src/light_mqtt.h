#pragma once
/**
 * @file light_mqtt.h
 * @brief MQTT менеджер для управления светом
 *
 * Платформа: ESP8266 (NodeMCU v3, D1 Mini Lite), ESP32 (C3, WROOM-32U)
 *
 * Особенности:
 * - Интеграция с брокером wqtt.ru
 * - Подписка на топики управления светом
 * - Публикация состояния света
 * - Связь с light_control::data::app()
 *
 * @note Требует наличия secret.h с настройками подключения
 */

#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#include "etl_mqtt.h"
#include "etl/etl_memory.h"

// Forward declaration
namespace light_control {
    namespace data {
        class app;
    }
}

namespace etl
{
    namespace mqtt
    {
        /**
         * @brief Менеджер MQTT для управления светом
         *
         * Специализированная реализация для интеграции с wqtt.ru
         * и управления освещением через light_control::data::app()
         */
        class light_manager
        {
        public:
            /**
             * @brief Конструктор
             */
            light_manager();

            /**
             * @brief Деструктор
             */
            ~light_manager();

            /**
             * @brief Инициализация менеджера
             *
             * Настраивает подключение к wqtt.ru и подписку на топики.
             *
             * @param wifi_mgr Указатель на WiFi менеджер
             * @param light_app Указатель на приложение управления светом
             * @return true при успешной инициализации
             */
            bool begin(etl::shared_ptr<etl::wifi::manager> wifi_mgr,
                       light_control::data::app* light_app);

            /**
             * @brief Остановка менеджера
             */
            void stop();

            /**
             * @brief Неблокирующий цикл обработки
             *
             * Вызывать регулярно из loop() для поддержания соединения
             * и обработки входящих сообщений.
             */
            void tick();

            /**
             * @brief Публикация состояния света
             *
             * Вызывать при изменении состояния света для отправки
             * актуальных данных в MQTT.
             *
             * @param power Состояние питания (true/false)
             * @param brightness Яркость (0-100)
             */
            void publish_light_state(bool power, uint8_t brightness);

            /**
             * @brief Проверка подключения к MQTT
             * @return true если подключено к брокеру
             */
            bool is_connected() const;

        protected:
            /**
             * @brief Настройка подписки на топики управления
             */
            void setup_subscriptions();

            /**
             * @brief Обработчик входящих MQTT сообщений
             * @param topic Топик
             * @param payload Данные
             * @param length Длина
             */
            void on_mqtt_message(const String& topic, const String& payload, size_t length);

            /**
             * @brief Обработчик изменения статуса MQTT
             * @param status Новый статус
             */
            void on_mqtt_status_changed(mqtt::status_t status);

        protected:
            // MQTT менеджер
            etl::shared_ptr<etl::mqtt::manager> m_mqtt_mgr;

            // Ссылка на приложение управления светом
            light_control::data::app* m_light_app = nullptr;

            // Топики
            static const char* TOPIC_POWER_SET;          ///< Топик установки питания
            static const char* TOPIC_BRIGHTNESS_SET;     ///< Топик установки яркости
            static const char* TOPIC_POWER_STATE;        ///< Топик состояния питания
            static const char* TOPIC_BRIGHTNESS_STATE;   ///< Топик состояния яркости
        };

    } // namespace mqtt
} // namespace etl

#else
    #pragma message("light_mqtt: no implementation for this platform")
#endif
