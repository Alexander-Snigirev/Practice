cmake_minimum_required(VERSION 3.30)

project(SummerPractice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SummerPractice main.cpp
    test.txt)

include(GNUInstallDirs)
install(TARGETS SummerPractice
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
