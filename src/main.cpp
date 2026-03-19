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

    wifi_server = etl::make_unique<etl::wifi::server_setup>(web_config/*, simulation_data.reset_wifi_on_start*/);
    if(wifi_server && wifi_server->begin()) {
        // Вывод информации о подключении
        Serial.println(F("\n=== WiFi Server Info ==="));
        Serial.print(F("Mode: "));
        Serial.println(wifi_server->get_mode());
        Serial.print(F("IP Address: "));
        Serial.println(wifi_server->get_ip_address());
        Serial.print(F("Hostname: http://"));
        Serial.print(wifi_server->get_mode() == "AP" ? wifi_server->get_ip_address() : "espdevice.local");
        Serial.println(F("/"));
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

#ifdef USE_WIFI_UI_SERVER
    if(etl::little_fs::begin()) {
        Serial.println("[OK] etl::little_fs::begin()");
        start_wifi_server();
    }
    else{
        Serial.println("[ERROR] etl::little_fs::begin()");
    }
#endif// USE_WIFI_UI_SERVER
}

void loop() {
    // Основной цикл пустой - тесты выполняются один раз в setup()
    // TODO ...

    // Обработка клиентских запросов
#ifdef USE_WIFI_UI_SERVER
    if(wifi_server) {
        wifi_server->handle_client();
    }
#endif// USE_WIFI_UI_SERVER
}
