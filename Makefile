all:
	gcc -Wall utplay.c `sdl2-config --cflags` `sdl2-config --libs` -lSDL2_mixer -o utplay
