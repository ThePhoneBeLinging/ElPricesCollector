cmake_minimum_required(VERSION 3.28)
project(ElPricesCollector)

set(CMAKE_CXX_STANDARD 20)


add_library(ElPricesCollector STATIC
        ElPricesCollector.cpp
        ElPricesStorageController.cpp
        HourPrice.cpp
        Date.cpp
        TimeUtil.cpp
)

target_include_directories(ElPricesCollector PUBLIC include)