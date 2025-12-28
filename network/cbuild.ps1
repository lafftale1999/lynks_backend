$ErrorActionPreference = "Stop"

$BUILD_DIR = "build"

cmake -S . -B $BUILD_DIR -G "MinGW Makefiles"
cmake --build $BUILD_DIR -j