cmake_minimum_required(VERSION 3.20)
project(TSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Убедись, что Qt находится в пути поиска (обычно не нужно, если установлен через Homebrew)
set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt@5/5.15.16_2/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS Core Gui Widgets Charts REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCES
        main.cpp
        gui.cpp
)

set(HEADERS
        gui.h
)

set(FORMS
        gui.ui
)

add_executable(TSP ${SOURCES} ${HEADERS} ${FORMS})

# Отключаем поиск OpenGL в Qt5Gui
set(Qt5Gui_OPENGL_INCLUDE_DIR "")  # Пустая строка, чтобы Qt5 не искал gl.h

# Линкуем Qt5 без OpenGL
target_link_libraries(TSP PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
)

# На macOS добавляем только фреймворки (без заголовков)
if(APPLE)
    target_link_libraries(TSP PRIVATE
            "-framework OpenGL"
            "-framework AGL"
    )
endif()