cmake_minimum_required(VERSION 3.5)

project(GeneticAlgorithm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GeneticAlgorithm main.cpp
    tsp.h
    tsp.cpp
    rapidcsv.h)

include(GNUInstallDirs)
install(TARGETS GeneticAlgorithm
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
