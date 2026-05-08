/**
 * @file etl_mqtt.cpp
 * @brief Реализация менеджера MQTT подключений
 *
 * Платформа: ESP8266 (NodeMCU v3, D1 Mini Lite), ESP32 (C3, WROOM-32U)
 */

#if defined(ESP8266) || defined(ESP32)

#include "etl_amqtt.h"

namespace etl
{
    namespace amqtt
    {
        // ============================================================================
        // Конструктор/деструктор
        // ============================================================================

        manager::manager(const config_t& config)
            : m_config(config),
              m_mqtt_client(nullptr)
        {
            Serial.println(F("[etl::mqtt::manager] Constructor"));
        }

        manager::~manager()
        {
            stop();
            Serial.println(F("[etl::mqtt::manager] Destructor"));
        }

        // ============================================================================
        // begin() / stop()
        // ============================================================================

        bool manager::begin()
        {
            Serial.println(F("[etl::mqtt::manager] begin()"));

            if (!m_config.enabled)
            {
                Serial.println(F("[etl::mqtt::manager] MQTT is disabled"));
                m_status = status_t::disconnected;
                return false;
            }

            if (strlen(m_config.broker_host) == 0)
            {
                Serial.println(F("[etl::mqtt::manager] No broker host configured"));
                m_status = status_t::error;
                return false;
            }

            // Убедимся, что WiFi клиент в чистом состоянии
            // (важно после stop() и переключения серверов webui)
            m_wifi_client.stop();
            m_wifi_client = WiFiClient();

            // Создание MQTT клиента с обновлённым WiFi клиентом
            m_mqtt_client = etl::make_shared<PubSubClient>(m_wifi_client);
            if (!m_mqtt_client)
            {
                Serial.println(F("[etl::mqtt::manager] Failed to create MQTT client"));
                m_status = status_t::error;
                return false;
            }

            // Настройка клиента
            m_mqtt_client->setServer(m_config.broker_host, m_config.broker_port);
            m_mqtt_client->setCallback([this](char* topic, byte* payload, unsigned int length) {
                this->on_mqtt_message(topic, payload, length);
            });

            Serial.printf("[etl::mqtt::manager] MQTT client configured: %s:%d\n",
                          m_config.broker_host, m_config.broker_port);

            m_status = status_t::disconnected;
            return true;
        }

        void manager::stop()
        {
            Serial.println(F("[etl::mqtt::manager] stop()"));

            // Отключение от брокера
            if (m_mqtt_client && m_mqtt_client->connected())
            {
                m_mqtt_client->disconnect();
            }

            // Уничтожение MQTT клиента (он использует m_wifi_client, который теперь "сломан")
            m_mqtt_client = nullptr;

            // Сброс WiFi клиента - при следующем begin() будет создан новый
            // Это важно при переключении серверов webui, когда WiFi останавливается
            m_wifi_client.stop();
            m_wifi_client = WiFiClient();

            // Отписка от WiFi менеджера
            if (m_wifi_manager && m_wifi_subscribed)
            {
                m_wifi_manager->unsubscribe_status(etl::settings::sender_id::mqtt);
                m_wifi_subscribed = false;
            }

            // Очистка подписок
            m_subscription_count = 0;

            // Очистка WiFi менеджера
            m_wifi_manager = nullptr;

            m_status = status_t::disconnected;
            m_wifi_connected = false;
        }

        // ============================================================================
        // tick()
        // ============================================================================

        void manager::tick()
        {
            uint32_t now = millis();

            // Проверка необходимости переподключения
            if (m_status == status_t::disconnected || m_status == status_t::error)
            {
                if (can_connect() && (now - m_last_reconnect_time >= RECONNECT_DELAY_MS))
                {
                    attempt_connect();
                }
                return;
            }

            // Обработка MQTT цикла (не чаще чем каждые MQTT_LOOP_INTERVAL мс)
            if (m_mqtt_client && m_mqtt_client->connected())
            {
                if (now - m_last_loop_time >= MQTT_LOOP_INTERVAL)
                {
                    m_mqtt_client->loop();
                    m_last_loop_time = now;
                }
            }
            else if (m_status == status_t::connected)
            {
                // Соединение потеряно
                Serial.println(F("[etl::mqtt::manager] MQTT connection lost"));
                m_status = status_t::disconnected;
                notify_status_change(m_status);
            }
        }

        // ============================================================================
        // connect() / disconnect()
        // ============================================================================

        bool manager::connect()
        {
            Serial.println(F("[etl::mqtt::manager] connect()"));

            if (!can_connect())
            {
                Serial.println(F("[etl::mqtt::manager] WiFi not available, cannot connect"));
                m_status = status_t::error;
                return false;
            }

            return attempt_connect();
        }

        void manager::disconnect()
        {
            Serial.println(F("[etl::mqtt::manager] disconnect()"));

            if (m_mqtt_client && m_mqtt_client->connected())
            {
                m_mqtt_client->disconnect();
            }

            m_status = status_t::disconnected;
            notify_status_change(m_status);
        }

        // ============================================================================
        // publish() / subscribe() / unsubscribe()
        // ============================================================================

