cmake_minimum_required(VERSION 3.25)
project(ElPricesCollector)

set(CMAKE_CXX_STANDARD 20)

find_package(SQLite3 REQUIRED)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG bb01c8db702fb41e5497aee9c0559ddf4bf13749)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(ElPricesCollector STATIC
        ElPricesCollector.cpp
        ElPricesStorageController.cpp
        HourPrice.cpp
        Date.cpp
        FeeController.cpp
        FeeController.h
        CeriusFees/Fee.cpp
        CeriusFees/Fee.h
        CeriusFees/Fee.cpp
        CeriusFees/Fee.h
)

target_include_directories(ElPricesCollector PUBLIC include)
target_link_libraries(ElPricesCollector PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(ElPricesCollector PRIVATE cpr::cpr)
target_link_libraries(ElPricesCollector PRIVATE Utility)
target_link_libraries(ElPricesCollector PRIVATE SQLiteCpp)