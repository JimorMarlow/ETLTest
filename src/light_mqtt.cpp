/**
 * @file light_mqtt.cpp
 * @brief Реализация MQTT менеджера для управления светом
 *
 * Интеграция с брокером wqtt.ru и light_control::data::app()
 */

#if defined(ESP8266) || defined(ESP32)

#include "light_mqtt.h"
#include "secret.h"

// Подключение light_control
// Замените на актуальный путь к вашему модулю управления светом
// #include "light_control_data.h"

namespace etl
{
    namespace mqtt
    {
        // Определение статических констант
        const char* light_manager::TOPIC_POWER_SET = "/home/guest/light/kitchen_workarea/set";
        const char* light_manager::TOPIC_BRIGHTNESS_SET = "/home/guest/light/kitchen_workarea/brightness/set";
        const char* light_manager::TOPIC_POWER_STATE = "/home/guest/light/kitchen_workarea/state";
        const char* light_manager::TOPIC_BRIGHTNESS_STATE = "/home/guest/light/kitchen_workarea/brightness/state";

        light_manager::light_manager()
        {
            Serial.println(F("[light_mqtt] Constructor"));
        }

        light_manager::~light_manager()
        {
            stop();
            Serial.println(F("[light_mqtt] Destructor"));
        }

        bool light_manager::begin(etl::shared_ptr<etl::wifi::manager> wifi_mgr,
                                   light_control::data::app* light_app)
        {
            Serial.println(F("[light_mqtt] begin()"));

#ifndef HAS_MQTT_SECRETS
            Serial.println(F("[light_mqtt] ERROR: secret.h not found, MQTT disabled"));
            return false;
#endif

            if (!wifi_mgr)
            {
                Serial.println(F("[light_mqtt] ERROR: wifi_mgr is null"));
                return false;
            }

            if (!light_app)
            {
                Serial.println(F("[light_mqtt] ERROR: light_app is null"));
                return false;
            }

            m_light_app = light_app;

            // Создание конфигурации MQTT
            mqtt::config_t config;
            strncpy(config.broker_host, MQTT_BROKER_HOST, sizeof(config.broker_host) - 1);
            config.broker_port = MQTT_BROKER_PORT;
            strncpy(config.username, MQTT_USERNAME, sizeof(config.username) - 1);
            strncpy(config.password, MQTT_PASSWORD, sizeof(config.password) - 1);
            strncpy(config.client_id, MQTT_CLIENT_ID, sizeof(config.client_id) - 1);
            config.enabled = true;

            // Создание MQTT менеджера
            m_mqtt_mgr = etl::make_shared<etl::mqtt::manager>(config);
            if (!m_mqtt_mgr)
            {
                Serial.println(F("[light_mqtt] ERROR: Failed to create MQTT manager"));
                return false;
            }

            // Установка WiFi менеджера
            m_mqtt_mgr->set_wifi_manager(wifi_mgr);

            // Установка callback на входящие сообщения
            auto msg_cb = [this](const String& topic, const String& payload, size_t length) {
                this->on_mqtt_message(topic, payload, length);
            };
            m_mqtt_mgr->set_message_callback(msg_cb);

            // Установка callback на статус
            auto status_cb = [this](mqtt::status_t status) {
                this->on_mqtt_status_changed(status);
            };
            m_mqtt_mgr->subscribe_status(etl::settings::sender_id::user1, status_cb);

            // Инициализация
            if (!m_mqtt_mgr->begin())
            {
                Serial.println(F("[light_mqtt] ERROR: MQTT manager begin() failed"));
                return false;
            }

            // Настройка подписок
            setup_subscriptions();

            Serial.println(F("[light_mqtt] Initialized successfully"));
            return true;
        }

        void light_manager::stop()
        {
            Serial.println(F("[light_mqtt] stop()"));

            if (m_mqtt_mgr)
            {
                m_mqtt_mgr->stop();
                m_mqtt_mgr = nullptr;
            }

            m_light_app = nullptr;
        }

        void light_manager::tick()
        {
            if (m_mqtt_mgr)
            {
                m_mqtt_mgr->tick();
            }
        }

        void light_manager::publish_light_state(bool power, uint8_t brightness)
        {
            if (!m_mqtt_mgr || !m_mqtt_mgr->is_connected())
            {
                return;
            }

            // Публикация состояния питания
            String power_payload = power ? "1" : "0";
            m_mqtt_mgr->publish(String(TOPIC_POWER_STATE), power_payload, true);

            // Публикация яркости
            String brightness_payload = String(brightness);
            m_mqtt_mgr->publish(String(TOPIC_BRIGHTNESS_STATE), brightness_payload, true);

            Serial.printf("[light_mqtt] Published state: power=%s, brightness=%d\n",
                          power ? "ON" : "OFF", brightness);
        }

        bool light_manager::is_connected() const
        {
            return m_mqtt_mgr && m_mqtt_mgr->is_connected();
        }

        void light_manager::setup_subscriptions()
        {
            if (!m_mqtt_mgr)
            {
                return;
            }

            // Подписка на топик управления питанием
            m_mqtt_mgr->subscribe(String(TOPIC_POWER_SET), 0);

            // Подписка на топик управления яркостью
            m_mqtt_mgr->subscribe(String(TOPIC_BRIGHTNESS_SET), 0);

            Serial.println(F("[light_mqtt] Subscriptions configured"));
        }

        void light_manager::on_mqtt_message(const String& topic, const String& payload, size_t length)
        {
            Serial.printf("[light_mqtt] Message: %s = %s\n", topic.c_str(), payload.c_str());

            // Обработка топика питания
            if (topic == String(TOPIC_POWER_SET))
            {
                if (m_light_app)
                {
                    bool power = (payload == "1" || payload.equalsIgnoreCase("on") || payload.equalsIgnoreCase("true"));
                    Serial.printf("[light_mqtt] Setting power: %s\n", power ? "ON" : "OFF");
                    // m_light_app->set_power(power);
                    // Раскомментируйте при наличии реализации
                }
                return;
            }

            // Обработка топика яркости
            if (topic == String(TOPIC_BRIGHTNESS_SET))
            {
                if (m_light_app)
                {
                    int brightness = payload.toInt();
                    brightness = constrain(brightness, 0, 100);
                    Serial.printf("[light_mqtt] Setting brightness: %d\n", brightness);
                    // m_light_app->set_brightness(brightness);
                    // Раскомментируйте при наличии реализации
                }
                return;
            }

            Serial.printf("[light_mqtt] Unhandled topic: %s\n", topic.c_str());
        }

        void light_manager::on_mqtt_status_changed(mqtt::status_t status)
        {
            switch (status)
            {
            case mqtt::status_t::connected:
                Serial.println(F("[light_mqtt] Connected to MQTT broker"));
                // Публикация начального состояния при подключении
                if (m_light_app)
                {
                    // publish_light_state(m_light_app->get_power(), m_light_app->get_brightness());
                    // Раскомментируйте при наличии реализации
                }
                break;

            case mqtt::status_t::disconnected:
                Serial.println(F("[light_mqtt] Disconnected from MQTT broker"));
                break;

            case mqtt::status_t::error:
                Serial.println(F("[light_mqtt] MQTT error"));
                break;

            default:
                break;
            }
        }

    } // namespace mqtt
} // namespace etl

#else
    #pragma message("light_mqtt: no implementation for this platform")
#endif
