cmake_minimum_required(VERSION 3.20)
project(MiniSurvival LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Workaround for environments where Apple CMake platform vars are missing and
# SFML imported targets fail during configure (@rpath flag unset).
if(APPLE AND NOT CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG)
    set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
endif()
if(APPLE AND NOT CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP)
    set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/engine/*.cpp")
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/game/*.cpp")
file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "src/app/client/*.cpp")
file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/app/server/*.cpp")
file(GLOB_RECURSE TOOL_SOURCES CONFIGURE_DEPENDS "src/tools/*.cpp")

add_library(engine_runtime ${ENGINE_SOURCES})
target_include_directories(engine_runtime PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(engine_runtime PUBLIC SFML::Graphics SFML::Window SFML::System)

add_library(game_sim ${GAME_SOURCES})
target_include_directories(game_sim PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(game_sim PUBLIC engine_runtime)

add_executable(survival_client ${CLIENT_SOURCES})
target_include_directories(survival_client PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(survival_client PRIVATE game_sim engine_runtime)

add_executable(survival_server ${SERVER_SOURCES})
target_include_directories(survival_server PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(survival_server PRIVATE game_sim engine_runtime SFML::System)

if(TOOL_SOURCES)
    add_executable(asset_tool ${TOOL_SOURCES})
    target_include_directories(asset_tool PRIVATE "${CMAKE_SOURCE_DIR}/src")
    target_link_libraries(asset_tool PRIVATE SFML::Graphics SFML::System)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/src/assets")
    add_custom_command(TARGET survival_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/src/assets"
                "$<TARGET_FILE_DIR:survival_client>/assets")
endif()

if(MSVC)
    target_compile_options(engine_runtime PRIVATE /W4 /permissive-)
    target_compile_options(game_sim PRIVATE /W4 /permissive-)
    target_compile_options(survival_client PRIVATE /W4 /permissive-)
    target_compile_options(survival_server PRIVATE /W4 /permissive-)
    if(TARGET asset_tool)
        target_compile_options(asset_tool PRIVATE /W4 /permissive-)
    endif()
else()
    target_compile_options(engine_runtime PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(game_sim PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(survival_client PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(survival_server PRIVATE -Wall -Wextra -Wpedantic)
    if(TARGET asset_tool)
        target_compile_options(asset_tool PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()
