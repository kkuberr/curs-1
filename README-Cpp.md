# OPC UA Qt Клиент (C++) — скелет

В этой папке находится скелет Qt (Widgets) приложения для GUI-клиента OPC UA. Проект подготовлен так, чтобы вы могли:

- Сразу собрать и запустить графический интерфейс (без зависимости от OPC UA).
- Позже включить реальную интеграцию с OPC UA (`open62541`), установив библиотеку и включая зависимости через `vcpkg`.

Требования (рекомендуется)
- Windows с Visual Studio (MSVC) или MSVC Build Tools (x64)
- git
- CMake 3.16+
- vcpkg (рекомендовано)

Краткая инструкция: установка `vcpkg`, нужных пакетов и сборка в VS Code

1) Установка/обновление `vcpkg` (в каталоге `C:\vcpkg`)

```powershell
# если vcpkg ещё не клонирован
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# если vcpkg уже есть — обновите
cd C:\vcpkg
git pull
.\bootstrap-vcpkg.bat
```

2) Установка зависимостей через `vcpkg` (рекомендуемый triplet: `x64-windows`)

```powershell
cd C:\vcpkg
.\vcpkg.exe install qt6-base:x64-windows
.\vcpkg.exe install open62541:x64-windows
.\vcpkg.exe install gtest:x64-windows
```

Примечания:
- Если при установке пакета вы видите `Invalid triplet`, выполните `git pull` и `bootstrap-vcpkg.bat`, чтобы обновить список портов и triplets.
- Названия пакетов в `vcpkg` могут меняться; используйте `.\vcpkg.exe search <name>`.

3) Интеграция с Visual Studio Code (рекомендуется указывать toolchain явно при конфигурации CMake)

Откройте PowerShell в корне проекта `curs-1` и выполните:

```powershell
mkdir -Force build
cd build
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build . --config Debug
```

Если хотите указывать `Qt6_DIR` вручную (иногда нужно):

```powershell
Get-ChildItem C:\vcpkg\installed -Recurse -Filter Qt6Config.cmake
# затем
cmake .. -DQt6_DIR="C:/vcpkg/installed/x64-windows/share/qt6/lib/cmake/Qt6" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -A x64
```

4) Запуск тестов (gtest)

```powershell
# из папки build
cmake --build . --config Debug --target test_opcua_client
ctest -C Debug --output-on-failure
```

5) Если вы не хотите использовать `open62541` пока — соберите только UI (Qt можно не устанавливать, CMake пропустит GUI):

```powershell
mkdir -Force build
cd build
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

Добавление `vcpkg.json` (manifest mode)
- Я добавлю файл `vcpkg.json` в корень проекта (если вы хотите manifest mode, CMake автоматически использует этот файл при конфигурации с `-DCMAKE_TOOLCHAIN_FILE=...`).

Дальнейшие шаги, которые я могу выполнить
- Добавить пример `vcpkg.json` и подробную секцию по отладке ошибок сборки (я добавлю `vcpkg.json` сейчас).
- Реализовать поддержку `open62541` в `OpcUaClient` после того, как зависимости будут установлены.

Если хотите, могу сразу добавить `vcpkg.json` в репозиторий с нужными зависимостями.
# OPC UA Qt Клиент (C++) — скелет

В этой папке находится скелет Qt (Widgets) приложения для GUI-клиента OPC UA. Проект подготовлен так, чтобы вы могли:

- Сразу собрать и запустить графический интерфейс (без зависимости от OPC UA).
- Позже включить реальную интеграцию с OPC UA (`open62541`), установив библиотеку и включив опцию CMake `BUILD_WITH_OPEN62541`.

Требования (рекомендуется)
- Windows с Visual Studio (MSVC) или MSVC Build Tools
- git
- CMake 3.16+
- vcpkg (опционально, но рекомендовано)

Рекомендуемый рабочий путь (vcpkg)
1. Клонируйте и подготовьте vcpkg (один раз):

    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    .\\bootstrap-vcpkg.bat
    .\\vcpkg.exe integrate install

2. Установите Qt6 и open62541 (замените архитектуру при необходимости):

    .\\vcpkg.exe install qt6-base:x64-windows
    .\\vcpkg.exe install open62541:x64-windows

   Примечание: имена пакетов могут меняться; если порта `open62541` нет в vcpkg в данный момент, можно использовать метод amalgamation (см. ниже).

3. Настройка и сборка через CMake (пример):

    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake -A x64 -DBUILD_WITH_OPEN62541=ON
    cmake --build . --config Release

4. Запустите полученный исполняемый файл из каталога сборки.

Сборка без open62541 (только UI)
- Просто настройте CMake без флага toolchain и оставьте `BUILD_WITH_OPEN62541` выключенной (по умолчанию):

    mkdir build && cd build
    cmake .. -A x64
    cmake --build . --config Release

Интеграция open62541 — варианты
- vcpkg: предпочтительный способ на Windows для воспроизводимых сборок.
- Amalgamation: скачайте `open62541.h`/`open62541.c` и добавьте их в `src/third_party`, затем подключите к CMake; это позволяет не использовать vcpkg, но требует ручной поддержки.

Дальнейшие шаги, которые я могу выполнить
- По вашему подтверждению я могу реализовать интеграцию с open62541 (подключение, обход узлов, чтение/запись) и протестировать инструкции сборки. При желании могу также добавить пример, подключающийся к публичному тестовому серверу.
