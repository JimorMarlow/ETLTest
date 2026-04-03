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
    
};
simulation_t simulation_data;
//////////////////////////////////////////////////////////////

#define USE_WIFI_UI_SERVER
//////////////////////////////////////////////////////////////
// WEB-UI
#ifdef USE_WIFI_UI_SERVER
#include "etl_webui.h"
#include "light_webui.h"
#include "etl/etl_littlefs.h"
etl::shared_ptr<etl::webui::web_server_base_t> wifi_server;   // Страница для выбора и настройки wifi сети и режима точки доступа
bool start_wifi_server() { // WiFi setup

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
    etl::webui::device_info_t device_info;  // device information
    if(simulation_data.custom_device_info)
    {
        device_info.name = F("ETL Test v" APP_VERSION_STRING);
        device_info.description = F("ESP8266/ESP32 ETL library test project");
    }
    // Опционально: кастомная SVG иконка
    if(simulation_data.custom_icon_svg)
    {
        device_info.icon_svg = F("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'><style>.led{animation:blink 1.5s infinite}@keyframes blink{0%,100%{opacity:1}50%{opacity:0.5}}</style><path d='M8 32c0-8 6-14 14-14s14 6 14 14-6 14-14 14-14-6-14-14zm8 0c0 3 3 6 6 6s6-3 6-6-3-6-6-6-6 3-6 6z' fill='#1d436d'/><path d='M28 32c0-8 6-14 14-14s14 6 14 14-6 14-14 14-14-6-14-14zm8 0c0 3 3 6 6 6s6-3 6-6-3-6-6-6-6 3-6 6z' fill='#1d436d'/><path d='M48 32c0-8 6-14 14-14v28c-8 0-14-6-14-14z' fill='#1d436d'/><circle class='led' cx='22' cy='32' r='3' fill='#a2d6fd'/><circle class='led' cx='42' cy='32' r='3' fill='#a2d6fd'/><circle class='led' cx='62' cy='32' r='3' fill='#a2d6fd'/></svg>");
    }

    // wifi_server = etl::make_shared<etl::webui::server_setup>(web_config);
    // Тестовый сервер для контента
    device_info = etl::webui::get_light_control_device_info();
    wifi_server = etl::make_shared<etl::webui::light_control_server>(web_config);
    
    if(wifi_server && wifi_server->begin(device_info)) {
        // Вывод информации о подключении
        const String& ip_addr = wifi_server->get_ip_address();
        const String& hostname_cfg = wifi_server->get_wifi_config().has_value() ? wifi_server->get_wifi_config()->get_hostname() : "espdevice";
        const String& mode = wifi_server->get_mode();

        Serial.println(F("\n=== WiFi Server Info ==="));
        Serial.print  (F("Mode:     ")); Serial.println(mode);
        Serial.print  (F("IP Addr:  ")); Serial.println(ip_addr.length() > 0 ? ip_addr : F("(AP IP: 192.168.4.1)"));
        Serial.print  (F("Hostname: http://"));
        if (mode == "AP") {
            Serial.println(F("192.168.4.1"));
        } else {
            Serial.print(hostname_cfg);
            Serial.println(F(".local"));
        }
        Serial.print  (F("mDNS:     http://")); Serial.print  (hostname_cfg); Serial.println(F(".local"));
        Serial.println(F("=========================\n"));
        return true;
    }
    else {
        Serial.println(F("[ERROR] WiFi server initialization failed!"));
        return false;
    }

    return false;
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

    // Обработка клиентских запросов и WiFi событий
#ifdef USE_WIFI_UI_SERVER
    if(wifi_server) {
        wifi_server->tick();        // Обновление статуса WiFi и обработка HTTP запросов
    }
#endif// USE_WIFI_UI_SERVER
}
