@echo off

:: ###########################################
:: #             1. Emscripten               #
:: ###########################################

echo Setup emscripten
cd ./third_parties/emsdk

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
mkdir build\web
cd .\build\web

echo Prepare cmake for build
call emcmake cmake ..\.. -DPLATFORM=Web -DCMAKE_SYSTEM_NAME=Emscripten || (
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

echo Deploy on localhost:8000
cd ../../
emrun --no_browser --port 8000 ./demo/index.html :: # to run the server