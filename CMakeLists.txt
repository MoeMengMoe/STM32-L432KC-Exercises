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
add_executable(${CMAKE_PROJECT_NAME})

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
)

# 2. 递归扫描源文件（确保路径加引号，防止空格报错）
file(GLOB_RECURSE LIBDRIVER_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/libdriver_ssd1306/*.c")

# 3. 将源文件添加到目标
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${LIBDRIVER_SRCS})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/drv8833_hal"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/drv8833_hal/drv8833.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Middlewares/drv8833_hal/drv8833_motor.c"
)