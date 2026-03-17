#include <Arduino.h>
#include "version.h"

// ETL library headers
#include "etl/etl_test.h"

void setup() {
    Serial.begin(115200);
    
    // Задержка для ESP32 C3 Super Mini для корректного вывода в терминал
    #ifdef ESP32
        delay(1000);
    #endif
    
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
}

void loop() {
    // Основной цикл пустой - тесты выполняются один раз в setup()
    delay(1000);
}
