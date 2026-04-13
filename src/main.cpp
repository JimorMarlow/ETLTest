#include <Arduino.h>
#include "version.h"
#include "pinout.h"

//////////////////////////////////////////////////////////////
// ETL library headers
#include "etl/etl_test.h"

//////////////////////////////////////////////////////////////
// Настройки тестирования
struct simulation_t {
    bool reset_wifi_on_start = false;   // Не считывать настройки, а заменить на значения по умолчанию
    bool custom_device_info = true;     // Установить отладочную информацию об устройстве
    bool custom_icon_svg = false;       // Установить отладочную иконку для устройства

    bool init_ui_settings = true;       // Использовать настройки интерфейса в общих настройках
    bool reset_ui_on_start = false;      // Не считывать настройки, а заменить на значения по умолчанию
    bool start_webui_settings_on_start = false;      // При старте запустить сервер настроек (например по зажатой кнопке)
};
simulation_t simulation_data;
//////////////////////////////////////////////////////////////

#define USE_WIFI_UI_SERVER
//////////////////////////////////////////////////////////////
// WEB-UI
#ifdef USE_WIFI_UI_SERVER
#include "etl_webui.h"
#include "light_webui.h"
#include "light_webui_mgr.h"
#include "etl/etl_littlefs.h"

etl::shared_ptr<light_control::light_webui_mgr> webui_manager;   // Менеджер управления серверами

bool start_wifi_server() {
    // setup available wi-fi points
    etl::webui::server_config_t web_config; // default settings
    // В setup() или до начала работы с WiFi
    etl::webui::settings::init_wifi_config(web_config, simulation_data.reset_wifi_on_start);

    if(simulation_data.init_ui_settings)
    {
        etl::webui::ui_config_t ui_config; // default UI settings
        etl::webui::settings::init_ui_config(ui_config, simulation_data.reset_ui_on_start);
    }

    // Настройка информации об устройстве
    etl::webui::device_info_t device_info = light_control::webui::get_light_control_device_info();
    Serial.print("[DeviceInfo] name: "); Serial.println(device_info.name);

    // Создание менеджера управления серверами
    webui_manager = etl::make_shared<light_control::light_webui_mgr>(device_info);
    Serial.printf("[WebUI Mgr] create: %s\n", webui_manager ? "OK" : "FAIL");

    // Запуск нужного сервера
    if(simulation_data.start_webui_settings_on_start) {
        webui_manager->start_settings();
    } else {
        webui_manager->start_content();
    }

    return webui_manager->trace_connection();
}
#endif//USE_WIFI_UI_SERVER
//////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    if(SERIAL_INIT_DELAY > 0) delay(SERIAL_INIT_DELAY);  // Задержка для ESP32 C3 Super Mini для корректного вывода в терминал
        
    Serial.println();
    Serial.println("=================================");
    Serial.println("  ETL Test Project");
    Serial.println("=================================");
    Serial.print("Version: ");
    Serial.println(APP_VERSION_STRING);
    Serial.println();
    
    // Запуск всех тестов ETL
    Serial.println("Running ETL tests...");
    Serial.println();
    etl::unittest::test_all(Serial);
    
    Serial.println();
    Serial.println("=================================");
    Serial.println("  All tests completed");
    Serial.println("=================================");
    Serial.println();

    // Инициализация глобальных настроек тестового проекта
    light_control::data::app().init(light_control::data::kitchen_light_t{});
        
#ifdef USE_WIFI_UI_SERVER
    if(etl::little_fs::begin()) {
        Serial.println("[LittleFS] etl::little_fs::begin(): OK");
        start_wifi_server();
    }
    else{
        Serial.println("[LittleFS] etl::little_fs::begin(): FALIED");
    }
#endif// USE_WIFI_UI_SERVER
}

void loop() {
    // Основной цикл пустой - тесты выполняются один раз в setup()
    // TODO ...


    light_control::data::app().tick();  // Обработчик отложенной записи данных управления светом

    // Обработка клиентских запросов и WiFi событий через менеджер
#ifdef USE_WIFI_UI_SERVER
    if(webui_manager) {
        webui_manager->tick();        // Обновление статуса WiFi и обработка HTTP запросов
    }
#endif// USE_WIFI_UI_SERVER
}
