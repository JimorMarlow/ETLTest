#pragma once
/**
 * @file etl_mqtt.h
 * @brief Менеджер MQTT подключений для ESP8266/ESP32
 *
 * Платформа: ESP8266 (NodeMCU v3, D1 Mini Lite), ESP32 (C3, WROOM-32U)
 *
 * Особенности:
 * - Подключение к MQTT брокеру через PubSubClient
 * - Асинхронная работа без блокировок через tick()
 * - Автопереподключение при разрывах соединения
 * - Подписка на статус WiFi через etl::wifi::manager
 * - Система уведомлений о смене статуса через callback'и
 * - Публикация и подписка на топики с автоматической переподпиской
 *
 * @note Код будет перемещён в библиотеку ETL после отладки
 */

#if defined(ESP8266) || defined(ESP32)

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>
#include "etl/etl_settings.h"
#include "etl/etl_memory.h"
#include "etl_webui_settings.h"
#include "etl_wifi.h"

// Forward declaration
namespace etl { namespace wifi { class manager; } }

namespace etl
{
    namespace mqtt
    {
        /**
         * @brief Статус подключения MQTT
         */
        enum class status_t : uint8_t
        {
            disconnected,      ///< Не подключено
            connecting,        ///< В процессе подключения
            connected,         ///< Подключено к брокеру
            error              ///< Ошибка подключения
        };

        /**
         * @brief Callback для входящих MQTT сообщений
         * @param topic Топик сообщения
         * @param payload Данные сообщения
         * @param length Длина данных
         */
        using message_callback_t = std::function<void(const String& topic, const String& payload, size_t length)>;

        /**
         * @brief Callback для уведомлений о смене статуса
         * @param new_status Новый статус
         */
        using status_callback_t = std::function<void(status_t new_status)>;

        /**
         * @brief Конфигурация MQTT подключения
         */
        struct config_t
        {
            char broker_host[64] = "";            ///< Адрес брокера
            uint16_t broker_port = 1883;          ///< Порт брокера
            char username[32] = "";               ///< Имя пользователя
            char password[64] = "";               ///< Пароль
            char client_id[32] = "esp_mqtt";      ///< Идентификатор клиента
            bool enabled = false;                 ///< Флаг включения MQTT

            /**
             * @brief Очистка конфигурации к значениям по умолчанию
             */
            void clear()
            {
                memset(broker_host, 0, sizeof(broker_host));
                memset(username, 0, sizeof(username));
                memset(password, 0, sizeof(password));
                memset(client_id, 0, sizeof(client_id));
                strncpy(client_id, "esp_mqtt", sizeof(client_id) - 1);
                broker_port = 1883;
                enabled = false;
            }

            /**
             * @brief Вывод конфигурации в Serial (без пароля)
             */
            void trace() const
            {
                Serial.println(F("=== mqtt::config_t ==="));
                Serial.printf("broker_host = %s\n", broker_host);
                Serial.printf("broker_port = %u\n", broker_port);
                Serial.printf("username    = %s\n", username);
                Serial.println(F("password    = ***"));
                Serial.printf("client_id   = %s\n", client_id);
                Serial.printf("enabled     = %s\n", enabled ? "YES" : "NO");
                Serial.println(F("===================="));
            }
        };

        /**
         * @brief Менеджер MQTT подключений
         *
         * Отвечает за подключение к MQTT брокеру, подписку/публикацию сообщений,
         * обработку разрывов соединения и уведомление подписчиков.
         *
         * Зависит от etl::wifi::manager - не пытается подключиться без WiFi.
         *
         * Пример использования:
         * @code
         * auto mqtt_mgr = etl::make_shared<etl::mqtt::manager>(config);
         * mqtt_mgr->set_wifi_manager(wifi_mgr);
         * mqtt_mgr->begin();
         *
         * void loop() {
         *     mqtt_mgr->tick();
         * }
         * @endcode
         */
        class manager
        {
        public:
            /**
             * @brief Конструктор
             * @param config Конфигурация MQTT
             */
            explicit manager(const config_t& config);

            /**
             * @brief Виртуальный деструктор
             */
            virtual ~manager();

            /**
             * @brief Инициализация менеджера
             *
             * Настраивает PubSubClient, но не подключается до вызова connect().
             *
             * @return true при успешной инициализации
             */
            virtual bool begin();

            /**
             * @brief Остановка MQTT
             *
             * Отключается от брокера, очищает подписки.
             */
            virtual void stop();

            /**
             * @brief Неблокирующий цикл обработки
             *
             * Вызывать регулярно из loop() для:
             * - Поддержания соединения с брокером (PubSubClient::loop())
             * - Автопереподключения при разрывах
             * - Обработки входящих сообщений
             */
            virtual void tick();

            /**
             * @brief Подключение к MQTT брокеру
             *
             * Проверяет наличие WiFi перед подключением.
             *
             * @return true если начато подключение
             */
            virtual bool connect();

            /**
             * @brief Отключение от MQTT брокера
             */
            virtual void disconnect();

