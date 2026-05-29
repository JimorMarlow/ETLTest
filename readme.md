# ETL Test

**ETL (Embedded Template Library) Test Project** — тестовый проект для проверки работы библиотеки [ETL](https://github.com/JimorMarlow/ETL) на различных микроконтроллерах ESP8266/ESP32.

![Version](https://img.shields.io/github/package-json/v/JimorMarlow/ETLTest)
![PlatformIO](https://img.shields.io/badge/PlatformIO-embedded-blue)

---

## 📋 Описание

Проект предназначен для:
- Тестирования компонентов библиотеки **ETL** на реальных устройствах
- Проверки совместимости с различными платами ESP8266 и ESP32
- Отладки и верификации функциональности ETL

---

## 🎯 Поддерживаемые платы

| Плата | Микроконтроллер | Окружение PlatformIO |
|-------|-----------------|----------------------|
| WEMOS D1 Mini | ESP8266EX | `d1_mini_lite` |
| ESP32 C3 Super Mini | ESP32-C3FH4 | `esp32c3` |
| ESP32-WROOM-32U | ESP32 | `esp32-wroom-32u` |

### Link: [ESP32 30 Pin vs 38 Pin Development Boards](https://zaitronics.com.au/blogs/guides/esp32-30-pin-vs-38-pin-development-boards?srsltid=AfmBOooAZg5WLt3bgtmTELwcuG8imrWg5joXwL56mScAKI5R04TmMIhI)

### 30 Pin ESP32 – Pinout and Capabilities

Below is the standard 30 pin ESP32 development board pinout.
<br><img src="docs\images\ESP32-30PIN-DEVBOARD-PINOUT.webp" alt="ESP32 30 Pin Development Board Pinout Diagram" width="500">

Although the ESP32 chip supports up to 48 GPIOs internally, the 30 pin board exposes a practical subset suitable for most beginner and intermediate projects.

#### 30 Pin Key Capabilities
- 15 ADC Channels – 12-bit SAR ADC with selectable ranges of 0–1V, 0–1.4V, 0–2V, or 0–4V.
- 2 UART Interfaces – With flow control and IrDA support.
- 25 PWM Outputs – Ideal for motor control, LED dimming, and servo projects.
- 2 DAC Channels – Two 8-bit DACs for generating true analog voltages.
- SPI, I2C, and I2S – Three SPI interfaces, one I2C interface, and two I2S interfaces for displays, sensors, and audio projects.
- 9 Touch Pads – Capacitive touch capable GPIOs.
For most hobbyist builds such as relay control, sensor reading, OLED displays, or small IoT devices, the 30 pin version provides more than enough functionality.

### 38 Pin ESP32 – Pinout and Expanded GPIO Access

The 38 pin board exposes additional GPIOs that are not available on many 30 pin variants.
<br><img src="docs\images\ESP32-38_PIN-DEVBOARD-PINOUT.webp" alt="ESP32 38 Pin Development Board Pinout Diagram" width="500">

#### Key Pinout Details – 38 Pin Module
- Power – 3.3V, GND, and VIN 5V input.
- ADC Inputs – GPIOs 32–39, 34–36, 0, 2, 4, 12–15, 25–27.
- DAC Outputs – GPIO25 DAC1 and GPIO26 DAC2.
- I2C – SDA GPIO21, SCL GPIO22.
- SPI – SD2 GPIO9, SD3 GPIO10, CMD GPIO11, CLK GPIO6, SD0 GPIO7, SD1 GPIO8.
- UART – TX0/RX0 GPIO1/GPIO3, TX2/RX2 GPIO17/GPIO16.
- Touch Pins – T0–T9 on GPIO 4, 0, 2, 15, 13, 12, 14, 27, 33, 32.
Functionally, both boards are extremely similar because they use the same ESP32 chip. The 38 pin version simply gives you access to more physical GPIO header pins, which can be important in larger projects involving multiple sensors, relays, displays, and communication modules.

---

## 🚀 Быстрый старт

### Требования

- [PlatformIO](https://platformio.org/) (расширение для VS Code или CLI)
- Драйверы для вашей платы (CP2102, CH340, и т.д.)

### Установка

1. Клонируйте репозиторий:
   ```bash
   git clone https://github.com/JimorMarlow/ETLTest.git
   cd ETLTest
   ```

2. Откройте проект в VS Code с установленным PlatformIO

3. Выберите окружение для вашей платы в `platformio.ini`

4. Соберите и загрузите прошивку:
   ```bash
   pio run --target upload --environment <ваше_окружение>
   ```

5. Откройте Serial Monitor (скорость 115200):
   ```bash
   pio device monitor
   ```

---

## 📁 Структура проекта

```
ETLTest/
├── .vscode/                 # Настройки Visual Studio Code
├── include/                 # Заголовочные файлы проекта
├── src/
│   ├── main.cpp             # Основной код с тестами ETL
│   └── version.h            # Управление версией проекта
├── test/                    # Unit-тесты (опционально)
├── .gitignore               # Игнорируемые файлы Git
├── package.json             # Метаданные проекта
├── platformio.ini           # Конфигурация PlatformIO
├── readme.md                # Документация
└── sync_version.py          # Скрипт синхронизации версий
```

---

## 🔧 Конфигурация

### Зависимости

Проект использует следующие библиотеки (указаны в `platformio.ini`):

- **[ETL](https://github.com/JimorMarlow/ETL)** — основная библиотека для тестирования

### Локальная отладка ETL

Для тестирования локальной версии ETL измените `platformio.ini`:

```ini
[env]
lib_deps = 
    ; ../ETL  ; раскомментируйте для локальной сборки
```

---

## 📊 Версионность

Версия проекта управляется через файл `src/version.h`:

```cpp
#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 0
```

После изменения версии запустите скрипт синхронизации:

```bash
python sync_version.py
```

Скрипт автоматически обновит версию в `package.json`.

---

## 🔗 Ссылки

- [ETL Library](https://github.com/JimorMarlow/ETL) — основная библиотека
- [PlatformIO Documentation](https://docs.platformio.org/) — документация PlatformIO
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino) — ядро ESP8266
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) — ядро ESP32

---

## 📝 Лицензия

Этот проект распространяется в рамках лицензии основной библиотеки ETL.

---

## 👤 Автор

JimorMarlow

---

## 🤝 Вклад в проект

Если вы нашли ошибку или хотите предложить улучшения, создайте issue или pull request в репозитории.

## 📋 Hystory
### v0.1.2 
- added NodeMCU v3 config
- conditional build for etl_wifi_setup only for ESP8266 (ESP32 will be added later)

### v1.1.3
Added 2 new ESP32 boards [SAMIROB WROOM ESP32](https://ali.click/min4d1z)
SAMIROB WROOM ESP32 Development Board 30Pin/38Pin Micro/Type-C USB WiFi+Bluetooth Ultra-Low Power Consumption Dual Core CPU
-added [env:esp32-wroom-30u]
-added [env:esp32-wroom-38u]


