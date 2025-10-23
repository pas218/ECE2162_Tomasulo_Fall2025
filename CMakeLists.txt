cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_COMPILER G++)
project(ECE2162_Tomasulo_Fall2025 LANGUAGES CXX)

include_directories(lib)

add_executable(${PROJECT_NAME} src/main.cpp)

# Link the static library from subproject1 using its alias sub::lib1
# Link the header only library from subproject2 using its alias sub::lib2
# This will cause the include directories for that target to be added to this project
#target_link_libraries(${PROJECT_NAME}
#    classes
#)




