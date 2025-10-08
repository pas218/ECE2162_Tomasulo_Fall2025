cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER G++)
project(ECE2162_Tomasulo_Fall2025 LANGUAGES CXX)

add_executable(ECE2162_Tomasulo_Fall2025 src/main.cpp)

add_subdirectory(lib)

target_include_directories(ECE2162_Tomasulo_Fall2025 PUBLIC lib)
target_link_libraries(ECE2162_Tomasulo_Fall2025 PUBLIC classes)

