add_library(USBX_TARGET INTERFACE)
target_include_directories(USBX_TARGET INTERFACE
  USBX/Target
)
target_link_libraries(USBX_TARGET INTERFACE STM32H7XX_HAL_DRIVER)

file(GLOB USBX_APP_SOURCES USBX/App/*.c)
add_library(USBX_APP STATIC ${USBX_APP_SOURCES})
target_include_directories(USBX_APP PUBLIC
  USBX/App
  USBX/Target
  ../include
)
target_link_libraries(USBX_APP PUBLIC
  USBX_DCD_STM32
)

file(GLOB THREADX_APP_SOURCES ThreadX/App/*.c)
add_library(THREADX_APP STATIC ${THREADX_APP_SOURCES})
target_include_directories(THREADX_APP PUBLIC ThreadX/App)
target_link_libraries(THREADX_APP PUBLIC
  azrtos::threadx
)
