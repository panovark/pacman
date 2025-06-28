# PacMan

A simple Pac-Man style game built with C++ and [SFML](https://www.sfml-dev.org/). This repository contains a CMake project that compiles the game and copies required resources to the build directory.

## Requirements

- CMake **3.25** or newer
- A C++20 compiler (e.g. `g++`)
- SFML development libraries (graphics, window, system and audio components)

On Debian/Ubuntu based systems you can install the tools and libraries with:

```bash
sudo apt-get update
sudo apt-get install -y cmake g++ libsfml-dev
```

## Building

Create a build directory and compile the project:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

The executable will be placed in `build/PacMan`. Resources (fonts, textures, audio files) are automatically copied next to the executable during the build step.

## Running

Launch the game from the build directory:

```bash
./build/PacMan
```

Enjoy!
