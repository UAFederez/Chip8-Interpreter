CC  = gcc
SRC = chip8.c \
	  chip8_main.c

CF  = -Wall -Wextra -g
ID  = -IC:/clib/sdl2/SDL2/include
LD  = -LC:/SDL2/lib -LC:/clib/sdl2/SDL2_ttf/lib
LF  = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

all: $(SRC)
	$(CC) $(ID) $(LD) $^ $(CF) $(LF)

