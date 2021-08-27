clang clarawm.c -o clarawm -lX11 -O2 -Wall -Werror &&
if [[ "$*" == "run" ]]
then
    Xephyr -ac -screen 800x600 -br -reset -terminate :1 &
    sleep .5
    DISPLAY=:1 ./clarawm
fi
