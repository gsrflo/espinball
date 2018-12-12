cmake_minimum_required(VERSION 2.8)

project(Demo)

message(${CMAKE_CURRENT_LIST_FILE})

file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/code/*.c)

if(NOT TOOLCHAINDIR)
    set(TOOLCHAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/Libraries/esptoolchain.cmake)
    message("No TOOLCHAINDIR provided. Using default: " ${TOOLCHAINDIR})
endif(NOT TOOLCHAINDIR)
include(${TOOLCHAINDIR})

enable_language(ASM)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(usrlib OBJECT ${SRCS} ${USR_SRCS})
add_library(rtoslib OBJECT ${RTOS_SRCS})
add_library(stmperipheralslib OBJECT ${PERIPH_SRCS})
add_library(stmutilitieslib OBJECT ${UTILITIES_SRCS})
add_library(ugfxlib OBJECT ${UGFX_SRCS})

add_executable(${PROJECT_NAME}.elf $<TARGET_OBJECTS:usrlib> $<TARGET_OBJECTS:rtoslib> $<TARGET_OBJECTS:stmperipheralslib> $<TARGET_OBJECTS:stmutilitieslib> $<TARGET_OBJECTS:ugfxlib>)

add_custom_target(${PROJECT_NAME}.bin
                  COMMAND ${ARM_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
                  COMMAND ${ARM_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
                  COMMAND ${ARM_OBJCOPY} -h -S -D ${PROJECT_NAME}.elf > ${PROJECT_NAME}.lst
                  COMMAND ${ARM_SIZE} ${PROJECT_NAME}.elf
)
add_dependencies(${PROJECT_NAME}.bin ${PROJECT_NAME}.elf)

add_custom_target(flash
                  COMMAND ${ST_FLASH} write ${PROJECT_NAME}.bin 0x8000000
)
add_dependencies(flash ${PROJECT_NAME}.bin)

