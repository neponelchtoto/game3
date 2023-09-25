rabbit: rabbit.o
	gcc -o rabbit rabbit.o `pkg-config --libs sdl SDL_ttf`

rabbit.o: rabbit.c
	gcc -c -Wno-switch rabbit.c `pkg-config --cflags sdl SDL_ttf` -O0 -g

clean:
	rm rabbit *.o
