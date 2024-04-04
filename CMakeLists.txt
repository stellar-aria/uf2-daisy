cmake_minimum_required(VERSION 3.28)
set(CMAKE_SYSTEM_NAME Generic)
cmake_path(SET CMAKE_TOOLCHAIN_FILE cmake/autodetect.cmake)
cmake_path(ABSOLUTE_PATH CMAKE_TOOLCHAIN_FILE)

project(uf2-daisy
  VERSION 0.1.0
  LANGUAGES CXX C ASM
)

set(THREADX_ARCH "cortex_m7")
set(THREADX_TOOLCHAIN "gnu")

# add dependencies
include(cmake/CPM.cmake)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
CPMAddPackage("gh:eclipse-threadx/threadx@6.4.1_rel")
CPMAddPackage("gh:eclipse-threadx/usbx@6.4.1_rel")
CPMAddPackage("gh:eclipse-threadx/filex@6.4.1_rel")
CPMAddPackage("gh:eclipse-threadx/netxduo@6.4.1_rel")
CPMAddPackage("gh:ARM-Software/CMSIS_6@6.0.0")
CPMAddPackage("gh:STMicroelectronics/stm32h7xx_hal_driver@1.11.3")
CPMAddPackage("gh:STMicroelectronics/cmsis_device_h7@1.10.4")
add_subdirectory(vendor)

add_subdirectory(rtos)

file(GLOB_RECURSE BOOTLOADER_SOURCES src/*.c src/*.S)
add_executable(bootloader ${BOOTLOADER_SOURCES})
target_include_directories(bootloader PUBLIC include)

target_link_libraries(bootloader PUBLIC
  USBX_APP
  THREADX_APP
)

cmake_path(SET LINKER_SCRIPT NORMALIZE ${CMAKE_CURRENT_LIST_DIR}/STM32H750IB_FLASH.ld)

set_target_properties(bootloader PROPERTIES
    C_STANDARD 23
    C_STANDARD_REQUIRED YES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(bootloader PUBLIC
    LINKER:-T,${LINKER_SCRIPT}
    $<$<CONFIG:DEBUG>:LINKER:-Map=bootloader.map>
    $<$<CONFIG:DEBUG>:LINKER:--cref>
    LINKER:--gc-sections
    LINKER:--check-sections
    LINKER:--unresolved-symbols=report-all
    LINKER:--warn-common
    $<$<CXX_COMPILER_ID:GNU>:LINKER:--warn-section-align>

  # Currently a GSoC project to port this to LLD
  $<$<CXX_COMPILER_ID:GNU>:LINKER:--print-memory-usage>

  $<$<CONFIG:RELEASE>:-flto>
)