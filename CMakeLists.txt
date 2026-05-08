cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Test_L432KC)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/Inc/rtc_app.h
        Core/Src/rtc_app.c
        Core/Inc/AHT20.h
        Core/Src/AHT20.c
        Core/Inc/drv8833.h
        Core/Src/drv8833.c
        Core/Inc/fan_app.h
        Core/Src/fan_app.c)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# --- 在文件末尾添加以下内容 ---

# 检查是否定义了 objcopy 工具（通常由工具链文件提供）
if(NOT CMAKE_OBJCOPY)
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
endif()

# 为可执行文件添加后处理步骤
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        # 生成 .hex 文件
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex
        # 生成 .bin 文件
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin
        COMMENT "Generating .hex and .bin files..."
)

# 可选：编译完成后自动显示内存占用情况 (Size)
if(NOT CMAKE_SIZE)
    set(CMAKE_SIZE arm-none-eabi-size)
endif()

# 1. 使用绝对路径包含头文件
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/interface"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/example"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/interface"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/example"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/interface"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/example"
)

# 2. 添加实际使用的中间件源文件
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/src/driver_ssd1306.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/interface/driver_ssd1306_interface_template.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/example/driver_ssd1306_basic.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/src/driver_aht20.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/interface/driver_aht20_interface_template.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_aht20/example/driver_aht20_basic.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/src/driver_bmp280.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/interface/driver_bmp280_interface_template.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_bmp280/example/driver_bmp280_basic.c"
)
