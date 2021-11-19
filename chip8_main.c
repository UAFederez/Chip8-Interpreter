#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_ttf.h>

#include "Chip8.h"

int main(int, char**)
{
    // SDL-Specific Initialization
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "SDL_Init Failure", "SDL Initialization was unsuccessful.", NULL);
        return -1;
    }

    if(TTF_Init() == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "TTF_Init Failure", "TTF Initialization was unsuccessful.", NULL);
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Chip-8 Interpreter",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          CHIP8_SCREEN_WIDTH  * CHIP8_SCREEN_SCALE,
                                          CHIP8_SCREEN_HEIGHT * CHIP8_SCREEN_SCALE + CHIP8_INFO_REGION_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if(window == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "SDL_Window Failure", "SDL_Window creation was unsuccessful.", NULL);
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(renderer == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "SDL_Renderer Failure", "SDL_Renderer creation was unsuccessful.", NULL);
        return -1;
    }

    // Load the program ROM
    FILE* rom_file = fopen("ROMS/Sirpinski2.ch8", "rb");

    if(rom_file == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
                                 "ROM Read Failure", "Failed to open the ROM file.", NULL);
        return -1;
    }

    fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);

    uint8_t* program = (uint8_t*) malloc(rom_size);
    fread (program, rom_size, 1, rom_file);
    fclose(rom_file);

    Chip8_CPU   cpu     = {};
    Chip8_Memory memory = {};

    // These are small enough that they can fit in the stack
    uint8_t stack_memory [4096];
    uint8_t stack_screen_buffer [CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT];

    memory.main_memory   = stack_memory;
    memory.screen_buffer = stack_screen_buffer;

    Chip8_Initialize(program, rom_size, &cpu, &memory);
    Chip8_RunProgram(&cpu, &memory, renderer);

    // Cleanup
    free(program);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
