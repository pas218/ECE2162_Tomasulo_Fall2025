# ECE2162_Tomasulo_Fall2025
I propose this project will follow the file structure outlined in this article: https://medium.com/@gtech.govind2000/best-practices-for-structuring-c-projects-industry-standards-71b82f6b145c.

## How to build

In repository, enter the following commands:

chmod +x build_project.sh

./build_project.sh

If that doesn't work, then enter the following commands:

mkdir build && cd build

cmake -G "MinGW Makefiles" ..

cmake --build .

The executable should be in the build folder.