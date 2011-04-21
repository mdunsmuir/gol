all : gol

gol : 
	gcc -Wall -O3 gol.c -o gol `sdl-config --cflags --libs`

clean :
	rm gol