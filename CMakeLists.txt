cmake_minimum_required(VERSION 3.10)
project(percolation)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(sim grid.cpp sim.cpp)
add_executable(vis grid.cpp graphics.cpp vis.cpp)
add_executable(vis_test grid.cpp graphics.cpp vis_test.cpp)

find_package(PkgConfig REQUIRED)


set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)
target_link_libraries(sim Threads::Threads)


pkg_check_modules(CAIROMM REQUIRED IMPORTED_TARGET
  cairomm-1.0)
target_include_directories(vis PUBLIC PkgConfig::CAIROMM)
target_link_libraries(vis PkgConfig::CAIROMM)
target_include_directories(vis_test PUBLIC PkgConfig::CAIROMM)
target_link_libraries(vis_test PkgConfig::CAIROMM)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libswresample
  libswscale
  libavutil)
target_include_directories(vis PUBLIC PkgConfig::LIBAV)
target_link_libraries(vis PkgConfig::LIBAV)

target_include_directories(vis_test PUBLIC PkgConfig::LIBAV)
target_link_libraries(vis_test PkgConfig::LIBAV)
