/**
 * @file light_mqtt.cpp
 * @brief Реализация MQTT менеджера для управления светом
 *
 * Интеграция с брокером wqtt.ru и light_control::data::app()
 */

#if defined(ESP8266) || defined(ESP32)

#include "secret.h"
#include "light_amqtt.h"
#include "light_webui.h"

namespace light_amqtt
{
    // Определение статических констант
    const char* light_manager::TOPIC_POWER_SET = ::mqtt::topic::kitchen::workarea::power::set;
    const char* light_manager::TOPIC_POWER_STATE = ::mqtt::topic::kitchen::workarea::power::state;
    
    const char* light_manager::TOPIC_BRIGHTNESS_SET = ::mqtt::topic::kitchen::workarea::brightness::set;
    const char* light_manager::TOPIC_BRIGHTNESS_STATE = ::mqtt::topic::kitchen::workarea::brightness::state;

    light_manager::light_manager()
    {
        Serial.println(F("[light_mqtt] Constructor"));
    }

    light_manager::~light_manager()
    {
        stop();
        Serial.println(F("[light_mqtt] Destructor"));
    }

    bool light_manager::begin(etl::shared_ptr<etl::wifi::manager> wifi_mgr)
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

        // Создание конфигурации MQTT
        etl::amqtt::config_t config;
        strncpy(config.broker_host, MQTT_BROKER_HOST, sizeof(config.broker_host) - 1);
        config.broker_port = MQTT_BROKER_PORT;
        strncpy(config.username, MQTT_USERNAME, sizeof(config.username) - 1);
        strncpy(config.password, MQTT_PASSWORD, sizeof(config.password) - 1);
        strncpy(config.client_id, MQTT_CLIENT_ID, sizeof(config.client_id) - 1);
        config.enabled = true;

        // Создание MQTT менеджера
        m_mqtt_mgr = etl::make_shared<etl::amqtt::manager>(config);
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
        auto status_cb = [this](etl::amqtt::status_t status) {
            this->on_mqtt_status_changed(status);
        };
        m_mqtt_mgr->subscribe_status(etl::settings::sender_id::user1, status_cb);

        // Инициализация
        if (!m_mqtt_mgr->begin())
        {
            Serial.println(F("[light_mqtt] ERROR: MQTT manager begin() failed"));
            return false;
        }

        // Подписка на изменения light_control::data::app()
        // При изменении данных из webui - публикуем состояние в MQTT
        m_light_subscribed = light_control::data::app().subscribe(
            etl::settings::sender_id::mqtt,
            [this](etl::settings::sender_id source) {
                // Если изменения пришли НЕ от mqtt (например из webui), публикуем в MQTT
                if (source != etl::settings::sender_id::mqtt)
                {
                    Serial.println(F("[light_mqtt] light_control data changed (not from MQTT), publishing state"));
                    publish_current_state();
                }
            }
        );

        if (m_light_subscribed)
        {
            Serial.println(F("[light_mqtt] Subscribed to light_control::data::app()"));
        }
        else
        {
            Serial.println(F("[light_mqtt] WARNING: Failed to subscribe to light_control data"));
        }

        // Настройка подписок на MQTT топики
        setup_subscriptions();

        Serial.println(F("[light_mqtt] Initialized successfully"));
        return true;
    }

    void light_manager::stop()
    {
        Serial.println(F("[light_mqtt] stop()"));

        // Отписка от light_control::data::app()
        if (m_light_subscribed)
        {
            light_control::data::app().unsubscribe(etl::settings::sender_id::mqtt);
            m_light_subscribed = false;
            Serial.println(F("[light_mqtt] Unsubscribed from light_control data"));
        }

        if (m_mqtt_mgr)
        {
            m_mqtt_mgr->stop();
            m_mqtt_mgr = nullptr;
        }
    }

    void light_manager::tick()
    {
        if (m_mqtt_mgr)
        {
            m_mqtt_mgr->tick();
        }
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
            bool power = (payload == "1" || payload.equalsIgnoreCase("on") || payload.equalsIgnoreCase("true"));
            Serial.printf("[light_mqtt] Setting power: %s\n", power ? "ON" : "OFF");

            // Применяем изменение к устройству
            if (auto current = light_control::data::app().get(); current)
            {
                light_control::data::kitchen_light_t updated = *current;
                updated.power = power;
                // Устанавливаем данные с идентификатором mqtt, чтобы webui обновился
                light_control::data::app().set(updated, etl::settings::sender_id::mqtt);
            }
            return;
        }

        // Обработка топика яркости
        if (topic == String(TOPIC_BRIGHTNESS_SET))
        {
            float brightness = payload.toFloat();
            brightness = constrain(brightness, 0.0f, 100.0f);
            Serial.printf("[light_mqtt] Setting brightness: %.1f\n", brightness);

            // Применяем изменение к устройству
            if (auto current = light_control::data::app().get(); current)
            {
                light_control::data::kitchen_light_t updated = *current;
                updated.brightness = brightness;
                // Устанавливаем данные с идентификатором mqtt, чтобы webui обновился
                light_control::data::app().set(updated, etl::settings::sender_id::mqtt);
            }
            return;
        }

        Serial.printf("[light_mqtt] Unhandled topic: %s\n", topic.c_str());
    }

    void light_manager::on_mqtt_status_changed(etl::amqtt::status_t status)
    {
        switch (status)
        {
        case etl::amqtt::status_t::connected:
            Serial.println(F("[light_mqtt] Connected to MQTT broker"));
            // Публикация начального состояния при подключении
            publish_current_state();
            break;

        case etl::amqtt::status_t::disconnected:
            Serial.println(F("[light_mqtt] Disconnected from MQTT broker"));
            break;

        case etl::amqtt::status_t::error:
            Serial.println(F("[light_mqtt] MQTT error"));
            break;

        default:
            break;
        }
    }

    void light_manager::publish_current_state()
    {
        if (!m_mqtt_mgr || !m_mqtt_mgr->is_connected())
        {
            return;
        }

        // Чтение текущего состояния
        if (auto current = light_control::data::app().get(); current)
        {
            // Публикация состояния питания
            String power_payload = current->power ? "1" : "0";
            m_mqtt_mgr->publish(String(TOPIC_POWER_STATE), power_payload, true);

            // Публикация яркости
            String brightness_payload = String(static_cast<int>(current->brightness));
            m_mqtt_mgr->publish(String(TOPIC_BRIGHTNESS_STATE), brightness_payload, true);

            Serial.printf("[light_mqtt] Published state: power=%s, brightness=%.1f\n",
                            current->power ? "ON" : "OFF", current->brightness);
        }
    }

    // MQTT менеджер для управления светом
    etl::shared_ptr<light_amqtt::light_manager> g_light_mqtt_mgr;
    etl::shared_ptr<light_amqtt::light_manager> get_light_mqtt_mgr() {
        return g_light_mqtt_mgr;        
    }
    void set_light_mqtt_mgr(etl::shared_ptr<light_amqtt::light_manager> mgr) {
        g_light_mqtt_mgr = mgr;
    }

} // namespace light_mqtt

#else
    #pragma message("light_mqtt: no implementation for this platform")
#endif
