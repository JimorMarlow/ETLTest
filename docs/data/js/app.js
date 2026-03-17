// Обновление статуса каждые 5 секунд
async function updateStatus() {
    try {
        const response = await fetch('/api/status');
        const data = await response.json();
        
        document.getElementById('wifi-status').textContent = data.wifi;
        document.getElementById('ip-address').textContent = data.ip;
        document.getElementById('uptime').textContent = data.uptime + 's';
    } catch (error) {
        console.error('Status update failed:', error);
    }
}

// Управление светом
async function controlLight(state) {
    try {
        const response = await fetch(`/api/light?state=${state}`);
        const result = await response.text();
        alert(`Light: ${result}`);
    } catch (error) {
        alert('Control failed: ' + error);
    }
}

// Установка яркости
async function setBrightness(value) {
    try {
        await fetch(`/api/brightness?value=${value}`);
        console.log('Brightness set to:', value);
    } catch (error) {
        console.error('Brightness set failed:', error);
    }
}

// Автоматическое обновление статуса
setInterval(updateStatus, 5000);
updateStatus(); // Первый вызов