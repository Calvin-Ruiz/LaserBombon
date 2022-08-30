mkdir build
cd build
:redo
clear
cmake ..
cmake --build . --config Release --parallel 8 && cmake --install . --config Release && exit
pause
goto redo
