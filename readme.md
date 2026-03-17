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
