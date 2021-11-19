#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static const uint32_t CHIP8_TIMER_PERIOD       = 1000 / 60; // 60hz to decrement delay timer and sound timer
static const uint32_t CHIP8_SCREEN_WIDTH       = 64;
static const uint32_t CHIP8_SCREEN_HEIGHT      = 32;
static const uint32_t CHIP8_SCREEN_SCALE       = 10;
static const uint32_t CHIP8_INFO_REGION_HEIGHT = 300;

typedef struct CHIP8MEMORY {
    uint8_t*  main_memory;
    uint8_t*  screen_buffer;
    uint16_t* ptr_stack;

    uint16_t  key_states;    // Each bit represents 1 - down, 0 - up for keys 0-9, A-F/a-f

    size_t   memory_size;
    size_t   stack_size;
    size_t   screen_w;
    size_t   screen_h;
} Chip8_Memory;

typedef struct CHIP8CPU {
    uint8_t   VX[16];       // General purpose, V0 - VF;
    uint16_t  PC;           // Program counter
    uint16_t  CIR;          // current instruction register
    uint16_t  I;            // Memory operand/pointer, referred to as 'I' in Chip8 docs
    uint8_t   delay_timer;  
    uint8_t   sound_timer; 
} Chip8_CPU;

// Used for diplaying information
typedef struct CHIP8DISPLAYCONTEXT {
    SDL_Renderer* renderer;
    TTF_Font*     font;
    SDL_Rect      dimensions;
} Chip8_DisplayContext;

// Only Chip8_Execute0xF for now returns a possible value (signal) if execution needs to
// be interrupted to wait for a key press, all others return 0, but only for uniformity
uint8_t Chip8_Execute0xE(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t nn, uint8_t reg_x);
uint8_t Chip8_Execute0xD(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t reg_y, uint8_t height);
uint8_t Chip8_Execute0x0(Chip8_CPU* cpu, Chip8_Memory* mem, uint16_t nnn);
uint8_t Chip8_Execute0x8(Chip8_CPU* cpu, uint8_t reg_x, uint8_t reg_y, uint8_t type);
uint8_t Chip8_Execute0xF(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t nn);
uint8_t Chip8_ExecInstruction(Chip8_CPU* cpu, Chip8_Memory* mem);

void Chip8_Initialize(const uint8_t* program, const size_t size, Chip8_CPU* cpu, Chip8_Memory* mem);
void Chip8_RunProgram(Chip8_CPU* cpu, Chip8_Memory* mem, SDL_Renderer* renderer);
uint16_t Chip8_u16_little_to_big_endian(const uint16_t val);
uint8_t min(uint8_t val, uint8_t min);

// Display Related functions
void Chip8_Display_Surrounding_N_Instructions(Chip8_DisplayContext* ctx, Chip8_CPU *cpu, Chip8_Memory *mem, int N);

#endif
