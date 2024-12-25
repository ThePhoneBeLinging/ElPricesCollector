cmake_minimum_required(VERSION 3.28)
project(ElPricesCollector)

set(CMAKE_CXX_STANDARD 20)


add_library(ElPricesCollector STATIC
        ElPricesCollector.cpp
        ElPricesCollector.h

)

target_include_directories(ElPricesCollector PUBLIC src/include)
target_include_directories(ElPricesCollector PUBLIC src)