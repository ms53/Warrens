cmake_minimum_required(VERSION 3.16)
project(Warrens)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# SFML
# -----------------------------
set(SFML_DIR "C:/SFML-3.0.2/lib/cmake/SFML")
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# -----------------------------
# Source files (auto-include)
# -----------------------------
file(GLOB_RECURSE COMMANDS_SRC CONFIGURE_DEPENDS
    src/core/*.cpp
    src/assets/*.cpp
    src/rendering/*.cpp
    src/systems/*.cpp
    src/ui/*.cpp
    src/commands/*.cpp
    src/assets/*.cpp
    src/entities/*.cpp
    src/events/*.cpp
    src/input/*.cpp
    src/map/*.cpp
    src/utils/*.cpp
)

# Always include main
set(MAIN_SRC
    src/main.cpp
)

# -----------------------------
# Executable
# -----------------------------
add_executable(Warrens
    ${MAIN_SRC}
    ${CORE_SRC}
    ${SYSTEMS_SRC}
    ${RENDERING_SRC}
    ${UI_SRC}
    ${COMMANDS_SRC}
)

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(Warrens PRIVATE
    src
)

# -----------------------------
# Link SFML
# -----------------------------
target_link_libraries(Warrens PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)