#pragma once
/**
 * @file etl_wifi_setup_html.h
 * @brief HTML шаблон для страницы настройки WiFi
 * 
 * Этот файл содержит HTML шаблон, который используется классом wifi_setup
 * для отображения веб-интерфейса настройки WiFi.
 * 
 * Для использования необходимо:
 * 1. Загрузить файл wifi_setup.html в LittleFS
 * 2. Или использовать встроенную функцию get_wifi_setup_html()
 */

#include <Arduino.h>

namespace etl
{
    namespace wifi
    {
        /**
         * @brief Получить HTML страницу настройки WiFi
         * @param config Конфигурация устройства
         * @return HTML строка
         */
        inline String get_wifi_setup_html_content(
            const String& device_name = "ESP Device v1.0.0",
            const String& device_description = "Smart home device based on ESP8266/ESP32",
            const String& device_icon_svg = "")
        {
            String html = F(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>WiFi Setup</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: #FFFFFF; min-height: 100vh; color: #1C1C1E; padding: 16px;
        }
        .container { max-width: 480px; margin: 0 auto; }
        .header {
            display: flex; justify-content: space-between; align-items: center;
            padding-bottom: 20px; border-bottom: 1px solid #C6C6C8; margin-bottom: 20px;
        }
        .header-title { font-size: 17px; font-weight: 600; color: #1C1C1E; }
        .lang-switch { display: flex; gap: 8px; }
        .lang-btn {
            padding: 6px 12px; border: 1px solid #C6C6C8; border-radius: 6px;
            background: #FFFFFF; font-size: 14px; font-weight: 500; cursor: pointer;
            transition: all 0.2s;
        }
        .lang-btn.active { background: #007AFF; border-color: #007AFF; color: #FFFFFF; }
        .lang-btn:hover { border-color: #007AFF; }
        .device-info-container {
            background: #F2F2F7; border-radius: 12px; padding: 16px;
            margin-bottom: 20px; display: flex; align-items: center; gap: 16px;
        }
        .device-icon { width: 48px; height: 48px; flex-shrink: 0; }
        .device-icon svg { width: 100%; height: 100%; }
        .device-info-content { flex: 1; min-width: 0; }
        .device-name { font-size: 20px; font-weight: 700; color: #1C1C1E; margin-bottom: 4px; }
        .device-description { font-size: 15px; font-weight: 400; color: #8E8E93; line-height: 1.4; }
        .status-section {
            background: #F2F2F7; border-radius: 10px; padding: 16px;
            margin-bottom: 20px; display: flex; align-items: center; gap: 12px;
        }
        .status-indicator { width: 12px; height: 12px; border-radius: 50%; flex-shrink: 0; }
        .status-indicator.disconnected { background: #FF3B30; }
        .status-indicator.connecting { background: #FF9500; animation: pulse 1s infinite; }
        .status-indicator.connected { background: #34C759; }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }
        .status-text { flex: 1; font-size: 15px; color: #1C1C1E; }
        .status-details { font-size: 13px; color: #8E8E93; margin-top: 4px; }
        .refresh-btn {
            padding: 8px 16px; border: 1px solid #007AFF; border-radius: 8px;
            background: #FFFFFF; color: #007AFF; font-size: 14px; font-weight: 500;
            cursor: pointer; transition: all 0.2s;
        }
        .refresh-btn:hover { background: #007AFF; color: #FFFFFF; }
        .refresh-btn:disabled { opacity: 0.5; cursor: not-allowed; }
        .section-title { font-size: 17px; font-weight: 600; color: #1C1C1E; margin-bottom: 12px; }
        .networks-list {
            background: #F2F2F7; border-radius: 10px; overflow: hidden; margin-bottom: 20px;
        }
        .network-item {
            display: flex; align-items: center; padding: 14px 16px;
            border-bottom: 1px solid #C6C6C8; cursor: pointer; transition: background 0.2s;
        }
        .network-item:last-child { border-bottom: none; }
        .network-item:hover { background: #E5E5EA; }
        .network-item.selected { background: #007AFF; }
        .network-item.selected .network-name,
        .network-item.selected .network-signal,
        .network-item.selected .network-lock { color: #FFFFFF; }
        .network-icon { margin-right: 12px; font-size: 18px; }
        .network-info { flex: 1; }
        .network-name { font-size: 15px; font-weight: 500; color: #1C1C1E; margin-bottom: 4px; }
        .network-signal { font-size: 13px; color: #8E8E93; }
        .network-lock-wrapper { display: flex; align-items: center; gap: 4px; margin-left: 8px; }
        .network-lock { font-size: 14px; color: #8E8E93; line-height: 1; }
        .network-checkmark { font-size: 14px; color: #34C759; line-height: 1; }
        .password-section {
            background: #F2F2F7; border-radius: 10px; padding: 16px; margin-bottom: 20px;
        }
        .input-group { margin-bottom: 12px; }
        .input-group:last-child { margin-bottom: 0; }
        .input-label {
            display: block; font-size: 13px; font-weight: 500;
            color: #8E8E93; margin-bottom: 6px;
        }
        .input-wrapper { position: relative; display: flex; align-items: center; }
        .input-field {
            width: 100%; height: 44px; padding: 0 12px; border: 1px solid #C6C6C8;
            border-radius: 8px; font-size: 15px; color: #1C1C1E; background: #FFFFFF;
            transition: border-color 0.2s;
        }
        .input-field:focus { outline: none; border-color: #007AFF; }
        .input-field::placeholder { color: #C7C7CC; }
        .show-password-btn {
            position: absolute; right: 12px; padding: 6px 10px; border: none;
            background: transparent; color: #007AFF; font-size: 13px; font-weight: 500; cursor: pointer;
        }
        .selected-network-header { margin-bottom: 16px; }
        .selected-network-title { font-size: 17px; font-weight: 600; color: #1C1C1E; margin-bottom: 8px; }
        .selected-network-divider { height: 1px; background: #C6C6C8; margin-bottom: 16px; }
        .ap-settings-section {
            background: #F2F2F7; border-radius: 10px; padding: 16px; margin-bottom: 20px;
        }
        .ap-settings-title {
            font-size: 15px; font-weight: 600; color: #1C1C1E;
            margin-bottom: 12px; padding-bottom: 8px; border-bottom: 1px solid #C6C6C8;
        }
        .btn {
            width: 100%; height: 44px; border: none; border-radius: 10px;
            font-size: 15px; font-weight: 600; cursor: pointer;
            transition: all 0.2s; margin-bottom: 12px;
        }
        .btn:last-child { margin-bottom: 0; }
        .btn-primary { background: #007AFF; color: #FFFFFF; }
        .btn-primary:hover { background: #0056CC; }
        .btn-primary:disabled { opacity: 0.5; cursor: not-allowed; }
        .btn-secondary { background: #FFFFFF; color: #007AFF; border: 1px solid #007AFF; }
        .btn-secondary:hover { background: #007AFF; color: #FFFFFF; }
        .btn-danger { background: #FF3B30; color: #FFFFFF; }
        .btn-danger:hover { background: #D63026; }
        .spinner {
            display: inline-block; width: 16px; height: 16px;
            border: 2px solid #FFFFFF; border-top-color: transparent;
            border-radius: 50%; animation: spin 0.8s linear infinite; margin-right: 8px; vertical-align: middle;
        }
        @keyframes spin { to { transform: rotate(360deg); } }
        .modal-overlay {
            position: fixed; top: 0; left: 0; right: 0; bottom: 0;
            background: rgba(0, 0, 0, 0.5); display: none;
            justify-content: center; align-items: center; z-index: 1000;
        }
        .modal-overlay.active { display: flex; }
        .modal {
            background: #FFFFFF; border-radius: 14px;
            padding: 20px; max-width: 320px; width: 90%;
        }
        .modal-title { font-size: 17px; font-weight: 600; color: #1C1C1E; margin-bottom: 12px; }
        .modal-message { font-size: 15px; color: #1C1C1E; margin-bottom: 20px; line-height: 1.4; }
        .modal-buttons { display: flex; gap: 12px; }
        .modal-btn {
            flex: 1; height: 40px; border: none; border-radius: 8px;
            font-size: 15px; font-weight: 600; cursor: pointer;
        }
        .modal-btn-cancel { background: #F2F2F7; color: #1C1C1E; }
        .modal-btn-confirm { background: #FF3B30; color: #FFFFFF; }
        .hidden { display: none !important; }
        @media (max-width: 480px) {
            body { padding: 12px; }
            .device-info-container { padding: 12px; }
            .device-icon { width: 40px; height: 40px; }
            .device-name { font-size: 18px; }
            .device-description { font-size: 14px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-title" data-i18n="title">WiFi Setup</div>
            <div class="lang-switch">
                <button class="lang-btn active" data-lang="en">EN</button>
                <button class="lang-btn" data-lang="ru">RU</button>
            </div>
        </div>

        <div class="device-info-container" id="deviceInfoContainer">
            <div class="device-icon" id="deviceIcon"></div>
            <div class="device-info-content">
                <div class="device-name" id="deviceName">DEVICE_NAME_PLACEHOLDER</div>
                <div class="device-description" id="deviceDescription">DEVICE_DESC_PLACEHOLDER</div>
            </div>
        </div>

        <div class="status-section">
            <div class="status-indicator disconnected" id="statusIndicator"></div>
            <div class="status-text">
                <div id="statusText" data-i18n="status_disconnected">Disconnected</div>
                <div class="status-details" id="statusDetails"></div>
            </div>
            <button class="refresh-btn" id="refreshBtn" data-i18n="refresh_btn">Refresh</button>
        </div>

        <div class="section-title" data-i18n="select_network">Select Network</div>
        <div class="networks-list" id="networksList">
            <div class="network-item" style="justify-content: center; color: #8E8E93;">
                <span data-i18n="scan_networks">Scan networks to see available networks</span>
            </div>
        </div>

        <div class="password-section hidden" id="passwordSection">
            <div class="selected-network-header">
                <div class="selected-network-title" id="selectedNetworkTitle"></div>
                <div class="selected-network-divider"></div>
            </div>
            <div class="input-group" id="passwordInputGroup">
                <label class="input-label" data-i18n="password_label">Password</label>
                <div class="input-wrapper">
                    <input type="password" class="input-field" id="passwordInput" data-i18n-placeholder="password_placeholder" placeholder="Enter password">
                    <button type="button" class="show-password-btn" id="showPasswordBtn" data-i18n="show_password">Show</button>
                </div>
            </div>
            <button class="btn btn-primary" id="connectBtn" style="margin-top: 12px;" data-i18n="join_btn">Join</button>
        </div>

        <div class="ap-settings-section">
            <div class="ap-settings-title" data-i18n="ap_settings_title">Access Point Settings</div>
            <div class="input-group">
                <label class="input-label" data-i18n="ap_ssid_label">AP SSID</label>
                <input type="text" class="input-field" id="apSsidInput" value="ESP_Device_AP">
            </div>
            <div class="input-group">
                <label class="input-label" data-i18n="ap_password_label">AP Password</label>
                <div class="input-wrapper">
                    <input type="password" class="input-field" id="apPasswordInput" value="password123">
                    <button type="button" class="show-password-btn" id="showApPasswordBtn" data-i18n="show_password">Show</button>
                </div>
            </div>
            <button class="btn btn-secondary" id="applyApSettingsBtn" style="margin-top: 12px;" data-i18n="apply_ap_settings_btn">Apply AP Settings</button>
        </div>

        <button class="btn btn-primary" id="saveRebootBtn" data-i18n="save_reboot_btn">Save & Reboot</button>
        <button class="btn btn-danger" id="factoryResetBtn" data-i18n="factory_reset_btn">Factory Reset</button>
    </div>

    <div class="modal-overlay" id="modalOverlay">
        <div class="modal">
            <div class="modal-title" id="modalTitle">Confirm</div>
            <div class="modal-message" id="modalMessage">Are you sure?</div>
            <div class="modal-buttons">
                <button class="modal-btn modal-btn-cancel" id="modalCancelBtn" data-i18n="cancel">Cancel</button>
                <button class="modal-btn modal-btn-confirm" id="modalConfirmBtn" data-i18n="confirm">Confirm</button>
            </div>
        </div>
    </div>

    <script>
        const translations = {
            en: {
                title: 'WiFi Setup', device_name: 'ESP Device v1.0.0',
                device_description: 'Smart home device based on ESP8266/ESP32',
                refresh_btn: 'Refresh', select_network: 'Select Network',
                scan_networks: 'Scan networks to see available networks',
                selected_network: 'Selected Network', password_label: 'Password',
                password_placeholder: 'Enter password', show_password: 'Show',
                hide_password: 'Hide', join_btn: 'Join', disconnect_btn: 'Disconnect',
                connect_btn: 'Connect', connecting: 'Connecting...',
                ap_settings_title: 'Access Point Settings', ap_ssid_label: 'AP SSID',
                ap_password_label: 'AP Password', apply_ap_settings_btn: 'Apply AP Settings',
                save_reboot_btn: 'Save & Reboot', factory_reset_btn: 'Factory Reset',
                status_disconnected: 'Disconnected', status_connecting: 'Connecting...',
                status_connected: 'Connected', confirm_reset: 'Are you sure you want to reset to factory settings?',
                cancel: 'Cancel', confirm: 'Confirm', success_saved: 'Settings saved! Rebooting...',
                success_ap_applied: 'AP settings applied! Reconnecting...',
                error_connection: 'Connection failed. Check password.',
                error_no_network: 'Please select a network first.',
                no_networks: 'No networks found', signal_excellent: 'Excellent',
                signal_good: 'Good', signal_weak: 'Weak', signal_very_weak: 'Very Weak'
            },
            ru: {
                title: 'Настройка WiFi', device_name: 'ESP Device v1.0.0',
                device_description: 'Устройство для умного дома на базе ESP8266/ESP32',
                refresh_btn: 'Обновить', select_network: 'Выберите сеть',
                scan_networks: 'Нажмите "Обновить" для поиска сетей',
                selected_network: 'Выбранная сеть', password_label: 'Пароль',
                password_placeholder: 'Введите пароль', show_password: 'Show',
                hide_password: 'Hide', join_btn: 'Join', disconnect_btn: 'Disconnect',
                connect_btn: 'Подключиться', connecting: 'Подключение...',
                ap_settings_title: 'Настройки точки доступа', ap_ssid_label: 'SSID точки доступа',
                ap_password_label: 'Пароль точки доступа', apply_ap_settings_btn: 'Применить настройки AP',
                save_reboot_btn: 'Запомнить и перезапустить', factory_reset_btn: 'Сброс настроек',
                status_disconnected: 'Не подключено', status_connecting: 'Подключение...',
                status_connected: 'Подключено', confirm_reset: 'Вы уверены, что хотите сбросить настройки к заводским?',
                cancel: 'Отмена', confirm: 'Подтвердить', success_saved: 'Настройки сохранены! Перезагрузка...',
                success_ap_applied: 'Настройки AP применены! Переподключение...',
                error_connection: 'Ошибка подключения. Проверьте пароль.',
                error_no_network: 'Пожалуйста, выберите сеть.',
                no_networks: 'Сети не найдены', signal_excellent: 'Отличный',
                signal_good: 'Хороший', signal_weak: 'Слабый', signal_very_weak: 'Очень слабый'
            }
        };

        const DEFAULT_DEVICE_ICON = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512"><path d="M256 0C114.6 0 0 114.6 0 256s114.6 512 256 512 256-114.6 256-256S397.4 0 256 0zm0 480c-123.7 0-224-100.3-224-224S132.3 32 256 32s224 100.3 224 224-100.3 224-224 224z" fill="#007AFF"/><path d="M256 128c-52.9 0-96 43.1-96 96v96c0 33.1 26.9 60 60 60h72c33.1 0 60-26.9 60-60v-96c0-52.9-43.1-96-96-96zm64 192c0 15.4-12.6 28-28 28h-72c-15.4 0-28-12.6-28-28v-96c0-35.2 28.8-64 64-64s64 28.8 64 64v96z" fill="#007AFF"/><circle cx="256" cy="256" r="48" fill="#007AFF"/></svg>`;

        let currentLang = localStorage.getItem('wifiSetupLang') || 'en';
        let selectedNetwork = null;
        let networks = [];
        let isConnected = false;

        window.deviceConfig = {
            version: 'DEVICE_NAME_PLACEHOLDER',
            description: 'DEVICE_DESC_PLACEHOLDER',
            iconSvg: 'DEVICE_ICON_PLACEHOLDER' || null
        };

        const deviceName = document.getElementById('deviceName');
        const deviceDescription = document.getElementById('deviceDescription');
        const deviceIcon = document.getElementById('deviceIcon');
        const statusIndicator = document.getElementById('statusIndicator');
        const statusText = document.getElementById('statusText');
        const statusDetails = document.getElementById('statusDetails');
        const refreshBtn = document.getElementById('refreshBtn');
        const networksList = document.getElementById('networksList');
        const passwordSection = document.getElementById('passwordSection');
        const selectedNetworkTitle = document.getElementById('selectedNetworkTitle');
        const passwordInputGroup = document.getElementById('passwordInputGroup');
        const passwordInput = document.getElementById('passwordInput');
        const showPasswordBtn = document.getElementById('showPasswordBtn');
        const connectBtn = document.getElementById('connectBtn');
        const apSsidInput = document.getElementById('apSsidInput');
        const apPasswordInput = document.getElementById('apPasswordInput');
        const showApPasswordBtn = document.getElementById('showApPasswordBtn');
        const applyApSettingsBtn = document.getElementById('applyApSettingsBtn');
        const saveRebootBtn = document.getElementById('saveRebootBtn');
        const factoryResetBtn = document.getElementById('factoryResetBtn');
        const modalOverlay = document.getElementById('modalOverlay');
        const modalTitle = document.getElementById('modalTitle');
        const modalMessage = document.getElementById('modalMessage');
        const modalCancelBtn = document.getElementById('modalCancelBtn');
        const modalConfirmBtn = document.getElementById('modalConfirmBtn');

        function init() {
            applyDeviceConfig();
            setLanguage(currentLang);
            updateStatusUI();
            scanNetworks();
        }

        function applyDeviceConfig() {
            const config = window.deviceConfig || {};
            if (config.version) deviceName.textContent = config.version;
            if (config.description) deviceDescription.textContent = config.description;
            if (config.iconSvg && config.iconSvg.trim()) {
                deviceIcon.innerHTML = config.iconSvg;
            } else {
                deviceIcon.innerHTML = DEFAULT_DEVICE_ICON;
            }
        }

        function setLanguage(lang) {
            currentLang = lang;
            localStorage.setItem('wifiSetupLang', lang);
            document.querySelectorAll('[data-i18n]').forEach(el => {
                const key = el.getAttribute('data-i18n');
                if (translations[lang][key]) el.textContent = translations[lang][key];
            });
            document.querySelectorAll('[data-i18n-placeholder]').forEach(el => {
                const key = el.getAttribute('data-i18n-placeholder');
                if (translations[lang][key]) el.placeholder = translations[lang][key];
            });
            deviceName.textContent = translations[lang].device_name;
            deviceDescription.textContent = translations[lang].device_description;
            const config = window.deviceConfig || {};
            if (config.version) deviceName.textContent = config.version;
            if (config.description) deviceDescription.textContent = config.description;
            document.querySelectorAll('.lang-btn').forEach(btn => {
                btn.classList.toggle('active', btn.getAttribute('data-lang') === lang);
            });
            updateConnectButtonText();
        }

        function updateStatusUI() {
            statusIndicator.className = 'status-indicator disconnected';
            statusText.textContent = translations[currentLang].status_disconnected;
            statusDetails.textContent = '';
        }

        function setStatus(status, details = '') {
            statusIndicator.className = `status-indicator ${status}`;
            statusText.textContent = translations[currentLang][`status_${status}`] || status;
            statusDetails.textContent = details;
        }

        async function scanNetworks() {
            refreshBtn.disabled = true;
            networksList.innerHTML = '<div class="network-item" style="justify-content: center;"><span class="spinner"></span></div>';
            try {
                const response = await fetch('/api/scan');
                const data = await response.json();
                networks = data.networks || [];
                renderNetworks();
            } catch (error) {
                networksList.innerHTML = `<div class="network-item" style="justify-content: center; color: #FF3B30;">Error: ${error.message}</div>`;
            }
            refreshBtn.disabled = false;
        }

        function renderNetworks() {
            if (networks.length === 0) {
                networksList.innerHTML = `<div class="network-item" style="justify-content: center; color: #8E8E93;">${translations[currentLang].no_networks}</div>`;
                return;
            }
            networksList.innerHTML = networks.map((network, index) => {
                const signalStrength = getSignalStrength(network.rssi);
                const signalText = translations[currentLang][`signal_${signalStrength}`] || signalStrength;
                const lockIcon = network.encryption === 'none' ? '🔓' : '🔒';
                const checkmark = network.connected ? '✅' : '';
                return `
                    <div class="network-item ${selectedNetwork === index ? 'selected' : ''}" data-index="${index}">
                        <span class="network-icon">📶</span>
                        <div class="network-info">
                            <div class="network-name">${escapeHtml(network.ssid)}</div>
                            <div class="network-signal">${signalText} • ${network.encryption === 'none' ? 'Open' : network.encryption}</div>
                        </div>
                        <div class="network-lock-wrapper">
                            ${checkmark ? `<span class="network-checkmark">${checkmark}</span>` : ''}
                            <span class="network-lock">${lockIcon}</span>
                        </div>
                    </div>
                `;
            }).join('');
            networksList.querySelectorAll('.network-item').forEach(item => {
                item.addEventListener('click', () => selectNetwork(parseInt(item.dataset.index)));
            });
        }

        function getSignalStrength(rssi) {
            if (rssi >= -50) return 'excellent';
            if (rssi >= -60) return 'good';
            if (rssi >= -70) return 'weak';
            return 'very_weak';
        }

        function selectNetwork(index) {
            if (selectedNetwork !== null && networks[selectedNetwork]) networks[selectedNetwork].connected = false;
            if (selectedNetwork === index && networks[index].connected) {
                selectedNetwork = index;
                renderNetworks();
                showPasswordSection(networks[index].ssid, true);
                return;
            }
            selectedNetwork = index;
            renderNetworks();
            showPasswordSection(networks[index].ssid, false);
        }

        function showPasswordSection(ssid, isDisconnect = false) {
            passwordSection.classList.remove('hidden');
            selectedNetworkTitle.textContent = ssid;
            if (isDisconnect) {
                passwordInputGroup.classList.add('hidden');
                connectBtn.textContent = translations[currentLang].disconnect_btn;
                connectBtn.className = 'btn btn-secondary';
            } else {
                passwordInputGroup.classList.remove('hidden');
                connectBtn.textContent = translations[currentLang].join_btn;
                connectBtn.className = 'btn btn-primary';
            }
        }

        function updateConnectButtonText() {
            if (selectedNetwork !== null && passwordSection.classList.contains('hidden') === false) {
                if (isConnected) {
                    connectBtn.textContent = translations[currentLang].disconnect_btn;
                    connectBtn.className = 'btn btn-secondary';
                } else {
                    connectBtn.textContent = translations[currentLang].join_btn;
                    connectBtn.className = 'btn btn-primary';
                }
            }
        }

        async function connectToNetwork() {
            if (selectedNetwork === null) {
                alert(translations[currentLang].error_no_network);
                return;
            }
            const network = networks[selectedNetwork];
            if (network.connected || isConnected) {
                network.connected = false;
                isConnected = false;
                setStatus('disconnected');
                passwordInputGroup.classList.remove('hidden');
                connectBtn.textContent = translations[currentLang].join_btn;
                connectBtn.className = 'btn btn-primary';
                renderNetworks();
                return;
            }
            const password = passwordInput.value;
            if (password.length < 8) {
                alert('Password must be at least 8 characters');
                return;
            }
            connectBtn.disabled = true;
            connectBtn.innerHTML = '<span class="spinner"></span>' + translations[currentLang].connecting;
            setStatus('connecting');
            try {
                const response = await fetch('/api/connect', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ssid: network.ssid, password })
                });
                const data = await response.json();
                if (data.success) {
                    network.connected = true;
                    isConnected = true;
                    setStatus('connected', `${network.ssid} • ${data.ip}`);
                    renderNetworks();
                    passwordInputGroup.classList.add('hidden');
                    connectBtn.textContent = translations[currentLang].disconnect_btn;
                    connectBtn.className = 'btn btn-secondary';
                } else {
                    alert(translations[currentLang].error_connection);
                    setStatus('disconnected');
                }
            } catch (error) {
                alert(translations[currentLang].error_connection);
                setStatus('disconnected');
            }
            connectBtn.disabled = false;
            connectBtn.textContent = isConnected ? translations[currentLang].disconnect_btn : translations[currentLang].join_btn;
        }

        function togglePassword(input, btn) {
            const isPassword = input.type === 'password';
            input.type = isPassword ? 'text' : 'password';
            btn.textContent = isPassword ? translations[currentLang].hide_password : translations[currentLang].show_password;
        }

        async function saveAndReboot() {
            showModal(translations[currentLang].success_saved, async () => {
                await fetch('/api/save', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ ssid: networks[selectedNetwork]?.ssid, password: passwordInput.value })
                });
                document.body.innerHTML = `
                    <div class="container" style="text-align: center; padding-top: 100px;">
                        <div class="spinner" style="width: 48px; height: 48px; border-width: 4px; border-color: #007AFF;"></div>
                        <p style="margin-top: 20px; font-size: 17px; color: #1C1C1E;">Rebooting...</p>
                        <p style="margin-top: 10px; font-size: 15px; color: #8E8E93;">Redirecting in 10 seconds...</p>
                    </div>
                `;
                await new Promise(resolve => setTimeout(resolve, 10000));
                window.location.reload();
            });
        }

        function factoryReset() {
            showModal(translations[currentLang].confirm_reset, async () => {
                await fetch('/api/reset', { method: 'POST' });
                document.body.innerHTML = `
                    <div class="container" style="text-align: center; padding-top: 100px;">
                        <div class="spinner" style="width: 48px; height: 48px; border-width: 4px; border-color: #FF3B30;"></div>
                        <p style="margin-top: 20px; font-size: 17px; color: #1C1C1E;">Resetting...</p>
                    </div>
                `;
                await new Promise(resolve => setTimeout(resolve, 2000));
                window.location.reload();
            });
        }

        async function applyApSettings() {
            const apSsid = apSsidInput.value;
            const apPassword = apPasswordInput.value;
            if (apSsid.length < 1) { alert('AP SSID cannot be empty'); return; }
            if (apPassword.length < 8) { alert('AP Password must be at least 8 characters'); return; }
            await fetch('/api/ap_settings', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ap_ssid: apSsid, ap_password: apPassword })
            });
            alert(translations[currentLang].success_ap_applied);
        }

        function showModal(message, onConfirm) {
            modalMessage.textContent = message;
            modalOverlay.classList.add('active');
            const handleConfirm = () => {
                modalOverlay.classList.remove('active');
                modalConfirmBtn.removeEventListener('click', handleConfirm);
                modalCancelBtn.removeEventListener('click', handleCancel);
                onConfirm();
            };
            const handleCancel = () => {
                modalOverlay.classList.remove('active');
                modalConfirmBtn.removeEventListener('click', handleConfirm);
                modalCancelBtn.removeEventListener('click', handleCancel);
            };
            modalConfirmBtn.addEventListener('click', handleConfirm);
            modalCancelBtn.addEventListener('click', handleCancel);
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }

        refreshBtn.addEventListener('click', scanNetworks);
        connectBtn.addEventListener('click', connectToNetwork);
        showPasswordBtn.addEventListener('click', () => togglePassword(passwordInput, showPasswordBtn));
        showApPasswordBtn.addEventListener('click', () => togglePassword(apPasswordInput, showApPasswordBtn));
        saveRebootBtn.addEventListener('click', saveAndReboot);
        factoryResetBtn.addEventListener('click', factoryReset);
        applyApSettingsBtn.addEventListener('click', applyApSettings);
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.addEventListener('click', () => setLanguage(btn.dataset.lang));
        });
        document.addEventListener('click', (e) => {
            if (!passwordSection.contains(e.target) && !e.target.closest('.network-item')) {
                if (!isConnected) passwordSection.classList.add('hidden');
            }
        });
        init();
    </script>
</body>
</html>
)rawliteral");

            // Замена плейсхолдеров
            html.replace("DEVICE_NAME_PLACEHOLDER", device_name);
            html.replace("DEVICE_DESC_PLACEHOLDER", device_description);
            
            if (device_icon_svg.length() > 0) {
                html.replace("DEVICE_ICON_PLACEHOLDER", device_icon_svg);
            } else {
                html.replace("DEVICE_ICON_PLACEHOLDER", "");
            }

            return html;
        }

        /**
         * @brief Получить HTML страницу настройки WiFi (обёртка для совместимости)
         * @deprecated Используйте get_wifi_setup_html_content()
         */
        inline String get_wifi_setup_html()
        {
            return get_wifi_setup_html_content();
        }

    } // namespace wifi
} // namespace etl
