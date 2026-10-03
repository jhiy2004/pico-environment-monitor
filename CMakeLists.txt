cmake_minimum_required(VERSION 3.10)
include(pico_sdk_import.cmake)

project(main)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#set(PICO_PLATFORM "rp2040")
#set(PICO_BOARD "pico")
#set(PICO_COMPILER "pico_arm_cortex_m0plus_gcc")

set(OLED_SDA 14)
set(OLED_SCK 15)
set(OLED_I2C i2c1)
set(OLED_I2C_CLK 400)

set(SD_SPI spi0)
set(SD_SPI_RX 16)
set(SD_SPI_TX 19)
set(SD_SPI_CSn 17)
set(SD_SPI_SCK 18)
set(SD_SPI_CLK 10) # In MHz

set(DHT_PIN 22)

set(ESP01_UART uart1)
set(ESP01_UART_RX 5)
set(ESP01_UART_TX 4)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

pico_sdk_init()

# Biblioteca de Display (INTERFACE)
add_library(Display INTERFACE)

target_include_directories(Display
    INTERFACE
        lib/Display/include
)

# Biblioteca de OLED display
add_library(SSD1306
    lib/SSD1306/src/SSD1306.c
)

target_include_directories(SSD1306
    PRIVATE
        lib/SSD1306/include
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(SSD1306
    PRIVATE
        pico_stdlib
        hardware_i2c
    PUBLIC
        Display
)

# Biblioteca de UI
add_library(UI
    lib/UI/src/UI.c
)

target_include_directories(UI
 
PUBLIC lib/UI/include
)

# Biblioteca de logger
add_library(FileLogger
    lib/FileLogger/src/FileLogger.c
    lib/Logger/src/Logger.c
)

target_include_directories(FileLogger
    PRIVATE
        lib/FileLogger/include
    PUBLIC
        lib/Logger/include
)

target_link_libraries(FileLogger
    PRIVATE
        pico_stdlib
        hardware_spi
)

# Biblioteca de temperatura (INTERFACE)
add_library(TempHumidity INTERFACE)

target_include_directories(TempHumidity
    INTERFACE
        lib/TempHumidity/include
)

# Biblioteca de temperatura mock
add_library(MockTempHumidity
    lib/MockTempHumidity/src/MockTempHumidity.c
)

target_include_directories(MockTempHumidity
    PRIVATE 
        lib/MockTempHumidity/include
)

target_link_libraries(MockTempHumidity
    PUBLIC
        TempHumidity
)

# Biblioteca de temperatura mock
add_library(DHT11
    lib/DHT11/src/DHT11.c
)

target_include_directories(DHT11
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
        lib/DHT11/include
)

target_link_libraries(DHT11
    PUBLIC
        TempHumidity
    PRIVATE
        pico_stdlib
)


# Biblioteca de Wifi Config (INTERFACE)
add_library(WifiConfig INTERFACE)

target_include_directories(WifiConfig
    INTERFACE
        lib/WifiConfig/include
)

# Biblioteca de Wifi (INTEFACE)
add_library(Wifi INTERFACE)

target_include_directories(Wifi
    INTERFACE
        lib/Wifi/include
)

target_link_libraries(Wifi
    INTERFACE
        WifiConfig
        TempHumidity
)

# Biblioteca de Wifi Fixed Config
add_library(WifiFixedConfig
    lib/WifiFixedConfig/src/WifiFixedConfig.c
)

target_include_directories(WifiFixedConfig
    PRIVATE
        lib/WifiFixedConfig/include
)

target_link_libraries(WifiFixedConfig
    PUBLIC
        WifiConfig
)


add_library(Esp01
    lib/Esp01/src/Esp01.c
)

target_include_directories(Esp01
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Esp01
    PUBLIC
        Wifi
    PRIVATE
        pico_stdlib        
)



# Executável
add_subdirectory(lib/pico-vfs)

add_executable(main
    main.c
    fs_init.c
)

target_link_libraries(main
    PRIVATE    
        pico_stdlib
        pico_multicore
        hardware_rtc
    
        SSD1306
        UI
        DHT11
        FileLogger

        WifiFixedConfig
        Esp01

        blockdevice_sd
        filesystem_fat
)

target_include_directories(main
    PRIVATE
        include
        ${CMAKE_CURRENT_BINARY_DIR}
)

pico_enable_filesystem(main)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)