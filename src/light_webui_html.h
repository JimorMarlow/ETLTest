#pragma once
/**
 * @file light_webui_html.h
 * @brief HTML макет для страницы управления светодиодной лампой
 *
 * Оптимизированная версия из docs\web-wifi\qwen-webui.002.html
 * Хранится в PROGMEM (flash) чтобы НЕ занимать RAM.
 * Отправляется через send_P() — без RAM-аллокаций источника.
 */

// SVG иконка устройства — в RAM (маленькая, ~4KB, не критично)
const char* LIGHT_DEVICE_ICON_SVG = R"rawliteral(<svg xmlns="http://www.w3.org/2000/svg" width="64mm" height="64mm" viewBox="0 0 64 64"><style>.lamp-glow{animation:glow 5s ease-in-out infinite}@keyframes glow{0%,100%{fill-opacity:1;stroke-opacity:1}50%{fill-opacity:0.7;stroke-opacity:0.7}}</style><rect x="14.45" y="2.39" width="36.71" height="6.44" ry="0.18" fill="#a2d6fe" stroke="#20446e" stroke-width="1.465"/><path class="lamp-glow" fill="#ffffff" stroke="#20446e" stroke-width="1.465" d="M 26.49 9.5 A 6.35 6.16 0 0 0 32.84 15.66 A 6.35 6.16 0 0 0 39.19 9.5"/><path fill="none" stroke="#a2d6fe" stroke-width="0.9" stroke-linecap="round" d="M 17.82 34.35 A 24.41 23.89 0 0 1 8.25 15.38"/><path fill="none" stroke="#a2d6fe" stroke-width="0.9" stroke-linecap="round" d="M 57.22 15.13 A 24.41 23.89 0 0 1 50.98 31.09"/><path fill="none" stroke="#a2d6fe" stroke-width="0.965" stroke-linecap="round" d="M 19.98 27.3 A 16.93 16.47 0 0 1 15.6 16.25"/><path fill="none" stroke="#a2d6fe" stroke-width="0.979" stroke-linecap="round" d="M 50.33 15.89 A 25.41 16.29 0 0 1 47.26 23.65"/><path fill="none" stroke="#a2d6fe" stroke-width="0.98" stroke-linecap="round" d="M 25.52 22.96 A 10.11 9.1 0 0 1 22.59 16.55"/><path fill="none" stroke="#a2d6fe" stroke-width="0.98" stroke-linecap="round" d="M 42.87 16.29 A 10.11 9.1 0 0 1 37.59 24.28"/><path fill="none" stroke="#1e436d" stroke-width="1.565" stroke-linecap="round" stroke-linejoin="round" d="M 43.52 58.03 L 40.14 54.58 L 36.43 54.32 L 21.47 40.08 L 21.47 37.67 L 22.97 36.43 L 24.66 36.82"/><path fill="none" stroke="#1e436d" stroke-width="1.565" stroke-linecap="round" stroke-linejoin="round" d="M 30.38 42.81 L 21.34 33.57 L 21.34 31.94 L 22.45 30.84 L 24.27 30.51"/><path fill="none" stroke="#1e436d" stroke-width="1.565" stroke-linecap="round" stroke-linejoin="round" d="M 33.7 39.56 L 23.29 29.08 L 23.29 27.39 L 24.85 25.89 L 27.39 26.09 L 36.82 35.85"/><path fill="none" stroke="#1e436d" stroke-width="1.565" stroke-linecap="round" stroke-linejoin="round" d="M 28.43 27.32 L 28.43 25.05 L 29.54 23.94 L 31.55 23.94 L 43.26 35.33"/><path fill="none" stroke="#1e436d" stroke-width="1.565" stroke-linecap="round" stroke-linejoin="round" d="M 40.34 44.18 L 40.47 40.67 L 43.46 36.5 L 43.46 32.72 L 42.35 31.68 L 42.35 26.87 L 43.52 25.63 L 45.47 25.7 L 48.27 31.88 L 50.48 37.86 L 51.26 40.07 L 51.39 43.78 L 54.45 46.84"/><path fill="#b6b6b6" fill-opacity="0.496063" stroke="none" stroke-width="1.12834" stroke-linecap="round" stroke-linejoin="round" d="m 165.60607,216.06102 c -1.07157,-1.21134 -4.33132,-4.5828 -7.24389,-7.49215 l -5.29557,-5.28971 -6.86346,-0.48712 c -5.89809,-0.41861 -6.97196,-0.57009 -7.63481,-1.07699 -0.42424,-0.32442 -12.79091,-12.04222 -27.48151,-26.03956 L 84.376665,150.22578 v -3.21394 -3.21394 l 1.602363,-1.36894 1.602364,-1.36894 1.619836,0.38698 c 1.568316,0.37467 1.844084,0.61486 8.670407,7.55212 3.877815,3.94084 8.801035,8.96809 10.940495,11.17168 4.00969,4.12989 5.24003,4.98856 6.66503,4.65166 1.74088,-0.41159 2.84822,-2.34938 2.31057,-4.04339 -0.11919,-0.37552 -5.0215,-5.58961 -10.89403,-11.58687 -5.87253,-5.99727 -10.848756,-11.22912 -11.058282,-11.62634 -0.209525,-0.39722 -0.705784,-0.89365 -1.102797,-1.10318 C 94.335607,136.25316 91.70822,133.7 88.893982,130.789 l -5.116796,-5.29272 v -1.76368 c 0,-1.68394 0.05865,-1.82124 1.297153,-3.03674 1.216723,-1.19413 1.472271,-1.30295 4.121418,-1.75502 l 2.824265,-0.48196 16.906018,16.96554 c 12.12922,12.17193 17.14754,17.01859 17.76081,17.15328 1.1496,0.2525 2.98667,-0.66997 3.5036,-1.7593 0.98418,-2.07401 1.50631,-1.46967 -19.45792,-22.5214 L 91.270673,108.75393 v -1.93121 -1.93122 l 1.905918,-1.88898 1.905918,-1.88897 3.485001,0.21461 3.485,0.21461 1.27829,1.47062 c 0.70305,0.80885 2.26069,2.43152 3.46141,3.60595 1.20072,1.17443 8.60328,8.78694 16.45012,16.91669 15.34254,15.89568 15.07675,15.66726 17.20475,14.78582 1.64532,-0.68152 2.28411,-2.98351 1.25279,-4.51464 -0.28742,-0.4267 -7.40159,-7.88953 -15.80929,-16.58406 l -15.28671,-15.80825 -0.0852,-2.60169 -0.0852,-2.601694 1.30818,-1.283876 1.30817,-1.283877 2.56262,0.08735 2.56263,0.08735 21.39254,20.816187 c 19.24914,18.73053 21.39111,20.90786 21.37823,21.73111 -0.0109,0.69655 -1.30981,2.71673 -5.44221,8.46413 -2.98534,4.15207 -5.49991,7.92878 -5.58793,8.39271 -0.088,0.46392 -0.24828,3.8941 -0.35613,7.62262 -0.16771,5.79778 -0.12895,6.90898 0.26773,7.67608 0.95846,1.85345 3.81741,2.02551 5.09326,0.30652 0.55141,-0.74292 0.63748,-1.48069 0.86778,-7.43788 l 0.25565,-6.61314 5.34192,-7.42171 c 2.93806,-4.08194 5.49418,-7.70621 5.68028,-8.05394 0.53626,-1.002 0.46244,-16.05002 -0.0839,-17.09726 -0.23223,-0.44517 -1.17641,-1.45787 -2.09818,-2.25044 l -1.67594,-1.44103 0.007,-7.85089 0.007,-7.85089 1.39359,-1.46105 1.39361,-1.461056 1.80853,0.189026 c 0.99469,0.10396 1.904,0.28935 2.0207,0.41197 0.1167,0.12261 2.32273,4.87639 4.9023,10.56395 4.41696,9.7387 9.05255,21.57683 14.42161,36.82913 1.55529,4.41822 1.62292,4.73778 1.64928,7.79322 0.0151,1.75236 0.11896,4.9748 0.23075,7.16098 l 0.20327,3.97489 6.43845,6.4443 c 3.54115,3.54437 6.68987,6.52425 6.99716,6.62197 0.45249,0.1439 -0.5476,1.29724 -5.26039,6.06649 -3.20051,3.23884 -11.67978,11.8356 -18.84282,19.1039 l -13.02372,13.21509 z" transform="scale(0.26458333)"/></svg>)rawliteral";

