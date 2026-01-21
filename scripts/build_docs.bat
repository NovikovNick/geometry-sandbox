@echo off

:: ###########################################
:: #           Build docs                 #
:: ###########################################

echo Build docs
call cmake --build .\build\x64-Debug --target docs || (
    echo failed to build docs
    pause
    exit /b 1
)
