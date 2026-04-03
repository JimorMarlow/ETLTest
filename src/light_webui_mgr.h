#pragma once
/**
 * @file light_webui_mgr.h
 * @brief Пользовательский менеджер управления серверами для светодиодной лампы
 *
 * Платформа: ESP8266 (NodeMCU v3), ESP32
 *
 * Особенности:
 * - Наследуется от etl::webui::web_manager
 * - Реализует создание сервера контента (light_control_server)
 * - Реализует создание сервера настроек (server_setup)
 * - Устанавливает callback'и для переключения между серверами
 */

#include "etl_webui_base.h"
#include "light_webui.h"

#if defined(ESP8266) || defined(ESP32)

namespace light_control {

/**
 * @brief Менеджер управления серверами для лампы
 *
 * Отвечает за переключение между сервером контента и сервером настроек.
 * Использует callback-механизм для связи с серверами.
 */
class light_webui_mgr : public etl::webui::web_manager
{
public:
    /**
     * @brief Конструктор
     * @param device_info Информация об устройстве
     */
    explicit light_webui_mgr(const etl::webui::device_info_t& device_info)
        : etl::webui::web_manager(device_info)
    {
    }

protected:
    /**
     * @brief Создать сервер контента
     *
     * Создаёт light_control_server с актуальными настройками WiFi.
     * Устанавливает callback'и для переключения между серверами.
     *
     * @return Указатель на созданный сервер контента
     */
    virtual etl::shared_ptr<etl::webui::web_server_base_t> on_create_content() override
    {
        Serial.println(F("[LightWebUIMgr] Creating content server..."));

        // Загружаем актуальные настройки WiFi
        auto web_config = etl::webui::settings::load_wifi_config();

        // Создаём сервер контента
        auto server = etl::make_shared<etl::webui::light_control_server>(
            web_config.has_value() ? web_config.value() : etl::webui::server_config_t()
        );

        // Устанавливаем callback'и через статические функции-обёртки
        s_content_server_ptr = this;
        server->set_on_settings_callback(&light_webui_mgr::on_settings_callback_static);
        server->set_on_factory_reset_callback(&light_webui_mgr::on_factory_reset_callback_static);

        return server;
    }

    /**
     * @brief Создать сервер настроек
     *
     * Создаёт server_setup с актуальными настройками WiFi.
     * Устанавливает callback'и для возврата к серверу контента и сброса настроек.
     *
     * @return Указатель на созданный сервер настроек
     */
    virtual etl::shared_ptr<etl::webui::web_server_base_t> on_create_settings() override
    {
        Serial.println(F("[LightWebUIMgr] Creating settings server..."));

        // Загружаем актуальные настройки WiFi
        auto web_config = etl::webui::settings::load_wifi_config();

        // Создаём сервер настроек
        auto server = etl::make_shared<etl::webui::server_setup>(
            web_config.has_value() ? web_config.value() : etl::webui::server_config_t()
        );

        // Устанавливаем callback'и через статические функции-обёртки
        s_settings_server_ptr = this;
        server->set_on_content_callback(&light_webui_mgr::on_content_callback_static);
        server->set_on_factory_reset_callback(&light_webui_mgr::on_factory_reset_callback_static);

        return server;
    }

private:
    // Статические указатели для callback'ов (т.к. лямбды не могут быть преобразованы в указатели на функции)
    inline static light_webui_mgr* s_content_server_ptr = nullptr;
    inline static light_webui_mgr* s_settings_server_ptr = nullptr;

    /**
     * @brief Статическая обёртка для callback настроек
     */
    static void on_settings_callback_static()
    {
        if (s_content_server_ptr) {
            s_content_server_ptr->start_settings();
        }
    }

    /**
     * @brief Статическая обёртка для callback контента
     */
    static void on_content_callback_static()
    {
        if (s_settings_server_ptr) {
            s_settings_server_ptr->start_content();
        }
    }

    /**
     * @brief Статическая обёртка для callback сброса настроек
     */
    static void on_factory_reset_callback_static()
    {
        if (s_content_server_ptr) {
            s_content_server_ptr->handle_factory_reset();
        } else if (s_settings_server_ptr) {
            s_settings_server_ptr->handle_factory_reset();
        }
    }

    /**
     * @brief Обработка сброса настроек
     *
     * Сбрасывает настройки WiFi и UI к значениям по умолчанию,
     * затем запускает сервер настроек.
     */
    void handle_factory_reset()
    {
        Serial.println(F("[LightWebUIMgr] Performing factory reset..."));

        // Сброс настроек WiFi
        etl::webui::server_config_t default_config;
        default_config.clear();
        etl::webui::settings::save_wifi_config(default_config);
        Serial.println(F("[LightWebUIMgr] WiFi settings reset to defaults"));

        // Сброс настроек интерфейса (если они были инициализированы)
        if (etl::webui::settings::load_ui_config().has_value()) {
            etl::webui::ui_config_t default_ui;
            default_ui.clear();
            etl::webui::settings::save_ui_config(default_ui);
            Serial.println(F("[LightWebUIMgr] UI settings reset to defaults"));
        }

        // Запускаем сервер настроек для повторной настройки
        start_settings();
    }
};

} // namespace light_control

#else
    #pragma message("light_webui_mgr: no implementation for this platform")
#endif
