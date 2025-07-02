cmake_minimum_required(VERSION 3.20)
project(TSP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt@5/5.15.16_2/lib/cmake/Qt5")

find_package(Qt5 COMPONENTS Core Gui Widgets Charts REQUIRED)
find_package(OpenGL REQUIRED)

set(SOURCES
        main.cpp
        gui.cpp
        tsp.cpp
        towndataparser.cpp
)

set(HEADERS
        gui.h
        town.h
        towndataparser.h
        tsp.h
        rapidcsv.h
)

set(FORMS
        gui.ui
)

add_executable(TSP ${SOURCES} ${HEADERS} ${FORMS})


set(Qt5Gui_OPENGL_INCLUDE_DIR "")

target_link_libraries(TSP PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
)

if(APPLE)
    target_link_libraries(TSP PRIVATE
            "-framework OpenGL"
            "-framework AGL"
    )
endif()