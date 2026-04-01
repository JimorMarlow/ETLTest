# История разработки: Переключение между WebUI и WiFi Setup

## Контекст

Для ESP устройства реализуется переключение между веб-интерфейсом управления датчиками и сервером настройки WiFi.

## Текущее состояние

### Выполнено

#### Этап 1: Макет WebUI
- [x] **Task 1.1:** Создан HTML макет для условной подсветки рабочей зоны
  - Файл: `docs\web-wifi\qwen-webui.001.html` (базовая версия)
  - Файл: `docs\web-wifi\qwen-webui.002.html` (текущая версия)
  
  **Реализованные функции в qwen-webui.002.html:**
  - Status bar с иконкой устройства и статусными индикаторами [W] [M] [T]
  - Кнопка настроек с переключением темной темы
  - Power button с индикацией включения
  - Brightness section со слайдером и кнопками +/-
  - Темная тема в стиле iOS (цвета из docs/etl_wifi_setup.md)
  - Адаптивный дизайн для мобильных устройств

#### Подготовка к разделению (Task 2.x)
- [x] **Task 2.1:** Изменить namespace etl::wifi на etl::webui (отмечено в task_webui.md)
- [x] **Task 2.2:** Проверить сборку всех проектов (отмечено в task_webui.md)
- [x] **Task 2.3:** Переименован файл истории в task_webui_hystory.md

### В процессе

### Ожидает выполнения

[STOP] - Дальнейшие задачи в процессе продумывания

---

## История изменений

### 1 апреля 2026 г.

**qwen-webui.002.html:**
- Добавлена рамка вокруг кнопки settings (в стиле кнопок brightness +/-)
- Power button смещена вверх (margin-top: -20px) для центрирования
- Иконка устройства перенесена в status-bar
- Status иконки [W] [M] [T] размещены с gap 2px
- Header центрирован, device-info-container упрощен
- Добавлен обработчик кнопки settings - переключение темной темы
- Обновлена документация в docs/tasks/task_light_webui.md

**Переименование namespace wifi -> webui:**
- Обновлены файлы:
  - `lib\ETLTest\etl_wifi_setup.h` - namespace etl::wifi -> etl::webui
  - `lib\ETLTest\etl_wifi_setup.cpp` - namespace etl::wifi -> etl::webui
  - `lib\ETLTest\etl_wifi_setup_html.h` - namespace etl::wifi -> etl::webui
  - `src\main.cpp` - все ссылки на etl::wifi:: заменены на etl::webui::
- Успешная компиляция всех конфигураций:
  - ✅ nodemcuv3 (ESP8266)
  - ✅ esp32c3 (ESP32-C3)
  - ✅ esp32-wroom-32u (ESP32)