            /**
             * @brief Публикация сообщения
             *
             * @param topic Топик для публикации
             * @param payload Данные сообщения
             * @param retain Флаг retain
             * @return true если сообщение отправлено
             */
            virtual bool publish(const String& topic, const String& payload, bool retain = false);

            /**
             * @brief Подписка на топик
             *
             * Топик сохраняется и будет автоматически переподписан после переподключения.
             *
             * @param topic Топик для подписки
             * @param qos Quality of Service (0, 1, 2)
             * @return true если подписка оформлена
             */
            virtual bool subscribe(const String& topic, uint8_t qos = 0);

            /**
             * @brief Отписка от топика
             *
             * @param topic Топик для отписки
             * @return true если отписка оформлена
             */
            virtual bool unsubscribe(const String& topic);

            /**
             * @brief Проверка подключения к брокеру
             * @return true если подключено
             */
            virtual bool is_connected() const;

            /**
             * @brief Получить текущий статус
             * @return Текущий статус подключения
             */
            virtual status_t get_status() const;

            /**
             * @brief Установить callback на входящие сообщения
             * @param cb Функция обратного вызова
             */
            virtual void set_message_callback(message_callback_t cb);

            /**
             * @brief Подписаться на уведомления о смене статуса
             *
             * @param id Идентификатор подписчика
             * @param cb Функция обратного вызова
             * @return true при успешной подписке
             */
            virtual bool subscribe_status(etl::settings::sender_id id, status_callback_t cb);

            /**
             * @brief Отписаться от уведомлений о статусе
             *
             * @param id Идентификатор подписчика
             * @return true если подписка была найдена и удалена
             */
            virtual bool unsubscribe_status(etl::settings::sender_id id);

            /**
             * @brief Установить ссылку на WiFi менеджер
             *
             * MQTT менеджер будет проверять статус WiFi перед подключением
             * и реагировать на изменения статуса WiFi.
             *
             * @param wifi_mgr Указатель на WiFi менеджер
             */
            virtual void set_wifi_manager(etl::shared_ptr<etl::wifi::manager> wifi_mgr);

            /**
             * @brief Получить конфигурацию
             * @return Текущая конфигурация
             */
            virtual const config_t& get_config() const;

            /**
             * @brief Обновить конфигурацию
             * @param config Новая конфигурация
             */
            virtual void update_config(const config_t& config);

        protected:
            /**
             * @brief Попытка подключения к брокеру
             * @return true при успешном подключении
             */
            virtual bool attempt_connect();

            /**
             * @brief Переподписка на все сохранённые топики
             */
            virtual void resubscribe_all();

            /**
             * @brief Обработчик входящих сообщений от PubSubClient
             * @param topic Топик
             * @param payload Данные
             * @param length Длина
             */
            virtual void on_mqtt_message(char* topic, byte* payload, unsigned int length);

            /**
             * @brief Уведомить подписчиков о смене статуса
             * @param new_status Новый статус
             */
            virtual void notify_status_change(status_t new_status);

            /**
             * @brief Обработчик изменения статуса WiFi
             */
            virtual void on_wifi_status_changed(etl::wifi::status_t wifi_status);

            /**
             * @brief Проверка возможности подключения (WiFi активен)
             * @return true если можно пытаться подключиться
             */
            virtual bool can_connect() const;

            // Константы
            static const uint32_t RECONNECT_DELAY_MS = 5000;        ///< Задержка между попытками переподключения
            static const uint32_t MQTT_LOOP_INTERVAL = 50;          ///< Минимальный интервал вызова loop()

        protected:
            // Конфигурация
            config_t m_config;                                      ///< Текущая конфигурация

            // Статус
            status_t m_status = status_t::disconnected;             ///< Текущий статус подключения

            // WiFi менеджер
            etl::wifi::manager* m_wifi_manager = nullptr;           ///< Ссылка на WiFi менеджер
            bool m_wifi_subscribed = false;                         ///< Флаг подписки на статус WiFi

            // PubSubClient
            WiFiClient m_wifi_client;                               ///< WiFi клиент для PubSubClient
            etl::shared_ptr<PubSubClient> m_mqtt_client;            ///< MQTT клиент

            // Подписки
            struct subscription_t
            {
                char topic[128];
                uint8_t qos;
            };
            subscription_t m_subscriptions[20];                     ///< Массив сохранённых подписок
            size_t m_subscription_count = 0;                        ///< Количество подписок

            // Переподключение
            uint32_t m_last_reconnect_time = 0;                     ///< Время последней попытки
            uint32_t m_last_loop_time = 0;                          ///< Время последнего вызова loop()

            // Callback'и
            message_callback_t m_message_callback;                  ///< Callback на входящие сообщения
            status_callback_t m_status_callbacks[static_cast<uint8_t>(etl::settings::sender_id::count)]; ///< Подписчики на статус

            // WiFi статус
            bool m_wifi_connected = false;                          ///< Флаг наличия WiFi подключения
        };

    } // namespace mqtt
} // namespace etl

#else
    #pragma message("etl_mqtt: no implementation for this platform")
#endif
