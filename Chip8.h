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
static const uint32_t CHIP8_SCREEN_SCALE       = 15;
static const uint32_t CHIP8_INFO_REGION_HEIGHT = 300;

typedef struct CHIP8MEMORY {
    union {
        uint8_t*  ptr_8;
        uint16_t* ptr_16;
    };
    uint8_t* screen_buffer;

    uint16_t key_states;    // Each bit represents 1 - down, 0 - up for keys 0-9, A-F/a-f

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
    uint16_t  stack_ptr;
} Chip8_CPU;

// Used for diplaying information
#define NUM_GLYPHS ('~' - ' ')

typedef struct CHIP8FONTATLAS {
    TTF_Font*    font;
    SDL_Rect     glyph_rects[NUM_GLYPHS];
    SDL_Texture* texture;
    size_t       atlas_w;
    size_t       atlas_h;
} Chip8_FontAtlas;

typedef struct CHIP8DISPLAYCONTEXT {
    SDL_Renderer* renderer;
    TTF_Font*     font;
    SDL_Rect      dimensions;
    Chip8_FontAtlas* atlas;
} Chip8_DisplayContext;

// Only Chip8_Execute0xF for now returns a possible value (signal) if execution needs to
// be interrupted to wait for a key press, all others return 0, but only for uniformity
uint8_t Execute0xE(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t nn, uint8_t reg_x);
uint8_t Execute0xD(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t reg_y, uint8_t height);
uint8_t Execute0x0(Chip8_CPU* cpu, Chip8_Memory* mem, uint16_t nnn);
uint8_t Execute0x8(Chip8_CPU* cpu, uint8_t reg_x, uint8_t reg_y, uint8_t type);
uint8_t Execute0xF(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t nn);
uint8_t ExecInstruction(Chip8_CPU* cpu, Chip8_Memory* mem);
uint8_t min(uint8_t val, uint8_t min);
size_t  max(size_t  val, size_t max);

void Initialize(const uint8_t* program, const size_t size, Chip8_CPU* cpu, Chip8_Memory* mem);
void RunProgram(Chip8_CPU* cpu, Chip8_Memory* mem, SDL_Renderer* renderer);

uint16_t LittleToBigEndianU16(const uint16_t val);

// Display Related functions
void DisplayCPUAndMemoryContents   (Chip8_DisplayContext*, Chip8_CPU*, Chip8_Memory*);
void DisplaySurroundingInstructions(Chip8_DisplayContext*, Chip8_CPU*, Chip8_Memory*, int N);
void ConstructFontAtlas(Chip8_FontAtlas* atlas, TTF_Font* font, SDL_Renderer*);
void DrawString(const char* str, const int x, const int y, Chip8_FontAtlas* atlas, SDL_Renderer* r);

#endif
