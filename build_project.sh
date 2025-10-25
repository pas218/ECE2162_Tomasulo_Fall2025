#!/bin/bash

BUILD_FOLDER="build"

if [ -d "$BUILD_FOLDER" ]; then
	rm -r build
	echo "Deleting current build folder."
fi

mkdir build

cd build

cmake -G "MinGW Makefiles" ..

cmake --build .