// Основной HTML макет — в PROGMEM (flash), НЕ в RAM!
const char PROGMEM LIGHT_WEBUI_HTML[] = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no,viewport-fit=cover">
<title>LED Lamp Control</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,'Helvetica Neue',Arial,sans-serif;background:#FFFFFF;min-height:100vh;min-height:100dvh;color:#1C1C1E;padding:env(safe-area-inset-top,16px) env(safe-area-inset-right,16px) env(safe-area-inset-bottom,16px) env(safe-area-inset-left,16px);display:flex;flex-direction:column}
body.dark-theme{background:#1C1C1E;color:#FFFFFF}
.container{max-width:480px;margin:0 auto;flex:1;display:flex;flex-direction:column;width:100%}
.status-bar{display:flex;justify-content:space-between;align-items:center;padding:12px 16px;margin-bottom:8px;background:#F2F2F7;border-radius:12px}
body.dark-theme .status-bar{background:#2C2C2E}
.device-icon-small{width:40px;height:40px;flex-shrink:0}
.device-icon-small svg{width:100%;height:100%}
.status-right{display:flex;align-items:center;gap:2px}
.connection-states{display:flex;align-items:center;gap:2px;padding:4px 8px;min-height:32px}
.connection-icon{font-size:20px;line-height:1;flex-shrink:0}
.connection-icon.wifi.sta{color:#44a6f3}
.connection-icon.wifi.ap{color:rgb(31,177,65)}
.header{display:flex;flex-direction:column;align-items:center;justify-content:center;padding-bottom:8px;margin-bottom:8px;text-align:center}
.device-info-container{display:flex;flex-direction:column;align-items:center;gap:0}
.device-info-content{display:flex;flex-direction:column;align-items:center}
.device-name{font-size:20px;font-weight:700;color:#1C1C1E;margin-bottom:4px}
body.dark-theme .device-name{color:#FFFFFF}
.device-description{font-size:15px;font-weight:400;color:#8E8E93;line-height:1.4}
body.dark-theme .device-description{color:#98989D}
.settings-button{width:44px;height:44px;border:none;border-radius:10px;background:#FFFFFF;color:#8E8E93;cursor:pointer;display:flex;align-items:center;justify-content:center;flex-shrink:0;transition:all 0.2s}
body.dark-theme .settings-button{background:#3A3A3C;color:#98989D}
.settings-button:hover{background:#007AFF;color:#FFFFFF}
body.dark-theme .settings-button:hover{background:#0A84FF;color:#FFFFFF}
.settings-button:active{transform:scale(0.95)}
.settings-button svg{width:24px;height:24px;fill:currentColor;transition:fill 0.2s}
.power-section{flex:1;display:flex;align-items:center;justify-content:center;padding:0;margin-top:-20px}
.power-button-wrapper{position:relative;width:160px;height:160px}
.power-button{position:relative;width:100%;height:100%;border:none;background:transparent;cursor:pointer;display:flex;align-items:center;justify-content:center;outline:none;-webkit-tap-highlight-color:transparent}
.power-button:focus{outline:none;box-shadow:none}
.power-ring{position:absolute;width:100%;height:100%;border-radius:50%;border:4px solid #E5E5EA;transition:border-color 0.3s,box-shadow 0.3s}
body.dark-theme .power-ring{border-color:#38383A}
.power-button.on .power-ring{border-color:#007AFF;box-shadow:0 0 30px rgba(0,122,255,0.4)}
.power-symbol{width:96px;height:96px;border-radius:50%;background:#F2F2F7;display:flex;align-items:center;justify-content:center;transition:background 0.3s,box-shadow 0.3s}
body.dark-theme .power-symbol{background:#2C2C2E}
.power-button.on .power-symbol{background:#007AFF;box-shadow:0 0 20px rgba(0,122,255,0.5)}
.power-symbol svg{width:48px;height:48px;fill:#8E8E93;transition:fill 0.3s}
body.dark-theme .power-symbol svg{fill:#98989D}
.power-button.on .power-symbol svg{fill:#FFFFFF;filter:drop-shadow(0 0 5px rgba(255,255,255,0.5))}
.brightness-section{background:#F2F2F7;border-radius:12px;padding:16px;margin-top:auto}
body.dark-theme .brightness-section{background:#2C2C2E}
.brightness-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:16px}
.brightness-label{font-size:15px;font-weight:600;color:#1C1C1E}
body.dark-theme .brightness-label{color:#FFFFFF}
.brightness-value{font-size:15px;font-weight:600;color:#007AFF}
body.dark-theme .brightness-value{color:#0A84FF}
.brightness-controls{display:flex;align-items:center;gap:12px}
.brightness-btn{width:44px;height:44px;border:none;border-radius:10px;background:#FFFFFF;color:#007AFF;font-size:20px;font-weight:600;cursor:pointer;transition:all 0.2s;display:flex;align-items:center;justify-content:center;flex-shrink:0}
body.dark-theme .brightness-btn{background:#3A3A3C;color:#0A84FF}
.brightness-btn:hover{background:#007AFF;color:#FFFFFF}
body.dark-theme .brightness-btn:hover{background:#0A84FF;color:#FFFFFF}
.brightness-btn:active{transform:scale(0.95)}
.brightness-slider-container{flex:1;position:relative;display:flex;align-items:center;height:44px;margin:0 4px}
.brightness-track{position:absolute;left:0;right:0;top:50%;transform:translateY(-50%);height:12px;background:#E5E5EA;border-radius:6px;z-index:0}
body.dark-theme .brightness-track{background:#3A3A3C}
.brightness-fill{position:absolute;left:0;top:50%;transform:translateY(-50%);height:12px;background:#007AFF;border-radius:6px;pointer-events:none;z-index:1}
body.dark-theme .brightness-fill{background:#0A84FF}
.brightness-slider{-webkit-appearance:none;appearance:none;width:100%;height:44px;background:transparent;cursor:pointer;position:relative;z-index:2}
.brightness-slider:focus{outline:none}
.brightness-slider::-webkit-slider-runnable-track{width:100%;height:12px;border-radius:6px;background:transparent}
body.dark-theme .brightness-slider::-webkit-slider-runnable-track{background:transparent}
.brightness-slider::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:28px;height:28px;border-radius:50%;background:#FFFFFF;box-shadow:0 2px 8px rgba(0,0,0,0.2);cursor:pointer;margin-top:-8px;transition:transform 0.1s}
body.dark-theme .brightness-slider::-webkit-slider-thumb{background:#3A3A3C;box-shadow:0 2px 8px rgba(0,0,0,0.4)}
.brightness-slider:active::-webkit-slider-thumb{transform:scale(1.1)}
.brightness-slider::-moz-range-track{width:100%;height:12px;border-radius:6px;background:transparent;border:none}
body.dark-theme .brightness-slider::-moz-range-track{background:transparent}
.brightness-slider::-moz-range-thumb{width:28px;height:28px;border-radius:50%;background:#FFFFFF;box-shadow:0 2px 8px rgba(0,0,0,0.2);cursor:pointer;border:none}
body.dark-theme .brightness-slider::-moz-range-thumb{background:#3A3A3C;box-shadow:0 2px 8px rgba(0,0,0,0.4)}
@media (max-width:480px){body{padding:env(safe-area-inset-top,12px) env(safe-area-inset-right,12px) env(safe-area-inset-bottom,12px) env(safe-area-inset-left,12px)}.container{max-width:100%}.power-button-wrapper{width:140px;height:140px}.power-symbol{width:84px;height:84px}.power-symbol svg{width:42px;height:42px}.device-name{font-size:18px}.device-description{font-size:14px}.brightness-section{margin-bottom:20px}}
body.large-font .device-name{font-size:24px}
body.large-font .device-description{font-size:18px}
body.large-font .brightness-label{font-size:18px}
body.large-font .brightness-value{font-size:18px}
body.large-font .brightness-value.bold-val{font-weight:700}
@keyframes spin{from{-webkit-transform:rotate(0);transform:rotate(0)}to{-webkit-transform:rotate(360deg);transform:rotate(360deg)}}
.settings-spinner{-webkit-transform-origin:50% 50%;transform-origin:50% 50%;-webkit-transform:translateZ(0);transform:translateZ(0)}
</style>
</head>
<body>
<div class="container">
<div class="status-bar">
<div class="device-icon-small" id="deviceIconSmall"></div>
<div class="status-right">
<div class="connection-states" id="connectionStates">
<span class="connection-icon wifi" id="wifiIcon">&#x1F4F6;</span>
</div>
<button class="settings-button" id="settingsBtn" title="Settings"><svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M19.14 12.94c.04-.31.06-.63.06-.94 0-.31-.02-.63-.06-.94l2.03-1.58a.49.49 0 0 0 .12-.61l-1.92-3.32a.488.488 0 0 0-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54a.484.484 0 0 0-.48-.41h-3.84a.484.484 0 0 0-.48.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96a.488.488 0 0 0-.59.22L2.09 8.83a.488.488 0 0 0 .12.61l2.03 1.58c-.04.31-.06.63-.06.94s.02.63.06.94l-2.03 1.58a.488.488 0 0 0-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.27.41.48.41h3.84c.24 0 .44-.17.48-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32a.488.488 0 0 0-.12-.61l-2.03-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/></svg></button>
</div>
</div>
<div class="header">
<div class="device-info-container">
<div class="device-info-content">
<div class="device-name" id="deviceName">LED Lamp</div>
<div class="device-description" id="deviceDescription">Under-cabinet light</div>
</div>
</div>
</div>
<div class="power-section">
<div class="power-button-wrapper">
<button class="power-button" id="powerBtn">
<div class="power-ring"></div>
<div class="power-symbol">
<svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M13 3h-2v10h2V3zm4.83 2.17l-1.42 1.42C17.99 7.86 19 9.81 19 12c0 3.87-3.13 7-7 7s-7-3.13-7-7c0-2.19 1.01-4.14 2.58-5.42L6.17 5.17C4.23 6.82 3 9.26 3 12c0 4.97 4.03 9 9 9s9-4.03 9-9c0-2.74-1.23-5.18-3.17-6.83z"/></svg>
</div>
</button>
</div>
</div>
<div class="brightness-section">
<div class="brightness-header">
<span class="brightness-label" data-i18n="brightness">Brightness</span>
<span class="brightness-value" id="brightnessValue">100%</span>
</div>
<div class="brightness-controls">
<button class="brightness-btn" id="brightnessDown">&minus;</button>
<div class="brightness-slider-container">
<div class="brightness-track" id="brightnessTrack"></div>
<input type="range" class="brightness-slider" id="brightnessSlider" min="5" max="100" value="100">
<div class="brightness-fill" id="brightnessFill"></div>
</div>
<button class="brightness-btn" id="brightnessUp">+</button>
</div>
</div>
</div>
<script>
const translations={en:{brightness:'Brightness',powerOn:'Power ON',powerOff:'Power OFF',settings:'Settings'},ru:{brightness:'Яркость',powerOn:'Включено',powerOff:'Выключено',settings:'Настройки'}};
let deviceState={power:false,brightness:100};
let uiConfig={lang:'en',darkTheme:false,largeFont:false,useBoldValues:false};
const $=id=>document.getElementById(id);
const deviceIconSmall=$('deviceIconSmall'),deviceName=$('deviceName'),deviceDescription=$('deviceDescription'),settingsBtn=$('settingsBtn'),powerBtn=$('powerBtn'),brightnessSlider=$('brightnessSlider'),brightnessValue=$('brightnessValue'),brightnessDown=$('brightnessDown'),brightnessUp=$('brightnessUp'),brightnessFill=$('brightnessFill'),connectionStates=$('connectionStates'),wifiIcon=$('wifiIcon');
function applyUIConfig(){if(uiConfig.darkTheme)document.body.classList.add('dark-theme');else document.body.classList.remove('dark-theme');if(uiConfig.largeFont)document.body.classList.add('large-font');else document.body.classList.remove('large-font');if(uiConfig.useBoldValues)brightnessValue.classList.add('bold-val');else brightnessValue.classList.remove('bold-val');const t=translations[uiConfig.lang]||translations.en;document.querySelector('[data-i18n="brightness"]').textContent=t.brightness;settingsBtn.title=t.settings}
function updateUI(){if(deviceState.power)powerBtn.classList.add('on');else powerBtn.classList.remove('on');brightnessSlider.value=deviceState.brightness;brightnessValue.textContent=deviceState.brightness+'%';brightnessFill.style.width=deviceState.brightness+'%'}
function setupEventListeners(){powerBtn.addEventListener('click',togglePower);brightnessSlider.addEventListener('input',handleSliderChange);brightnessUp.addEventListener('click',()=>adjustBrightness(5));brightnessDown.addEventListener('click',()=>adjustBrightness(-5));settingsBtn.addEventListener('click',showSettingsDialog)}
function togglePower(){deviceState.power=!deviceState.power;if(deviceState.power&&deviceState.brightness<5)deviceState.brightness=50;updateUI();sendState()}
function handleSliderChange(){deviceState.brightness=parseInt(brightnessSlider.value);brightnessValue.textContent=deviceState.brightness+'%';brightnessFill.style.width=deviceState.brightness+'%';sendState()}
function adjustBrightness(d){if(!deviceState.power){deviceState.power=true;powerBtn.classList.add('on')}deviceState.brightness=Math.max(5,Math.min(100,deviceState.brightness+d));brightnessSlider.value=deviceState.brightness;brightnessValue.textContent=deviceState.brightness+'%';brightnessFill.style.width=deviceState.brightness+'%';sendState()}
function sendState(){const data={power:deviceState.power,brightness:deviceState.brightness};fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)}).catch(e=>console.log('Send error:',e))}
function showSettingsDialog(){const svg=settingsBtn.querySelector('svg');const isDark=uiConfig.darkTheme;const borderColor=isDark?'rgba(255,255,255,0.4)':'rgba(0,0,0,0.4)';const topColor=isDark?'#FFFFFF':'#1C1C1E';if(svg){svg.outerHTML='<div class="settings-spinner" style="width:24px;height:24px;border:2px solid '+borderColor+';border-top-color:'+topColor+';border-radius:50%;animation:spin .8s linear infinite;will-change:transform;-webkit-backface-visibility:hidden;backface-visibility:hidden"></div>'}settingsBtn.style.pointerEvents='none';fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'}}).catch(()=>{});setTimeout(()=>{window.location.reload()},15000)}
async function loadUIConfig(){try{const r=await fetch('/api/ui_config');if(r.ok){const c=await r.json();uiConfig={lang:c.language||'en',darkTheme:c.dark_theme||false,largeFont:c.large_font||false,useBoldValues:c.use_bold_values||false};applyUIConfig()}}catch(e){console.log('UI config load error:',e)}}
async function loadDeviceInfo(){try{const r=await fetch('/api/device_info');if(r.ok){const d=await r.json();deviceName.textContent=d.name;deviceDescription.textContent=d.description;if(d.icon_svg)deviceIconSmall.innerHTML=d.icon_svg}}catch(e){console.log('Device info load error:',e)}}
async function loadStatus(){try{const r=await fetch('/api/status');if(r.ok){const s=await r.json();if(s.wifi==='ap'){wifiIcon.textContent='\u{1F4E1}';wifiIcon.classList.add('ap');wifiIcon.classList.remove('sta')}else if(s.wifi==='sta'){wifiIcon.textContent='\u{1F4F6}';wifiIcon.classList.add('sta');wifiIcon.classList.remove('ap')}else{wifiIcon.classList.remove('sta','ap')}}}catch(e){console.log('Status load error:',e)}}
async function loadState(){try{const r=await fetch('/api/state');if(r.ok){const d=await r.json();deviceState={power:d.power||false,brightness:d.brightness||100};updateUI()}}catch(e){console.log('State load error:',e)}}
function init(){applyUIConfig();updateUI();setupEventListeners();loadDeviceInfo();loadUIConfig();loadStatus();loadState()}
init();
// ============================================================================
// Автообновление состояния (поллинг)
// ============================================================================
let statePollInterval = null;
function startStatePolling(intervalMs = 2000) {
    if (statePollInterval) clearInterval(statePollInterval);
    statePollInterval = setInterval(async () => {
        try {
            const r = await fetch('/api/state', { cache: 'no-store' });
            if (r.ok) {
                const d = await r.json();
                // Обновляем только если данные реально изменились
                if (d.power !== deviceState.power || d.brightness !== deviceState.brightness) {
                    console.log('[UI] State updated from server:', d);
                    deviceState = {
                        power: d.power || false,
                        brightness: d.brightness || 100
                    };
                    updateUI();
                }
            }
        } catch (e) {
            // Тихо игнорируем ошибки сети (устройство может быть временно недоступно)
            // console.log('[UI] Poll error:', e);
        }
    }, intervalMs);
    console.log('[UI] State polling started:', intervalMs + 'ms');
}
function stopStatePolling() {
    if (statePollInterval) {
        clearInterval(statePollInterval);
        statePollInterval = null;
        console.log('[UI] State polling stopped');
    }
}
// Запускаем поллинг после инициализации
document.addEventListener('visibilitychange', () => {
    // Паузим поллинг, когда вкладка неактивна — экономия трафика и ресурсов ESP
    if (document.hidden) {
        stopStatePolling();
    } else {
        startStatePolling(2000);
    }
});
// Старт при загрузке
startStatePolling(2000);
</script>
</body>
</html>)rawliteral";
