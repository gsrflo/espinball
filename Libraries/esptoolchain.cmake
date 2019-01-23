set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

message("Using esptoolchain.cmake")

get_filename_component(CURRENT_PATH ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)

# setup include directories
include_directories(
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/inc
    ${CURRENT_PATH}/STM32F4/Libraries/CMSIS/Include
    ${CURRENT_PATH}/STM32F4/Libraries/CMSIS/Device/ST/STM32F4xx/Include
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/inc
    ${CURRENT_PATH}/STM32F4/Utilities/STM32_EVAL/STM32F429I-Discovery
    ${CURRENT_PATH}/STM32F4/Utilities/STM32_EVAL/Common
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/include
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Demo/Common/include
    ${CURRENT_PATH}/ugfx
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont
    ${CURRENT_PATH}/ugfx/boards/base/STM32F429i-Discovery/
    ${CURRENT_PATH}/usr
)

#setup sources
file(GLOB USR_SRCS
    ${CURRENT_PATH}/usr/gdisp_lld_ILI9341.c
    ${CURRENT_PATH}/usr/ginput_lld_mouse.c
    ${CURRENT_PATH}/usr/ParTest.c
    ${CURRENT_PATH}/usr/port.c
    ${CURRENT_PATH}/usr/timertest.c
    ${CURRENT_PATH}/usr/system_stm32f4xx.c
    ${CURRENT_PATH}/usr/ESPL_functions.c
    ${CURRENT_PATH}/usr/startup_stm32f429_439xx.S
)

# If you need more sources from the FreeRTOS library, add them below
file(GLOB RTOS_SRCS
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/list.c
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/queue.c
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/tasks.c
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/timers.c
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_1.c
    ${CURRENT_PATH}/FreeRTOS/FreeRTOS/Demo/Common/Minimal/flash.c
)

# If you need more sources from the STMPeripheralDrivers, add them below
file(GLOB PERIPH_SRCS
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
    ${CURRENT_PATH}/STM32F4/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
)

file(GLOB UTILITIES_SRCS
    ${CURRENT_PATH}/STM32F4/Utilities/STM32_EVAL/STM32F429I-Discovery/stm32f429i_discovery_ioe.c
    ${CURRENT_PATH}/STM32F4/Utilities/STM32_EVAL/STM32F429I-Discovery/stm32f429i_discovery_lcd.c
    ${CURRENT_PATH}/STM32F4/Utilities/STM32_EVAL/STM32F429I-Discovery/stm32f429i_discovery_sdram.c
) 

# If you need more sources from the ugfx library, add them below
file(GLOB UGFX_SRCS
    ${CURRENT_PATH}/ugfx/src/gfx.c
    ${CURRENT_PATH}/ugfx/src/gdriver/gdriver.c
    ${CURRENT_PATH}/ugfx/src/gdisp/gdisp_fonts.c
    ${CURRENT_PATH}/ugfx/src/gdisp/gdisp.c
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont/mf_encoding.c
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont/mf_font.c
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont/mf_justify.c
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont/mf_scaledfont.c
    ${CURRENT_PATH}/ugfx/src/gdisp/mcufont/mf_rlefont.c
    ${CURRENT_PATH}/ugfx/src/gevent/gevent.c
    ${CURRENT_PATH}/ugfx/src/ginput/ginput.c
    ${CURRENT_PATH}/ugfx/src/ginput/ginput_mouse.c
    ${CURRENT_PATH}/ugfx/src/gos/gos_freertos.c
    ${CURRENT_PATH}/ugfx/src/gtimer/gtimer.c
    ${CURRENT_PATH}/ugfx/src/gwin/gwin_console.c
    ${CURRENT_PATH}/ugfx/src/gwin/gwin.c
    ${CURRENT_PATH}/ugfx/src/gwin/gwin_wm.c
)

