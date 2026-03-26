# История разработки WebUI для Kitchen Light

## Дата начала: 26 марта 2026 г.

### Контекст
Проект переключения между WebUI сервером (управление устройством) и WiFi Setup сервером (настройка WiFi).

### Архитектурные решения

#### Режимы работы
| Режим | Активация | HTTP порт | mDNS | Поведение |
|-------|-----------|-----------|------|-----------|
| **WebUI** | Обычный старт | 80 | `kitchenlight.local` | Сервер управления устройством |
| **WiFi Setup** | 3 нажатия кнопки | 80 | `espdevice.local` | Сервер настройки WiFi |
| **Factory Reset** | Кнопка при старте | - | - | Сброс настроек, запуск WiFi Setup |

#### Переключение серверов
- **Один порт 80**, сервера переключаются (не работают одновременно)
- По умолчанию запускается **WebUI Server**
- При 3 нажатиях кнопки: WebUI → остановка → WiFi Setup → запуск
- После [Save & Reboot] или [Factory Reset]: WiFi Setup → остановка → WebUI → запуск

#### Структура классов
```
etl_web_server_base_t (базовый класс в ETL)
├── device_info_t (name, description, icon_svg)
├── begin(device_info_t)
├── stop()
├── handle()
└── is_initialized()

webui_server_t (WebUI Server)
└── наследуется от etl_web_server_base_t

wifi::server_setup_t (WiFi Setup Server)
└── наследуется от etl_web_server_base_t
```

### Прогресс разработки

#### Этап 1: Макет WebUI ✅
**Задача:** Создать HTML макет KitchenLight UI
**Файл:** `docs\web-wifi\qwen-webui.001.html`
**Статус:** Завершено

**Реализованные элементы:**
- Header с названием устройства
- **Индикатор WiFi** (connected / ap-mode / disconnected)
- **Кнопка WiFi Setup** (иконка ⚙ в header)
- Device Info (иконка, название, описание)
- Status section (онлайн/офлайн)
- Light Control:
  - Power toggle (переключатель)
  - Brightness slider (0-100%)
  - Light preview
- Переключатель языков EN/RU
- Кнопка размера шрифта
- Модальное окно подтверждения

**API endpoints (ожидает реализации):**
- `GET /api/config` — информация об устройстве
- `GET /api/status` — статус (power, brightness, connected, wifi_mode, ssid)
- `POST /api/light` — управление светом {power: bool, brightness: int}
- `POST /api/wifi_setup` — переключение в режим настройки WiFi

#### Этап 2: Базовый класс Web Server ⏳
**Задача:** Создать базовый класс в ETL
**Файл:** `ETL/src/etl/etl_web_server_base.h`
**Статус:** Не начато

**План:**
- Перенести `device_info_t` из `etl_wifi_setup.h`
- Создать `etl_web_server_base_t` с виртуальными методами
- Обновить WiFi Setup Server для наследования

#### Этап 3: Реализация WebUI Server ⏳
**Задача:** Создать WebUI Server
**Файлы:** `src/webui_server.h`, `src/webui_server.cpp`, `src/webui_html.h`
**Статус:** Не начато

#### Этап 4: Менеджер серверов ⏳
**Задача:** Создать менеджер переключения
**Файл:** `src/server_manager.h/cpp`
**Статус:** Не начато

#### Этап 5: Интеграция в main.cpp ⏳
**Задача:** Обновить main.cpp с переключением серверов
**Статус:** Не начато

#### Этап 6: Тестирование ⏳
**Статус:** Не начато

---

### Требования к макету (согласованы)
1. ✅ Индикатор WiFi для показа статуса подключения
2. ✅ Кнопка настроек WiFi в интерфейсе (сверху справа)
3. ✅ Переключение языков EN/RU
4. ✅ Регулировка размера шрифта
5. ✅ Адаптивный дизайн (мобильные + десктоп)
6. ✅ iOS-like стиль (как в WiFi Setup)

---

### Следующие шаги (для продолжения)
1. **Согласовать макет** с пользователем
2. **Создать базовый класс** `etl_web_server_base_t` в ETL библиотеке
3. **Обновить WiFi Setup Server** для наследования от базового класса
4. **Создать WebUI Server** с KitchenLight функционалом
5. **Создать Server Manager** для переключения между серверами
6. **Интегрировать в main.cpp** с симуляцией кнопок
7. **Протестировать** на всех платформах (nodemcuv3, esp32c3, esp32-wroom-32u)

---

### Примечания
- Настройки WiFi перемещены в ETL библиотеку
- Симуляция кнопок через `simulation_t` в main.cpp
- Оба сервера используют общий порт 80
- mDNS имена разные: `kitchenlight.local` (WebUI) и `espdevice.local` (WiFi Setup)
