#pragma once
/**
 * @file etl_wifi.h
 * @brief Менеджер WiFi подключений для ESP8266/ESP32
 *
 * Платформа: ESP8266 (NodeMCU v3, D1 Mini Lite), ESP32 (C3, WROOM-32U)
 *
 * Особенности:
 * - Управление подключениями в режимах AP, STA, STA+AP
 * - Асинхронная работа без блокировок через tick()
 * - Автопереподключение при разрывах соединения
 * - Система уведомлений о смене статуса через callback'и
 * - Наследуется от etl::settings::notify для подписки на изменения настроек
 *
 * @note Код будет перемещён в библиотеку ETL после отладки
 */

// Подключение WiFi библиотек
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else
  #pragma message("ERROR: no Wi-Fi lib specified")
#endif

#include <Arduino.h>
#include <functional>
#include "etl/etl_vector.h"
#include "etl_webui_settings.h"

#if defined(ESP8266) || defined(ESP32)

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Статус подключения WiFi менеджера
         */
        enum class status_t : uint8_t
        {
            disconnected,      ///< Не подключено
            connecting,        ///< В процессе подключения
            connected_sta,     ///< Подключено к WiFi (STA режим)
            ap_mode,           ///< Режим точки доступа
            ap_sta_mode,       ///< Одновременная работа AP + STA
            error              ///< Ошибка подключения
        };

        /**
         * @brief Режим работы WiFi
         */
        enum class mode_t : uint8_t
        {
            none,              ///< WiFi выключен
            ap,                ///< Только точка доступа
            sta,               ///< Только клиент
            ap_sta             ///< Точка доступа + клиент
        };

        /**
         * @brief Результат сканирования WiFi сети
         */
        struct scan_result_t
        {
            String ssid;               ///< SSID сети
            int32_t rssi;              ///< Уровень сигнала (dBm)
            String encryption;         ///< Тип шифрования (WPA2, WPA, Open)
            uint8_t channel;           ///< Канал
            bool connected = false;    ///< Флаг: подключено к этой сети
        };

        /**
         * @brief Callback тип для уведомлений о смене статуса
         * @param new_status Новый статус подключения
         */
        using status_callback_t = void(*)(status_t new_status);

        /**
         * @brief Менеджер WiFi подключений
         *
         * Отвечает за управление WiFi подключениями в различных режимах,
         * обработку разрывов соединения и уведомление подписчиков.
         *
         * Пример использования:
         * @code
         * etl::wifi::manager wifi_mgr;
         * wifi_mgr.begin();
         *
         * void loop() {
         *     wifi_mgr.tick();
         * }
         * @endcode
         */
        class manager
        {
        public:
            /**
             * @brief Конструктор
             * @param config Конфигурация WiFi (SSID, пароль, AP настройки)
             */
            explicit manager(const etl::webui::server_config_t& config);

            /**
             * @brief Виртуальный деструктор
             */
            virtual ~manager();

            /**
             * @brief Инициализация менеджера
             *
             * Загружает настройки, пытается подключиться к сохранённой сети.
             * Если не удалось - запускает AP режим.
             *
             * @return true при успешной инициализации
             */
            virtual bool begin();

            /**
             * @brief Остановка WiFi
             *
             * Отключается от всех сетей, останавливает mDNS.
             */
            virtual void stop();

            /**
             * @brief Неблокирующий цикл обработки
             *
             * Вызывать регулярно из loop() для:
             * - Мониторинга статуса WiFi
             * - Автопереподключения при разрывах
             * - Обработки отложенных операций
             */
            virtual void tick();

            /**
             * @brief Подключение к STA сети
             *
             * Начинает асинхронное подключение к указанной сети.
             * Результат отслеживается через tick() и get_status().
             *
             * @param ssid SSID сети
             * @param password Пароль сети
             * @param timeout Таймаут подключения в мс (по умолчанию 15000)
             * @return true если начато подключение
             */
            virtual bool connect(const String& ssid, const String& password, uint32_t timeout = 15000);

            /**
             * @brief Отключение от текущей сети
             */
            virtual void disconnect();

            /**
             * @brief Запуск точки доступа
             *
             * Запускает AP с параметрами из конфигурации.
             *
             * @param ssid SSID точки доступа (если empty - используется из config)
             * @param password Пароль точки доступа (если empty - используется из config)
             * @return true при успешном запуске
             */
            virtual bool start_ap(const String& ssid = "", const String& password = "");

            /**
             * @brief Переключение режима работы WiFi
             *
             * @param mode Новый режим работы
             * @return true при успешном переключении
             */
            virtual bool set_mode(mode_t mode);

            /**
             * @brief Получить текущий статус подключения
             * @return Текущий статус
             */
            virtual status_t get_status() const;

            /**
             * @brief Получить текущий режим работы
             * @return Текущий режим
             */
            virtual mode_t get_mode() const;

            /**
             * @brief Получить IP адрес
             * @return IP адрес в формате String
             */
            virtual String get_ip_address() const;

            /**
             * @brief Проверка подключения к внешней сети
             * @return true если подключено к STA сети
             */
            virtual bool is_connected() const;

            /**
             * @brief Сканирование доступных WiFi сетей
             *
             * Блокирующая операция с таймаутом.
             *
             * @param results Вектор для результатов сканирования
             * @return Количество найденных сетей
             */
            virtual int32_t scan_networks(etl::vector<scan_result_t>& results);

            /**
             * @brief Добавить callback на смену статуса
             *
             * @param cb Функция обратного вызова
             */
            virtual void add_status_callback(status_callback_t cb);

            /**
             * @brief Удалить callback статуса
             *
             * @param cb Функция обратного вызова для удаления
             */
            virtual void remove_status_callback(status_callback_t cb);

            /**
             * @brief Обновить конфигурацию WiFi
             *
             * Вызывается при изменении настроек через web-интерфейс.
             *
             * @param config Новая конфигурация
             */
            virtual void update_config(const etl::webui::server_config_t& config);

            /**
             * @brief Применить изменения настроек из системы настроек
             *
             * Вызывается при получении уведомления об изменении настроек.
             * Загружает актуальные настройки и применяет их.
             */
            virtual void apply_settings_changes();

            /**
             * @brief Получить текущую конфигурацию
             * @return Текущая конфигурация
             */
            virtual const etl::webui::server_config_t& get_config() const;

            /**
             * @brief Инициализация mDNS
             *
             * @param hostname Имя хоста для mDNS
             * @return true при успешной инициализации
             */
            virtual bool init_mdns(const String& hostname);

            /**
             * @brief Остановить mDNS
             */
            virtual void stop_mdns();

            /**
             * @brief Получить имя хоста
             * @return Имя хоста
             */
            virtual String get_hostname() const;

        protected:
            /**
             * @brief Подключение к STA сети (внутренний метод)
             *
             * @param timeout Таймаут подключения в мс
             * @return true при успешном подключении
             */
            virtual bool connect_to_sta(uint32_t timeout);

            /**
             * @brief Внутреннее обновление статуса
             *
             * Синхронизирует m_connection_status с WiFi.status()
             */
            virtual void update_status();

            /**
             * @brief Уведомить подписчиков о смене статуса
             *
             * @param new_status Новый статус
             */
            virtual void notify_status_change(status_t new_status);

            /**
             * @brief Получить тип шифрования из WiFi.encryptionType()
             *
             * @param type Тип шифрования
             * @return Строковое представление типа шифрования
             */
            virtual String get_encryption_type(uint8_t type) const;

            /**
             * @brief Попытка переподключения при разрыве
             *
             * Вызывается из tick() при обнаружении разрыва STA соединения.
             */
            virtual void attempt_reconnect();

            // Константы
            static const uint32_t SCAN_CACHE_TIME = 30000;          ///< Время кэширования сканирования (30 сек)
            static const uint8_t MAX_RECONNECT_ATTEMPTS = 5;        ///< Максимум попыток переподключения до перехода в AP
            static const uint32_t RECONNECT_DELAY_MS = 5000;        ///< Задержка между попытками переподключения

        protected:
            // Конфигурация
            etl::webui::server_config_t m_config;                   ///< Текущая конфигурация WiFi

            // Статус
            status_t m_status = status_t::disconnected;             ///< Текущий статус подключения
            mode_t m_mode = mode_t::none;                           ///< Текущий режим работы

            // Переподключение
            uint8_t m_reconnect_attempts = 0;                       ///< Счётчик попыток переподключения
            uint32_t m_last_reconnect_time = 0;                     ///< Время последней попытки
            bool m_reconnect_pending = false;                       ///< Флаг отложенного переподключения

            // Кэш сканирования
            etl::vector<scan_result_t> m_scan_cache;                ///< Кэш результатов сканирования
            uint32_t m_scan_timestamp = 0;                          ///< Время последнего сканирования

            // Callback'и
            etl::vector<status_callback_t> m_status_callbacks;      ///< Список подписчиков на статус

            // mDNS
            bool m_mdns_initialized = false;                        ///< Флаг инициализации mDNS
        };

    } // namespace wifi
} // namespace etl

#else
    #pragma message("etl_wifi: no implementation for this platform")
#endif
