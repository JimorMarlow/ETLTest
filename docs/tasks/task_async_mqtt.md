# Работа с mqtt по асинхронному протоколу

Заменить PubSubClient на AsyncMqttClient

## AsyncMqttClient (by Marvin Roger) 

A non-blocking, asynchronous library specifically designed for ESP8266 and ESP32. 
Best for: ESP8266 and ESP32 power users.
Pros: Does not block the main loop; handles connection drops and background tasks automatically.
Cons: Requires additional "Async" dependencies; more complex to set up.
GitHub: https://github.com/marvinroger/async-mqtt-client