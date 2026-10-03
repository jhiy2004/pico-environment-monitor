cmake_minimum_required(VERSION 3.10)

project(test)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Biblioteca de Display (INTERFACE)
add_library(Display INTERFACE)

target_include_directories(Display
    INTERFACE
        ../lib/Display/include
)

# Biblioteca de console display
add_library(ConsoleDisplay
    ../lib/ConsoleDisplay/src/ConsoleDisplay.c
)

target_include_directories(ConsoleDisplay
    PRIVATE 
        ../lib/ConsoleDisplay/include
)

target_link_libraries(ConsoleDisplay
    PUBLIC
        Display
)

# Biblioteca de UI
add_library(UI
    ../lib/UI/src/UI.c
)

target_include_directories(UI
    PUBLIC ../lib/UI/include
)


# Biblioteca de logger
add_library(ConsoleLogger
    ../lib/ConsoleLogger/src/ConsoleLogger.c
    ../lib/Logger/src/Logger.c
)

target_include_directories(ConsoleLogger
    PRIVATE
        ../lib/ConsoleLogger/include
    PUBLIC
        ../lib/Logger/include
)

# Biblioteca de temperatura (INTERFACE)
add_library(TempHumidity INTERFACE)

target_include_directories(TempHumidity
    INTERFACE
        ../lib/TempHumidity/include
)

# Biblioteca de temperatura mock
add_library(MockTempHumidity
    ../lib/MockTempHumidity/src/MockTempHumidity.c
)

target_include_directories(MockTempHumidity
    PRIVATE 
        ../lib/MockTempHumidity/include
)

target_link_libraries(MockTempHumidity
    PUBLIC
        TempHumidity
)

# Biblioteca de Wifi Config (INTERFACE)
add_library(WifiConfig INTERFACE)

target_include_directories(WifiConfig
    INTERFACE
        ../lib/WifiConfig/include
)

target_link_libraries(WifiConfig
    INTERFACE
        TempHumidity
)

# Biblioteca de Wifi (INTEFACE)
add_library(Wifi INTERFACE)

target_include_directories(Wifi
    INTERFACE
        ../lib/Wifi/include
)

target_link_libraries(Wifi
    INTERFACE
        WifiConfig
)


# Biblioteca de Wifi Fixed Config
add_library(WifiFixedConfig
    ../lib/WifiFixedConfig/src/WifiFixedConfig.c
)

target_include_directories(WifiFixedConfig
    PRIVATE
        ../lib/WifiFixedConfig/include
)

target_link_libraries(WifiFixedConfig
    PUBLIC
        WifiConfig
)

# Biblioteca de Wifi
add_library(MockWifi
    ../lib/MockWifi/src/MockWifi.c
)

target_include_directories(MockWifi
    PRIVATE
        ../lib/MockWifi/include
)

target_link_libraries(MockWifi
    PUBLIC
        Wifi
)


# Executável
add_executable(test
    main_test.c
)

target_link_libraries(test
    PRIVATE
        WifiFixedConfig
        MockWifi
        MockTempHumidity
        UI
        ConsoleDisplay
        ConsoleLogger
)