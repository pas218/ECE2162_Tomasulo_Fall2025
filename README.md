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



## Zach's build procedure
Open an MSYS2 UCRT64 terminal
cd into the repository (i.e.: cd "C:\ECE2162\ECE2162_Tomasulo_Fall2025")
chmod +x build_project.sh
export PATH="/c/Program Files/CMake/bin:$PATH"
source ~/.bashrc
./build_project.sh
cd build
cmake -G "MinGW Makefiles" ..
cd ..
./build/ECE2162_Tomasulo_Fall2025.exe

## Calling the function
You can specify the file path of the input file through the command line.
If no file path is given, the program will use the default path of "src/input.txt".

Example (uses "src/input_file.txt"):
./build/ECE2162\ECE2162_Tomasulo_Fall2025 src/input_file.txt