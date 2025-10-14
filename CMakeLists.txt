cmake_minimum_required(VERSION 3.16)
project(OpenGlt VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)

# Set include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Linking/include
    ${CMAKE_SOURCE_DIR}/OpenGlt/src
)

# Source files
set(SOURCES
    OpenGlt/src/glad.c
    OpenGlt/src/graphics/light.cpp
    OpenGlt/src/graphics/material.cpp
    OpenGlt/src/graphics/mesh.cpp
    OpenGlt/src/graphics/model.cpp
    OpenGlt/src/graphics/texture.cpp
    OpenGlt/src/physics/rigidbody.cpp
    OpenGlt/src/physics/enviornment.cpp
    OpenGlt/src/algorithm/bounce.cpp
    OpenGlt/src/io/Camera.cpp
    OpenGlt/src/io/joystick.cpp
    OpenGlt/src/io/keyboard.cpp
    OpenGlt/src/io/Mouse.cpp
    OpenGlt/src/io/Screen.cpp
    OpenGlt/src/Main.cpp
    OpenGlt/src/graphics/Shader.cpp
    OpenGlt/stb.cpp
)

# Header files
set(HEADERS
    OpenGlt/src/graphics/light.h
    OpenGlt/src/graphics/material.h
    OpenGlt/src/graphics/mesh.h
    OpenGlt/src/graphics/model.h
    OpenGlt/src/graphics/models/cube.hpp
    OpenGlt/src/graphics/models/lamp.hpp
    OpenGlt/src/graphics/texture.h
    OpenGlt/src/io/Camera.h
    OpenGlt/src/io/joystick.h
    OpenGlt/src/io/keyboard.h
    OpenGlt/src/io/Mouse.h
    OpenGlt/src/graphics/Shader.h
    OpenGlt/src/io/Screen.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32
        _CONSOLE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
    
    # Link libraries for Windows - using absolute paths
    target_link_libraries(${PROJECT_NAME} 
        ${CMAKE_SOURCE_DIR}/Linking/lib/GLFW/glfw3.lib
        ${CMAKE_SOURCE_DIR}/Linking/lib/assimp/assimp-vc143-mtd.lib
        opengl32
        user32
        gdi32
        shell32
    )
    
    # Copy DLLs to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/OpenGlt/glfw3.dll
        ${CMAKE_SOURCE_DIR}/OpenGlt/assimp-vc143-mtd.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    # Linux/macOS settings
    target_link_libraries(${PROJECT_NAME} 
        OpenGL::GL
        glfw
        assimp
        dl
    )
endif()

# Copy assets to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/OpenGlt/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /sdl>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Enable GLM experimental extensions (required by some glm/gtx headers)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
