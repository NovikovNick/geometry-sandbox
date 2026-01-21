@echo off

:: ###########################################
:: #                Deploy                   #
:: ###########################################

echo Deploy on localhost:8000
call ./third_party/emsdk/emsdk_env.bat
emrun --port 8000 ./docs/html