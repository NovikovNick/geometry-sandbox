@echo off

:: ###########################################
:: #             1. Emscripten               #
:: ###########################################

echo Setup emscripten

git submodule update --init --recursive
cd ./third_party/emsdk

:: # Download and install the latest SDK tools.
call ./emsdk install latest

:: # Make the "latest" SDK "active" for the current user. (writes .emscripten file)
call ./emsdk activate latest

cd ../../

:: ###########################################
:: #           2. Build project              #
:: ###########################################

echo Build project

rmdir /s /q ./build
mkdir build\wasm
cd .\build\wasm

echo Prepare cmake for build
call emcmake cmake ..\.. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Emscripten -DGSAND_ENABLE_TESTS=OFF -DGSAND_BUILD_DOCS=OFF -DGSAND_BUILD_SAMPLES=ON || (
    echo failed to prepare cmake
    pause
    exit /b 1
)

echo Build project
call cmake --build . --config Release || (
    echo failed to build
    pause
    exit /b 1
)
