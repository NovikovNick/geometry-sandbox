@echo off

:: ###########################################
:: #           Build project                 #
:: ###########################################

echo Prepare cmake for build
mkdir build\win
call cmake -S . -B .\build\win || (
    echo failed to prepare cmake
    pause
    exit /b 1
)

echo Build project    
call cmake --build .\build\win --config Release || (
    echo failed to build project
    pause
    exit /b 1
)

