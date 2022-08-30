mkdir build
cd build
:redo
clear
cmake ..
cmake --build . --config Debug --parallel 8 && cmake --install . --config Debug && exit
pause
goto redo
