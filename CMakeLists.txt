cmake_minimum_required(VERSION 3.25)

project(Raycaster
    VERSION 1.0.0
    LANGUAGES CXX
)

# ==============================================================================
# C++ standard
# ==============================================================================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Options
# ==============================================================================

option(RAYCASTER_BUILD_TESTS "Build unit tests" ON)
option(RAYCASTER_BUILD_DOCS  "Build documentation" OFF)
option(RAYCASTER_ENABLE_SANITIZERS "Enable address/UB sanitizers" OFF)

# ==============================================================================
# Platform detection
# ==============================================================================

if(WIN32)
    set(RAYCASTER_PLATFORM WINDOWS)
elseif(APPLE)
    set(RAYCASTER_PLATFORM MACOS)
elseif(UNIX)
    set(RAYCASTER_PLATFORM LINUX)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

message(STATUS "Platform: ${RAYCASTER_PLATFORM}")

# ==============================================================================
# Sources
# ==============================================================================

set(RAYCASTER_CORE_SOURCES
    src/core/Game.cpp
    src/core/Engine.cpp
    src/core/Timer.cpp
)

set(RAYCASTER_MATH_SOURCES
    src/math/Vector2.cpp
    src/math/MathUtils.cpp
)

set(RAYCASTER_ENTITY_SOURCES
    src/entity/Entity.cpp
    src/entity/Player.cpp
    src/entity/Camera.cpp
)

set(RAYCASTER_WORLD_SOURCES
    src/world/Map.cpp
    src/world/MapLoader.cpp
    src/world/Collision.cpp
)

set(RAYCASTER_RENDERER_SOURCES
    src/renderer/Renderer.cpp
    src/renderer/Raycaster.cpp
    src/renderer/Shader.cpp
)

set(RAYCASTER_PLATFORM_COMMON_SOURCES
    src/platform/Platform.cpp
    src/platform/Console.cpp
    src/platform/Input.cpp
)

if(WIN32)
    set(RAYCASTER_PLATFORM_SOURCES
        src/platform/windows/WinConsole.cpp
        src/platform/windows/WinInput.cpp
    )
else()
    set(RAYCASTER_PLATFORM_SOURCES
        src/platform/unix/NcursesConsole.cpp
        src/platform/unix/NcursesInput.cpp
    )
endif()

set(RAYCASTER_ENGINE_SOURCES
    ${RAYCASTER_CORE_SOURCES}
    ${RAYCASTER_MATH_SOURCES}
    ${RAYCASTER_ENTITY_SOURCES}
    ${RAYCASTER_WORLD_SOURCES}
    ${RAYCASTER_RENDERER_SOURCES}
    ${RAYCASTER_PLATFORM_COMMON_SOURCES}
    ${RAYCASTER_PLATFORM_SOURCES}
)

# Headers (for IDEs only)
set(RAYCASTER_HEADERS
    include/core/Game.hpp
    include/core/Engine.hpp
    include/core/Timer.hpp

    include/math/Vector2.hpp
    include/math/MathUtils.hpp

    include/entity/Entity.hpp
    include/entity/Player.hpp
    include/entity/Camera.hpp

    include/world/Map.hpp
    include/world/MapLoader.hpp
    include/world/Collision.hpp

    include/renderer/Renderer.hpp
    include/renderer/Raycaster.hpp
    include/renderer/Shader.hpp
    include/renderer/Color.hpp

    include/platform/Platform.hpp
    include/platform/Console.hpp
    include/platform/Input.hpp

    include/config/Config.hpp
    include/config/Types.hpp
)

# ==============================================================================
# Engine library
# ==============================================================================

add_library(raycaster_engine
    ${RAYCASTER_ENGINE_SOURCES}
    ${RAYCASTER_HEADERS}
)

target_include_directories(raycaster_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# Platform compile definitions
# ------------------------------------------------------------------------------

if(WIN32)
    target_compile_definitions(raycaster_engine PRIVATE RAYCASTER_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(raycaster_engine PRIVATE RAYCASTER_PLATFORM_MACOS)
else()
    target_compile_definitions(raycaster_engine PRIVATE RAYCASTER_PLATFORM_LINUX)
endif()

# ------------------------------------------------------------------------------
# Compiler warnings
# ------------------------------------------------------------------------------

if(MSVC)
    target_compile_options(raycaster_engine PRIVATE
        /W4
        /utf-8
        /permissive-
        /Zc:__cplusplus
    )
else()
    target_compile_options(raycaster_engine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

# ------------------------------------------------------------------------------
# Sanitizers
# ------------------------------------------------------------------------------

if(RAYCASTER_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(raycaster_engine PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(raycaster_engine PRIVATE
        -fsanitize=address,undefined
    )
endif()

# ------------------------------------------------------------------------------
# Platform libraries
# ------------------------------------------------------------------------------

if(WIN32)

    target_link_libraries(raycaster_engine
        PRIVATE
            kernel32
            user32
    )

else()

    find_package(Curses REQUIRED)

    target_include_directories(raycaster_engine PRIVATE
        ${CURSES_INCLUDE_DIR}
    )

    target_link_libraries(raycaster_engine
        PRIVATE
            ${CURSES_LIBRARIES}
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(raycaster_engine PRIVATE m)
    endif()

endif()

# ==============================================================================
# Main executable
# ==============================================================================

add_executable(raycaster
    src/main.cpp
)

target_link_libraries(raycaster
    PRIVATE
        raycaster_engine
)

# ==============================================================================
# Assets copy (only for the app)
# ==============================================================================

add_custom_command(TARGET raycaster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:raycaster>/assets
    COMMENT "Copying assets to build directory..."
)

# ==============================================================================
# Unit tests
# ==============================================================================

if(RAYCASTER_BUILD_TESTS)

    enable_testing()

    set(TEST_SOURCES
        tests/test_map.cpp
        tests/test_player.cpp
        tests/test_raycaster.cpp
        tests/test_math.cpp
    )

    foreach(TEST_FILE ${TEST_SOURCES})

        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME}
            ${TEST_FILE}
        )

        target_link_libraries(${TEST_NAME}
            PRIVATE
                raycaster_engine
        )

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    endforeach()

endif()

# ==============================================================================
# Documentation
# ==============================================================================

if(RAYCASTER_BUILD_DOCS)

    find_package(Doxygen)

    if(DOXYGEN_FOUND)

        set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )

    else()
        message(WARNING "Doxygen not found, documentation will not be built")
    endif()

endif()

# ==============================================================================
# Install
# ==============================================================================

install(TARGETS raycaster
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
)

install(DIRECTORY assets/
    DESTINATION share/raycaster/assets
)

# ==============================================================================
# Summary
# ==============================================================================

message(STATUS "")
message(STATUS "=== Raycaster Configuration Summary ===")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Platform:     ${RAYCASTER_PLATFORM}")
message(STATUS "  Build type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  Tests:        ${RAYCASTER_BUILD_TESTS}")
message(STATUS "  Docs:         ${RAYCASTER_BUILD_DOCS}")
message(STATUS "  Sanitizers:   ${RAYCASTER_ENABLE_SANITIZERS}")
message(STATUS "======================================")
message(STATUS "")
