cmake_minimum_required(VERSION 3.20)
project(AFlickerOfStillness C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

#[[
Find and collect files
]]

# Find SDL3 and SDL3_image
##### Make sure to copy the correct file paths for loading SDL before compiling!!!! #####
set(SDL3_DIR "C:/dev/SDL3/SDL3-devel-3.4.16-mingw/SDL3-3.4.16/x86_64-w64-mingw32/lib/cmake/SDL3"
        CACHE PATH "Path to SDL3 Config" FORCE)
set(SDL3_image_DIR "C:/dev/SDL3/SDL3_image-devel-3.4.4-mingw/SDL3_image-3.4.4/x86_64-w64-mingw32/lib/cmake/SDL3_image"
        CACHE PATH "Path to SDL3_image Config" FORCE)
set(CMAKE_PREFIX_PATH
        "C:/dev/SDL3/SDL3-devel-3.4.16-mingw/SDL3-3.4.16/x86_64-w64-mingw32"
        "C:/dev/SDL3/SDL3_image-devel-3.4.4-mingw/SDL3_image-3.4.4/x86_64-w64-mingw32"
        CACHE STRING "Global Prefixes" FORCE
)
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED CONFIG)

# Find all source and header files
file(GLOB SOURCE_FILES_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
file(GLOB_RECURSE SOURCE_FILES_SCREEN_LOGIC
        "${CMAKE_CURRENT_SOURCE_DIR}/screen_logic/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/screen_logic/*.h"
)
file(GLOB_RECURSE SOURCE_FILES_FILE_HANDLING
        "${CMAKE_CURRENT_SOURCE_DIR}/file_handling/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/file_handling/*.h"
)

# Combine all source and header files into a single list
set(SOURCE_FILES ${SOURCE_FILES_ROOT} ${SOURCE_FILES_SCREEN_LOGIC} ${SOURCE_FILES_FILE_HANDLING}
        screen_logic/pause_screen.c
        screen_logic/pause_screen.h
        screen_logic/screen_data.c
        screen_logic/screen_data.h)

#[[
Define project executable
]]

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#[[
Link SDL3 and SDL3_image libraries
]]

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3-shared
        SDL3_image::SDL3_image
)

#[[
Copy files to run folder
]]

# Copy your fonts folder to the build target directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts"
)

# Copy game_data folder run folder
file(GLOB_RECURSE GAME_DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/game_data/*")
set_property(SOURCE main.c PROPERTY OBJECT_DEPENDS "${GAME_DATA_FILES}")
foreach(DATA_FILE_PATH ${GAME_DATA_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/game_data" "${DATA_FILE_PATH}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DATA_FILE_PATH}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/game_data/${REL_PATH}"
    )
endforeach()

# Copy user_data folder to run folder
file(GLOB_RECURSE GAME_DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/user_data/*")
set_property(SOURCE main.c PROPERTY OBJECT_DEPENDS "${GAME_DATA_FILES}")
foreach(DATA_FILE_PATH ${GAME_DATA_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/user_data" "${DATA_FILE_PATH}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DATA_FILE_PATH}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/user_data/${REL_PATH}"
    )
endforeach()

# Copy SDL3.dll to run folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3::SDL3-shared>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

# Copy SDL3_image.dll to run folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:SDL3_image::SDL3_image>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)