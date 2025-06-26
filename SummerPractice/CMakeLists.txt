cmake_minimum_required(VERSION 3.5)

project(SummerPractice LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SummerPractice main.cpp
    test
    test.txt)

include(GNUInstallDirs)
install(TARGETS SummerPractice
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
