cmake_minimum_required(VERSION 3.28)
project(ElPricesCollector)

set(CMAKE_CXX_STANDARD 20)


add_library(ElPricesCollector STATIC
        ElPricesCollector.cpp
        include/ElPricesCollector/ElPricesCollector.h
        ElPricesStorageController.cpp
        ElPricesStorageController.h
        PriceObjects/HourPrice.cpp
        PriceObjects/HourPrice.h
        PriceObjects/Date.cpp
        PriceObjects/Date.h
)

target_include_directories(ElPricesCollector PUBLIC include)