        bool manager::publish(const String& topic, const String& payload, bool retain /*= false*/)
        {
            if (!m_mqtt_client || !m_mqtt_client->connected())
            {
                Serial.println(F("[etl::mqtt::manager] publish() - not connected"));
                return false;
            }

            bool result = m_mqtt_client->publish(topic.c_str(), payload.c_str(), retain);
            if (!result)
            {
                Serial.printf("[etl::mqtt::manager] publish() failed: %s\n", topic.c_str());
            }
            return result;
        }

        bool manager::subscribe(const String& topic, uint8_t qos /*= 0*/)
        {
            Serial.printf("[etl::mqtt::manager] subscribe() topic: %s\n", topic.c_str());

            // Сохраняем подписку
            if (m_subscription_count >= 20)
            {
                Serial.println(F("[etl::mqtt::manager] subscribe() - max subscriptions reached"));
                return false;
            }

            // Проверяем дубликаты
            for (size_t i = 0; i < m_subscription_count; ++i)
            {
                if (String(m_subscriptions[i].topic) == topic)
                {
                    // Обновляем QoS
                    m_subscriptions[i].qos = qos;
                    Serial.println(F("[etl::mqtt::manager] subscribe() - subscription updated"));

                    // Если подключены - переподписываемся
                    if (m_mqtt_client && m_mqtt_client->connected())
                    {
                        return m_mqtt_client->subscribe(topic.c_str(), qos);
                    }
                    return true;
                }
            }

            // Добавляем новую подписку
            strncpy(m_subscriptions[m_subscription_count].topic, topic.c_str(), 127);
            m_subscriptions[m_subscription_count].topic[127] = '\0';
            m_subscriptions[m_subscription_count].qos = qos;
            m_subscription_count++;

            // Если подключены - подписываемся сразу
            if (m_mqtt_client && m_mqtt_client->connected())
            {
                return m_mqtt_client->subscribe(topic.c_str(), qos);
            }

            return true;
        }

        bool manager::unsubscribe(const String& topic)
        {
            Serial.printf("[etl::mqtt::manager] unsubscribe() topic: %s\n", topic.c_str());

            // Отписка от брокера
            if (m_mqtt_client && m_mqtt_client->connected())
            {
                m_mqtt_client->unsubscribe(topic.c_str());
            }

            // Удаление из списка
            for (size_t i = 0; i < m_subscription_count; ++i)
            {
                if (String(m_subscriptions[i].topic) == topic)
                {
                    // Сдвигаем оставшиеся
                    for (size_t j = i; j < m_subscription_count - 1; ++j)
                    {
                        m_subscriptions[j] = m_subscriptions[j + 1];
                    }
                    m_subscription_count--;
                    return true;
                }
            }

            return false;
        }

        // ============================================================================
        // Геттеры
        // ============================================================================

        bool manager::is_connected() const
        {
            return m_mqtt_client && m_mqtt_client->connected();
        }

        status_t manager::get_status() const
        {
            return m_status;
        }

        const config_t& manager::get_config() const
        {
            return m_config;
        }

        void manager::update_config(const config_t& config)
        {
            m_config = config;
        }

        // ============================================================================
        // Callback'и
        // ============================================================================

        void manager::set_message_callback(message_callback_t cb)
        {
            m_message_callback = std::move(cb);
        }

        bool manager::subscribe_status(etl::settings::sender_id id, status_callback_t cb)
        {
            uint8_t idx = static_cast<uint8_t>(id);
            if (idx >= static_cast<uint8_t>(etl::settings::sender_id::count)) return false;
            if (id == etl::settings::sender_id::broadcast) return false;

            m_status_callbacks[idx] = std::move(cb);
            Serial.printf("[etl::mqtt::manager] Status subscribed for id: %d\n", idx);
            return true;
        }

        bool manager::unsubscribe_status(etl::settings::sender_id id)
        {
            uint8_t idx = static_cast<uint8_t>(id);
            if (idx >= static_cast<uint8_t>(etl::settings::sender_id::count)) return false;

            if (m_status_callbacks[idx])
            {
                m_status_callbacks[idx] = nullptr;
                Serial.printf("[etl::mqtt::manager] Status unsubscribed for id: %d\n", idx);
                return true;
            }
            return false;
        }

        void manager::set_wifi_manager(etl::shared_ptr<etl::wifi::manager> wifi_mgr)
        {
            // Отписка от предыдущего менеджера
            if (m_wifi_manager && m_wifi_subscribed)
            {
                m_wifi_manager->unsubscribe_status(etl::settings::sender_id::mqtt);
                m_wifi_subscribed = false;
            }

            m_wifi_manager = wifi_mgr;

            // Подписка на уведомления от нового менеджера
            if (m_wifi_manager)
            {
                auto cb = [this](etl::wifi::status_t status) {
                    this->on_wifi_status_changed(status);
                };

                if (m_wifi_manager->subscribe_status(etl::settings::sender_id::mqtt, cb))
                {
                    m_wifi_subscribed = true;
                    Serial.println(F("[etl::mqtt::manager] Subscribed to WiFi manager status"));

                    // Обновляем текущий статус WiFi
                    auto wifi_status = m_wifi_manager->get_status();
                    m_wifi_connected = (wifi_status == etl::wifi::status_t::connected_sta ||
                                        wifi_status == etl::wifi::status_t::ap_sta_mode);
                }
            }
        }

