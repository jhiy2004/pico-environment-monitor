# Pico Environment Monitor


## Project structure
```
project
├─ CMakeLists.txt
├─ config.h.in
├─ fs_init.c
├─ generate_image_include
│  ├─ .env.example
│  ├─ generate_on_off_icons_array.py
│  ├─ generate_small_number_array.py
│  ├─ generate_temp_font_array.py
│  ├─ img_to_array.py
│  └─ requirements.txt
├─ images
│  ├─ class_diagram.png
│  ├─ OLED
│  │  ├─ full_screen.bmp
│  │  ├─ full_screen_network.bmp
│  │  ├─ on_off_icons.bmp
│  │  ├─ small_numbers_spritesheet.bmp
│  │  └─ temp_font_spritesheet.bmp
│  └─ pico_pinout.svg
├─ lib
│  ├─ consoleDisplay
│  │  ├─ include
│  │  │  └─ consoleDisplay.h
│  │  └─ src
│  │     └─ consoleDisplay.c
│  ├─ consoleLogger
│  │  ├─ include
│  │  │  └─ consoleLogger.h
│  │  └─ src
│  │     └─ consoleLogger.c
│  ├─ DHT11
│  │  ├─ include
│  │  │  └─ DHT11.h
│  │  └─ src
│  │     └─ DHT11.c
│  ├─ display
│  │  └─ display.h
│  ├─ Esp01
│  │  └─ src
│  │     └─ Esp01.c
│  ├─ logger
│  │  ├─ include
│  │  │  └─ logger.h
│  │  └─ src
│  │     └─ logger.c
│  ├─ MockTempHumidity
│  │  └─ src
│  │     └─ MockTempHumidity.c
│  ├─ MockWifi
│  │  ├─ include
│  │  └─ src
│  │     └─ MockWifi.c
│  ├─ pico-vfs
│  ├─ SDLogger
│  │  ├─ include
│  │  │  └─ SDLogger.h
│  │  └─ src
│  │     └─ SDLogger.c
│  ├─ ssd1306
│  │  ├─ include
│  │  │  └─ ssd1306.h
│  │  └─ src
│  │     └─ ssd1306.c
│  ├─ TempHumidity
│  │  ├─ include
│  │  │  └─ TempHumidity.h
│  │  └─ src
│  ├─ ui
│  │  ├─ include
│  │  │  ├─ full_screen.h
│  │  │  ├─ on_off_icons.h
│  │  │  ├─ small_numbers.h
│  │  │  ├─ temp_numbers.h
│  │  │  └─ ui.h
│  │  └─ src
│  │     └─ ui.c
│  ├─ Wifi
│  │  └─ include
│  │     └─ Wifi.h
│  ├─ WifiConfig
│  │  ├─ include
│  │  │  └─ WifiConfig.h
│  │  └─ src
│  ├─ WifiFileConfig
│  │  ├─ include
│  │  └─ src
│  │     └─ WifiFileConfig.c
│  └─ WifiFixedConfig
│     ├─ include
│     └─ src
│        └─ WifiFixedConfig.c
├─ main.c
├─ pico_sdk_import.cmake
├─ README.md
├─ sd-content
│  ├─ index.html
│  ├─ index.js
│  └─ style.css
└─ tests
   ├─ CMakeLists.txt
   └─ main_test.c
```

## Components

## Tests Instructions

## Build Instructions

## Images
