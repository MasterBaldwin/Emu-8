cmake_minimum_required(VERSION 3.5)
project(Emu-8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:/Dev/Projects/Emu-8/cmake")
set(SDL2_PATH "C:/Dev/Libraries/SDL2 2.0.4")
set(SOURCE_FILES "src/Chip8.cpp" "src/Chip8.h" "src/Console.cpp" "src/Console.h" "src/Display.cpp" "src/Display.h" "src/File.cpp" "src/File.h" "src/Time.cpp" "src/Time.h")

find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
add_executable(Emu-8 ${SOURCE_FILES})
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(Emu-8 ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES})
