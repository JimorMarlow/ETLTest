#pragma once

// version.h - ETLTest version control
// NOTE: after version changing run script in terminal:
// > python sync_version.py
// and config files: package.json will be patched...

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 17

// Хелпер для stringify
#define APP_STRINGIFY_HELPER(x) #x
#define APP_STRINGIFY(x) APP_STRINGIFY_HELPER(x)

// Собрать строку версии "X.X.X"
#define APP_VERSION_STRING APP_STRINGIFY(APP_VERSION_MAJOR) "." APP_STRINGIFY(APP_VERSION_MINOR) "." APP_STRINGIFY(APP_VERSION_PATCH)
