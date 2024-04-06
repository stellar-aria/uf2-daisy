# CMSIS Core
add_library(CMSIS_CORE INTERFACE)
target_include_directories(CMSIS_CORE INTERFACE
  ${CMSIS_6_SOURCE_DIR}/CMSIS/Core/Include
)

# CMSIS Device
add_library(CMSIS_DEVICE_H7 STATIC)
target_include_directories(CMSIS_DEVICE_H7 PUBLIC
  ${cmsis_device_h7_SOURCE_DIR}/Include
)
target_sources(CMSIS_DEVICE_H7 PRIVATE
  cmsis_device_h7/src/system_stm32h7xx.c
)
target_link_libraries(CMSIS_DEVICE_H7 PUBLIC
  CMSIS_CORE
  STM32H7XX_HAL_DRIVER
)

# STM32H7xx HAL Driver
file(GLOB STM32H7XX_HAL_DRIVER_SOURCES ${stm32h7xx_hal_driver_SOURCE_DIR}/Src/*.c) # All .c files
list(FILTER STM32H7XX_HAL_DRIVER_SOURCES EXCLUDE REGEX "template.c$") # unless they're templates
add_library(STM32H7XX_HAL_DRIVER STATIC ${STM32H7XX_HAL_DRIVER_SOURCES})

target_include_directories(STM32H7XX_HAL_DRIVER PUBLIC
  ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc
  ${stm32h7xx_hal_driver_SOURCE_DIR}/Inc/Legacy
  stm32h7xx_hal_driver/include
)

# Weird Circular dependency (HAL -> CMSIS-Device -> HAL)
target_link_libraries(STM32H7XX_HAL_DRIVER PUBLIC
  CMSIS_DEVICE_H7
)

target_compile_options(STM32H7XX_HAL_DRIVER PRIVATE
    -Wno-unused-but-set-variable
)

target_compile_definitions(STM32H7XX_HAL_DRIVER PRIVATE
  USE_HAL_DRIVER
  USE_FULL_LL_DRIVER
)


file(GLOB USBX_DCD_STM32_SOURCES usbx_stm32_device_controllers/src/*.c)
add_library(USBX_DCD_STM32 STATIC ${USBX_DCD_STM32_SOURCES})
target_include_directories(USBX_DCD_STM32 PUBLIC
  usbx_stm32_device_controllers/include
  usbx_stm32_device_controllers/config
)
target_link_libraries(USBX_DCD_STM32 PUBLIC
  STM32H7XX_HAL_DRIVER
  azrtos::usbx
)
