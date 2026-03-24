#include <Arduino.h>
#include "version.h"
#include "pinout.h"

//////////////////////////////////////////////////////////////
// ETL library headers
#include "etl/etl_test.h"

//////////////////////////////////////////////////////////////
// WEB-UI
#ifndef ESP32
#define USE_WIFI_UI_SERVER
#include "etl_wifi_setup.h"
#include "etl/etl_littlefs.h"
etl::unique_ptr<etl::wifi::server_setup> wifi_server;   // Страница для выбора и настройки wifi сети и режима точки доступа

struct simulation_t {
    bool reset_wifi_on_start = true;    // Не считывать настройки, а заменить на значения по умолчанию
};
simulation_t simulation_data;   // Настройки тестирования

bool start_wifi_server() { // WiFi setup
    // setup available wi-fi points
    etl::wifi::server_config_t web_config; // default settings
    etl::wifi::device_info_t device_info;  // device information
    
    // В setup() или до начала работы с WiFi
    etl::wifi::settings::init_config(web_config, simulation_data.reset_wifi_on_start);

    // Настройка информации об устройстве
    device_info.name = F("ETL Test v" APP_VERSION_STRING);
    device_info.description = F("ESP8266/ESP32 ETL library test project");
    // Опционально: кастомная SVG иконка
    // device_info.icon_svg = "<svg>...</svg>";

    wifi_server = etl::make_unique<etl::wifi::server_setup>(web_config);
    if(wifi_server && wifi_server->begin(device_info)) {
        // Вывод информации о подключении
        const String& ip_addr = wifi_server->get_ip_address();
        const String& hostname_cfg = wifi_server->get_config().get_hostname();
        const String& mode = wifi_server->get_mode();

        Serial.println(F("\n=== WiFi Server Info ==="));
        Serial.print  (F("Mode:     ")); Serial.println(mode);
        Serial.print  (F("IP Addr:  ")); Serial.println(ip_addr.length() > 0 ? ip_addr : F("(AP IP: 192.168.4.1)"));
        Serial.print  (F("Hostname: http://")); Serial.print  (mode == "AP" ? F("192.168.4.1") : hostname_cfg + ".local"); Serial.println(F("/"));
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
#endif// ESP32
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
        wifi_server->handle();        // Обновление статуса WiFi
        wifi_server->handle_client(); // Обработка HTTP запросов
    }
#endif// USE_WIFI_UI_SERVER
}