#can use find_program multiple times on same target. First hit will be used. Allows randomly searching at (our) default locations.
find_program(ARM_GCC arm-none-eabi-gcc ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_GCC arm-none-eabi-gcc /DIST/it/sw/amd64/gcc-arm/gcc/bin/ NO_DEFAULT_PATH)
find_program(ARM_GCC arm-none-eabi-gcc NO_DEFAULT_PATH)
find_program(ARM_GCC arm-none-eabi-gcc)
message("ARM_GCC= ${ARM_GCC}")

find_program(ARM_GCXX arm-none-eabi-g++ ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_GCXX arm-none-eabi-g++ /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_GCXX arm-none-eabi-g++ NO_DEFAULT_PATH)
find_program(ARM_GCXX arm-none-eabi-g++)
message("ARM_GCXX= ${ARM_GCXX}")

find_program(ARM_LINKER arm-none-eabi-ld ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_LINKER arm-none-eabi-ld /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_LINKER arm-none-eabi-ld NO_DEFAULT_PATH)
find_program(ARM_LINKER arm-none-eabi-ld)
message("ARM_LINKER= ${ARM_LINKER}")

find_program(ARM_OBJCOPY arm-none-eabi-objcopy ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy NO_DEFAULT_PATH)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
message("ARM_OBJCOPY= ${ARM_OBJCOPY}")

find_program(ARM_OBJDUMP arm-none-eabi-objdump ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_OBJDUMP arm-none-eabi-objdump /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_OBJDUMP arm-none-eabi-objdump NO_DEFAULT_PATH)
find_program(ARM_OBJDUMP arm-none-eabi-objdump)
message("ARM_OBJDUMP= ${ARM_OBJDUMP}")

find_program(ARM_AR arm-none-eabi-ar ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_AR arm-none-eabi-ar /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_AR arm-none-eabi-ar NO_DEFAULT_PATH)
find_program(ARM_AR arm-none-eabi-ar)
message("ARM_AR= ${ARM_AR}")

find_program(ARM_RANLIB arm-none-eabi-ranlib ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_RANLIB arm-none-eabi-ranlib /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_RANLIB arm-none-eabi-ranlib NO_DEFAULT_PATH)
find_program(ARM_RANLIB arm-none-eabi-ranlib)
message("ARM_RANLIB= ${ARM_RANLIB}")

find_program(ARM_SIZE arm-none-eabi-size ${CURRENT_PATH}/../../../bin/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_SIZE arm-none-eabi-size /DIST/it/sw/amd64/gcc-arm/gcc/bin NO_DEFAULT_PATH)
find_program(ARM_SIZE arm-none-eabi-size NO_DEFAULT_PATH)
find_program(ARM_SIZE arm-none-eabi-size)
message("ARM_SIZE= ${ARM_SIZE}")

find_program(ST_FLASH st-flash ${CURRENT_PATH}/../../../bin/gcc-arm/stlink NO_DEFAULT_PATH)
find_program(ST_FLASH st-flash /DIST/it/sw/amd64/gcc-arm/stlink NO_DEFAULT_PATH)
find_program(ST_FLASH st-flash NO_DEFAULT_PATH)
find_program(ST_FLASH st-flash)
message("ST_FLASH= ${ST_FLASH}")


set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -march=armv7e-m -mtune=cortex-m4 -mlittle-endian -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -std=c99 -w -Wunused-value -O3 -ffast-math -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-common --param max-inline-insns-single=1000 -DUSE_STDPERIPH_DRIVER -DGFX_USE_OS_FREERTOS=TRUE -DSTM32F429_439xx=TRUE -g")

set(CMAKE_C_COMPILER ${ARM_GCC})
set(CMAKE_CXX_COMPILER ${ARM_GCXX})

set(CMAKE_AR ${ARM_AR})
set(CMAKE_RANLIB ${ARM_RANLIB})
set(CMAKE_LINKER ${ARM_LINKER})

set(CMAKE_C_LINK_EXECUTABLE
    "${CMAKE_LINKER} -o <TARGET> <OBJECTS> --start-group -lc -lm -lnosys --end-group -L ${CURRENT_PATH}/softfp/arm-none-eabi_softfp -L ${CURRENT_PATH}/softfp/lib_softfp -T ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/usr/stm32f429zi_flash.ld"
)

