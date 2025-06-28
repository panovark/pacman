cmake_minimum_required(VERSION 3.25)
project(PacMan)

set(CMAKE_CXX_STANDARD 20)

file(COPY resources/fonts/PressStart2P.ttf
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/fonts)

file(COPY resources/audio
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

find_package(SFML COMPONENTS Graphics Window Audio)
if(NOT SFML_FOUND)
    unset(SFML_FOUND CACHE)
    find_package(SFML COMPONENTS graphics window audio REQUIRED)
endif()

add_executable(PacMan
        src/main.cpp
        src/Game.cpp
        src/Ghost.cpp
        src/Player.cpp
        src/Level.cpp
)

if(TARGET SFML::Graphics)
    target_link_libraries(PacMan
            SFML::Graphics
            SFML::Window
            SFML::System
            SFML::Audio)
else()
    target_link_libraries(PacMan
            sfml-graphics
            sfml-window
            sfml-system
            sfml-audio)
endif()
