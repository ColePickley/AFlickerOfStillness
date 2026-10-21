cmake_minimum_required(VERSION 3.20)
project(AFlickerOfStillness C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# -----------------------------------------------------------------------------
# 1. Load Local SDL3 Archive
# -----------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(sdl3 URL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL-main.zip")

set(SDL_SHARED ON CACHE BOOL "Build shared SDL library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build static SDL library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test binaries" FORCE)

FetchContent_MakeAvailable(sdl3)

# -----------------------------------------------------------------------------
# 2. Load Local SDL3_image Archive (FIXED VARIABLES)
# -----------------------------------------------------------------------------
FetchContent_Declare(
        sdl3_image
        URL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL_image-main.zip"
)

# FIX: Disable vendoring so it doesn't look for the missing external/ folders
set(SDLIMAGE_VENDORED OFF CACHE BOOL "Use system packages instead of missing vendored submodules" FORCE)
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "Disable SDL_image sample programs" FORCE)

# Optional: Disable advanced formats that require heavy external submodules
set(SDLIMAGE_AVIF OFF CACHE BOOL "Disable AVIF to avoid submodule dependencies" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "Disable WebP to avoid submodule dependencies" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "Disable JXL to avoid submodule dependencies" FORCE)

FetchContent_MakeAvailable(sdl3_image)

# -----------------------------------------------------------------------------
# 3. Find and Collect Source Files
# -----------------------------------------------------------------------------
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

set(SOURCE_FILES ${SOURCE_FILES_ROOT} ${SOURCE_FILES_SCREEN_LOGIC} ${SOURCE_FILES_FILE_HANDLING}
        AStar.c
        astar.h)

# -----------------------------------------------------------------------------
# 4. Define Project Executable
# -----------------------------------------------------------------------------
if(WIN32)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

# -----------------------------------------------------------------------------
# 5. Link SDL3 Source Targets
# -----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3-shared
        SDL3_image-shared
)

# -----------------------------------------------------------------------------
# 6. Post-Build Actions: Asset Tracking and Dynamic Runtime Linking
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts"
)

file(GLOB_RECURSE GAME_DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/game_data/*")
foreach(DATA_FILE_PATH ${GAME_DATA_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/game_data" "${DATA_FILE_PATH}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DATA_FILE_PATH}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/game_data/${REL_PATH}"
    )
endforeach()

file(GLOB_RECURSE USER_DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/user_data/*")
foreach(DATA_FILE_PATH ${USER_DATA_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/user_data" "${DATA_FILE_PATH}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DATA_FILE_PATH}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/user_data/${REL_PATH}"
    )
endforeach()

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3-shared>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3_image-shared>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endif()