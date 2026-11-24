---
id: 20261120-145759-etltest-src-secret-h
scope: project
path: 
saved_at: 2026-11-20T14:57:59.208Z
tags: secrets, etltest, build
---

# ETLTest: не перезаписывать src/secret.h

В ETLTest файл src/secret.h содержит реальные учётные данные MQTT: он в репозиторий не входит (git-ignored), а восстанавливается копированием из приватного C:\Projects\Arduino\ETLSecrets\secret.h. Никогда не копировать поверх него src/secret.h.example (это шаблон с заглушками) — для проверки сборки с USE_WIFI_UI_SERVER использовать уже существующий secret.h. Если файл потерян, восстановить из ETLSecrets и сверить SHA256.
