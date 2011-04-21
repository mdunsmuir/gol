all : gol

gol : lib/libsprut.a lib/libsprutext.a 
	gcc -Wall -O3 src/gol.c -o gol -L ./lib -lsprut -lsprutext `sdl-config --cflags --libs`

lib/libsprut.a : lib/sprut.o
	ar rcs lib/libsprut.a lib/sprut.o

lib/sprut.o : lib
	gcc -c src/sprut.c -o lib/sprut.o `sdl-config --cflags --libs`

lib/libsprutext.a : lib/sprutext.o
	ar rcs lib/libsprutext.a lib/sprutext.o

lib/sprutext.o : lib
	gcc -c src/sprutext.c -o lib/sprutext.o `sdl-config --cflags --libs`

lib :
	mkdir lib

clean :
	rm -r gol lib/ *~ */*~