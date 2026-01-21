#!/bin/bash

# ###########################################
# #             1. Emscripten               #
# ###########################################

echo "Setup emscripten"

git submodule update --init --recursive
cd ./third_party/emsdk || exit 1

# Download and install the latest SDK tools.
./emsdk install latest

# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
./emsdk activate latest

# Source the environment script to set up PATH and other variables
source ./emsdk_env.sh

cd ../../ || exit 1

# ###########################################
# #           2. Build project              #
# ###########################################

echo "Build project"

rm -rf ./build
mkdir -p build/wasm
cd ./build/wasm || exit 1

echo "Prepare cmake for build"
emcmake cmake ../.. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Emscripten -DGSAND_ENABLE_TESTS=OFF -DGSAND_BUILD_DOCS=OFF -DGSAND_BUILD_SAMPLES=ON || {
    echo "failed to prepare cmake"
    read -p "Press any key to continue..."
    exit 1
}

echo "Build project"
cmake --build . --config Release || {
    echo "failed to build"
    read -p "Press any key to continue..."
    exit 1
}