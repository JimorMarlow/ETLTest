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
#include "light_webui.h"
#include "etl_amqtt.h"
#include "etl/etl_memory.h"

namespace light_amqtt
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
         * @return true при успешной инициализации
         */
        bool begin(etl::shared_ptr<etl::wifi::manager> wifi_mgr);

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
        void on_mqtt_status_changed(etl::amqtt::status_t status);

        /**
         * @brief Публикация текущего состояния света
         *
         * Считывает данные из light_control::data::app() и публикует в MQTT
         */
        void publish_current_state();

    protected:
        // MQTT менеджер
        etl::shared_ptr<etl::amqtt::manager> m_mqtt_mgr;

        // Флаг подписки на изменения light_control::data::app()
        bool m_light_subscribed = false;

        // Топики
        static const char* TOPIC_POWER_SET;          ///< Топик установки питания
        static const char* TOPIC_BRIGHTNESS_SET;     ///< Топик установки яркости
        static const char* TOPIC_POWER_STATE;        ///< Топик состояния питания
        static const char* TOPIC_BRIGHTNESS_STATE;   ///< Топик состояния яркости
    };

    // MQTT менеджер для управления светом
    etl::shared_ptr<light_amqtt::light_manager> get_light_mqtt_mgr();        
    void set_light_mqtt_mgr(etl::shared_ptr<light_amqtt::light_manager> mgr);        

} // namespace light_mqtt

#else
    #pragma message("light_mqtt: no implementation for this platform")
#endif
