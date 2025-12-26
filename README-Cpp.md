Инструкция по развертыванию и сборке проекта
Этот проект представляет собой OPC UA клиент на C++/Qt. Для управления зависимостями используется vcpkg в режиме манифеста (vcpkg.json).

1. Подготовка окружения
Сценарий А: vcpkg еще не установлен
Если у вас нет vcpkg, выполните следующие команды в PowerShell от имени Администратора:

git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe integrate install
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "Machine")
После этого перезапустите терминал.

Сценарий Б: vcpkg уже есть в системе
Если vcpkg уже установлен (например, в D:\dev\vcpkg), просто укажите путь к нему в переменной окружения:

[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\путь\к\вашему\vcpkg", "Machine")
После этого перезапустите терминал.

2. Сборка проекта
Теперь мы используем переменную $env:VCPKG_ROOT, чтобы команда была универсальной для любого компьютера. Выполните эти команды в папке проекта:

Шаг 1: Конфигурация (CMake)
Мы используем файл манифеста vcpkg.json, поэтому vcpkg сам скачает open62541, qtbase и gtest.

cmake -S . -B build -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
Шаг 2: Компиляция

cmake --build build --config Debug
3. Развертывание (Deployment) и запуск
Для работы Qt-интерфейса необходимо скопировать плагины (библиотеки отрисовки и платформы) в папку с исполняемым файлом.

Шаг 3: Копирование плагинов Qt
Примечание: Путь в манифесте vcpkg обычно ведет в подпапку vcpkg_installed внутри вашего проекта.

PowerShell

Copy-Item ".\build\vcpkg_installed\x64-windows\Qt6\plugins" -Destination ".\build\Debug" -Recurse -Force
Шаг 4: Запуск приложения
PowerShell

& ".\build\Debug\opcua_qt_client.exe"
Что делает эта инструкция (для справки):

Автоматизация зависимостей: Благодаря vcpkg.json, вам не нужно искать и скачивать библиотеки вручную.

Гибкость путей: Использование $env:VCPKG_ROOT вместо жестко прописанного пути (C:/Users/konst/...) позволяет запустить сборку на любом ПК.

Корректный запуск GUI: Копирование папки plugins гарантирует, что Qt найдет драйверы для отрисовки окон (библиотеку qwindows.dll), и программа не вылетит с ошибкой при старте.