        // ============================================================================
        // Protected методы
        // ============================================================================

        bool manager::attempt_connect()
        {
            if (!can_connect())
            {
                Serial.println(F("[etl::mqtt::manager] attempt_connect() - WiFi not available"));
                return false;
            }

            if (!m_mqtt_client)
            {
                Serial.println(F("[etl::mqtt::manager] attempt_connect() - MQTT client not initialized"));
                return false;
            }

            m_status = status_t::connecting;
            m_last_reconnect_time = millis();
            notify_status_change(m_status);

            Serial.printf("[etl::mqtt::manager] Connecting to MQTT: %s:%d as %s\n",
                          m_config.broker_host, m_config.broker_port, m_config.client_id);

            // LWT топик
            String lwt_topic = String(m_config.client_id) + "/status";
            String client_id_str(m_config.client_id);

            bool result;
            if (strlen(m_config.username) > 0 && strlen(m_config.password) > 0)
            {
                result = m_mqtt_client->connect(
                    client_id_str.c_str(),
                    m_config.username,
                    m_config.password,
                    lwt_topic.c_str(),
                    0, true, "offline"
                );
            }
            else
            {
                result = m_mqtt_client->connect(
                    client_id_str.c_str(),
                    lwt_topic.c_str(),
                    0, true, "offline"
                );
            }

            if (result)
            {
                m_status = status_t::connected;

                // Публикация online статуса
                m_mqtt_client->publish(lwt_topic.c_str(), "online", true);

                // Переподписка на все топики
                resubscribe_all();

                Serial.println(F("[etl::mqtt::manager] Connected to MQTT broker"));
                notify_status_change(m_status);
                return true;
            }

            int state = m_mqtt_client->state();
            Serial.printf("[etl::mqtt::manager] MQTT connection failed, state: %d\n", state);
            m_status = status_t::error;
            notify_status_change(m_status);
            return false;
        }

        void manager::resubscribe_all()
        {
            if (!m_mqtt_client || !m_mqtt_client->connected())
            {
                return;
            }

            Serial.printf("[etl::mqtt::manager] Resubscribing to %d topics\n", m_subscription_count);

            for (size_t i = 0; i < m_subscription_count; ++i)
            {
                bool result = m_mqtt_client->subscribe(m_subscriptions[i].topic, m_subscriptions[i].qos);
                Serial.printf("[etl::mqtt::manager]   Subscribe to %s: %s\n",
                              m_subscriptions[i].topic, result ? "OK" : "FAILED");
            }
        }

        void manager::on_mqtt_message(char* topic, byte* payload, unsigned int length)
        {
            String topic_str(topic);
            String payload_str;
            payload_str.reserve(length);
            for (unsigned int i = 0; i < length; ++i)
            {
                payload_str += (char)payload[i];
            }

            Serial.printf("[etl::mqtt::manager] Message received: %s = %s\n",
                          topic_str.c_str(), payload_str.c_str());

            if (m_message_callback)
            {
                m_message_callback(topic_str, payload_str, length);
            }
        }

        void manager::notify_status_change(status_t new_status)
        {
            for (uint8_t i = 0; i < static_cast<uint8_t>(etl::settings::sender_id::count); ++i)
            {
                if (m_status_callbacks[i])
                {
                    m_status_callbacks[i](new_status);
                }
            }
        }

        void manager::on_wifi_status_changed(etl::wifi::status_t wifi_status)
        {
            bool was_connected = m_wifi_connected;
            m_wifi_connected = (wifi_status == etl::wifi::status_t::connected_sta ||
                                wifi_status == etl::wifi::status_t::ap_sta_mode);

            Serial.printf("[etl::mqtt::manager] WiFi status changed: %d, connected: %s\n",
                          static_cast<int>(wifi_status), m_wifi_connected ? "YES" : "NO");

            // Если WiFi отключился - отключаем MQTT
            if (!m_wifi_connected && was_connected)
            {
                Serial.println(F("[etl::mqtt::manager] WiFi lost, disconnecting MQTT"));
                if (m_mqtt_client && m_mqtt_client->connected())
                {
                    m_mqtt_client->disconnect();
                }
                m_status = status_t::disconnected;
                notify_status_change(m_status);
            }

            // Если WiFi подключился - пытаемся подключить MQTT
            if (m_wifi_connected && !was_connected && m_config.enabled)
            {
                Serial.println(F("[etl::mqtt::manager] WiFi connected, attempting MQTT connection"));
                m_last_reconnect_time = 0; // Сразу пробуем подключиться
            }
        }

        bool manager::can_connect() const
        {
            return m_wifi_connected && m_config.enabled;
        }

    } // namespace mqtt
} // namespace etl

#else
    #pragma message("etl_mqtt: no implementation for this platform")
#endif
