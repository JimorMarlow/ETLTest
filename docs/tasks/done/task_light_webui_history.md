# История изменений по задаче task_light_webui.md

## Сессия - 06.04.2026

### Выполненные задачи:

#### Обновление Wi-Fi иконки и поддержка цветовых режимов

**Файл:** `src\light_webui_html.h`

**Изменения:**

1. **Заменена SVG иконка Wi-Fi**
   - Источник: `docs\images\icon_wi-fi_blue.svg`
   - Оптимизирована: убраны комментарии, metadata, Inkscape атрибуты
   - viewBox изменён с `0 0 24 24` на `0 0 32 32`
   - Элементы: эллипс (точка) + 2 дуги с правильными `stroke` атрибутами

2. **Обновлены CSS классы для Wi-Fi статусов**
   - `.status-icon.wifi` — базовый стиль (серый `#8E8E93`)
   - `.status-icon.wifi.sta` — STA режим (синий `#44a6f3`)
   - `.status-icon.wifi.ap` — AP режим (зеленый `rgb(31,177,65)`)
   - `.status-icon.wifi.error` — ошибка (серый `#8E8E93`)

3. **Обновлена JavaScript функция `loadStatus()`**
   - Вместо inline `style.fill` теперь использует CSS классы
   - STA: добавляет класс `sta`, убирает `ap`, `error`
   - AP: добавляет класс `ap`, убирает `sta`, `error`
   - Error: добавляет класс `error`, убирает `sta`, `ap`
   - Если статус неизвестен: убирает все классы

4. **Device Icon в status-bar**
   - `.device-icon-small` слева в status-bar
   - Загружается через `/api/device_info`

5. **Убраны MQTT и Telegram иконки** (пока не нужны)
   - Удалены SVG элементы из HTML
   - Удалены CSS классы для mqtt и telegram
   - Удалены JavaScript переменные `statusMqtt`, `statusTelegram`

6. **Исправлено выравнивание status-bar**
   - Новая структура: `.status-right` содержит `.status-icons` + `.settings-button`
   - Device Icon слева, иконки + settings справа
   - Wi-Fi иконка теперь рядом с кнопкой settings

#### Исправление ошибок (06.04.2026)

**Проблемы:**
- Wi-Fi иконка выглядела как одна точка (не было `stroke` у дуг)
- Telegram и MQTT иконки отображались на экране
- Контейнер иконок не был рядом с кнопкой settings

**Исправления:**
- Добавлены `stroke="#44a6f6"` к SVG элементам Wi-Fi
- Убраны MQTT и Telegram SVG элементы из HTML
- Реструктурирован status-bar: `.status-right` содержит иконки + кнопку settings

### Тестирование компиляции:

- [x] d1_mini_lite (ESP8266) — **SUCCESS** (RAM: 79.5%, Flash: 51.4%, 6.01s)
- [ ] nodemcuv3 (ESP8266) — ожидается
- [ ] esp32c3 — ожидается
- [ ] esp32-wroom-32u — ожидается
