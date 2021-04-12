# Simulation code for magnetic percolation processes

## About

Simulate the magnetic percolation process on a regular grid.

![example percolation grid](https://github.com/lbotsch/magnetic-percolation-simulation/blob/main/test_grid.png?raw=true)


## Build instructions

To build the code, you need the following:
 - cmake (https://cmake.org/)
 - A c++17 compiler (tested with GCC 9.3)
 - cairomm (https://www.cairographics.org/cairomm/)
 - libavformat, libavcodec, libswresample, libswscale, libavutil (https://libav.org/)

To build the code, run the following commands:
```bash
mkdir build
cd build
cmake ..
make
```
This will create the executables `sim` and `vis` in the root project folder.
