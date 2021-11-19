#include "Chip8.h"

uint8_t Chip8_Execute0xF(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t nn)
{
    switch(nn) {
        case 0x07: cpu->VX[reg_x] = cpu->delay_timer; break;
        case 0x0a: return reg_x; break;
        case 0x15: cpu->delay_timer = cpu->VX[reg_x]; break;
        case 0x18: cpu->sound_timer = cpu->VX[reg_x]; break;
        case 0x1e: cpu->I += cpu->VX[reg_x]; break;
        case 0x29: cpu->I = (cpu->VX[reg_x] * 5); break;
        case 0x33: {
            mem->main_memory[cpu->I + 0] = cpu->VX[reg_x] / 100;
            mem->main_memory[cpu->I + 1] = cpu->VX[reg_x] / 10 % 10;
            mem->main_memory[cpu->I + 2] = cpu->VX[reg_x] % 10;
        } break;
        case 0x55: memcpy(mem->main_memory + cpu->I, cpu->VX, reg_x + 1); break;
        case 0x65: memcpy(cpu->VX, mem->main_memory + cpu->I, reg_x + 1); break;
    }
    return 0xFF;
}

uint8_t Chip8_Execute0xE(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t nn, uint8_t reg_x)
{
    switch(nn) {
        case 0x9e: { if(mem->key_states & (0x8000 >> reg_x)) cpu->PC += 2; } break;
        case 0xa1: { if((mem->key_states & (0x8000 >> reg_x)) == 0) cpu->PC += 2; } break;
    }
    return 0;
}

uint8_t Chip8_Execute0xD(Chip8_CPU* cpu, Chip8_Memory* mem, uint8_t reg_x, uint8_t reg_y, uint8_t height)
{
    uint16_t pixel_offset = (cpu->VX[reg_y] % 32) * mem->screen_w + (cpu->VX[reg_x] % 64);
    uint8_t* sprite_addr  = mem->main_memory + cpu->I;
    uint8_t* screen_addr  = mem->screen_buffer + pixel_offset;

    uint8_t collision = 0x00;
    for(uint8_t row_idx = 0; row_idx < height; row_idx++) {
        for(uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
            uint8_t sprite_pixel = ((*sprite_addr) & (0x80 >> bit_idx)) != 0;

            collision = (*screen_addr && sprite_pixel) | collision;
            *screen_addr ^= sprite_pixel;
            screen_addr++;
        }
        screen_addr += mem->screen_w - 8;
        sprite_addr++;
    }
    cpu->VX[0x0F] = collision;
    return 0;
}

uint8_t Chip8_Execute0x8(Chip8_CPU* cpu, uint8_t reg_x, uint8_t reg_y, uint8_t type) 
{
    switch(type) {
        case 0x00: cpu->VX[reg_x]  = cpu->VX[reg_y]; break;
        case 0x01: cpu->VX[reg_x] |= cpu->VX[reg_y]; break;
        case 0x02: cpu->VX[reg_x] &= cpu->VX[reg_y]; break;
        case 0x03: cpu->VX[reg_x] ^= cpu->VX[reg_y]; break;
        case 0x04: {
            cpu->VX[0x0F] = (cpu->VX[reg_y] > (255 - cpu->VX[reg_x]));
            cpu->VX[reg_x] += cpu->VX[reg_y]; 
        } break;
        case 0x05: {
            cpu->VX[0x0F]  = (cpu->VX[reg_x] > cpu->VX[reg_y]);
            cpu->VX[reg_x] -= cpu->VX[reg_y]; 
        } break;
        case 0x06: {
            cpu->VX[0x0F]  = (cpu->VX[reg_x] & 1);
            cpu->VX[reg_x] = (cpu->VX[reg_x] >> 1);
        } break;
        case 0x07: {
            cpu->VX[0x0F]  = (cpu->VX[reg_y] > cpu->VX[reg_x]);
            cpu->VX[reg_x] = (cpu->VX[reg_y] - cpu->VX[reg_x]);
        } break;
        case 0x0E: {
            cpu->VX[0x0F]  = (cpu->VX[reg_x] & 0x80) >> 7;
            cpu->VX[reg_x] = (cpu->VX[reg_x] << 1);
        } break;
    }
    return 0;
}

uint8_t Chip8_Execute0x0(Chip8_CPU* cpu, Chip8_Memory* mem, uint16_t nnn)
{
    switch(nnn) {
        case 0x0E0: memset(mem->screen_buffer, 0x00, mem->screen_w * mem->screen_h); break;
        case 0x0EE: cpu->PC = *(mem->ptr_stack++); break;
    }
    return 0;
}

uint8_t Chip8_ExecInstruction(Chip8_CPU* cpu, Chip8_Memory* mem)
{
    // 'Decode'
    uint16_t opcode = (cpu->CIR & 0xF000) >> 12; 
    uint16_t nnn    = (cpu->CIR & 0x0FFF);       // Lower 3 nibbles, typically an address operand
    uint8_t  nn     = (cpu->CIR & 0x00FF);       // Lower byte, typically a byte literal
    uint8_t  reg_x  = (cpu->CIR & 0x0F00) >> 8;  // typically a register index
    uint8_t  reg_y  = (cpu->CIR & 0x00F0) >> 4;  // typically a register index
    uint8_t  optype = (cpu->CIR & 0x000F);       // nibble which differentiates inst's with same opcode

    uint8_t wait_key = 0xFF;

    // Execute
    switch(opcode) {
        case 0x0: Chip8_Execute0x0(cpu, mem, nnn); break; 
        case 0x1: cpu->PC = nnn; break;
        case 0x2: *(--mem->ptr_stack) = cpu->PC; cpu->PC = nnn; break;
        case 0x3: cpu->PC += (2 * (cpu->VX[reg_x] == nn)); break;
        case 0x4: cpu->PC += (2 * (cpu->VX[reg_x] != nn)); break;
        case 0x5: cpu->PC += (2 * (cpu->VX[reg_x] == cpu->VX[reg_y])); break;
        case 0x6: cpu->VX[reg_x]  = nn; break;
        case 0x7: cpu->VX[reg_x] += nn; break;
        case 0x8: Chip8_Execute0x8(cpu, reg_x, reg_y, optype); break;
        case 0x9: cpu->PC += (2 * (cpu->VX[reg_x] != cpu->VX[reg_y])); break;
        case 0xa: cpu->I = nnn; break;
        case 0xb: cpu->PC = nnn + cpu->VX[0]; break;
        case 0xc: cpu->VX[reg_x] = (rand() % 255) & nn; break;
        case 0xd: Chip8_Execute0xD(cpu, mem, reg_x, reg_y, optype); break;
        case 0xe: Chip8_Execute0xE(cpu, mem, nn, reg_x); break;
        case 0xf: wait_key = Chip8_Execute0xF(cpu, mem, reg_x, nn); break;
        default: break;
    }
    return wait_key;
}


void Chip8_RunProgram(Chip8_CPU* cpu, Chip8_Memory* mem, SDL_Renderer* renderer)
{
    TTF_Font* display_font = TTF_OpenFont("data/Consolas.ttf", 20);

    if(display_font == NULL) {
        SDL_Log("Error failed to open TTF_Font: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* screen_texture = SDL_CreateTexture(renderer,
                                                    SDL_PIXELFORMAT_RGBA8888,
                                                    SDL_TEXTUREACCESS_TARGET,
                                                    mem->screen_w, mem->screen_h);
    if(screen_texture == NULL) {
        SDL_Log("Error failed to create SDL_Texture: %s\n", SDL_GetError());
        return;
    }

    SDL_Texture* info_texture = SDL_CreateTexture(renderer,
                                                  SDL_PIXELFORMAT_RGBA8888,
                                                  SDL_TEXTUREACCESS_TARGET,
                                                  mem->screen_w * CHIP8_SCREEN_SCALE, 
                                                  CHIP8_INFO_REGION_HEIGHT);
    if(info_texture == NULL) {
        SDL_Log("Error failed to create SDL_Texture: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect display_region;
    display_region.x = 0;
    display_region.y = 0;
    display_region.w = mem->screen_w * CHIP8_SCREEN_SCALE;
    display_region.h = mem->screen_h * CHIP8_SCREEN_SCALE;

    SDL_Rect info_region, info_region_dest;
    info_region.x = 0;
    info_region.y = 0;
    info_region.w = display_region.w;
    info_region.h = CHIP8_INFO_REGION_HEIGHT;

    info_region_dest   = info_region;
    info_region_dest.y = display_region.h;
    info_region_dest.h = CHIP8_INFO_REGION_HEIGHT;

    SDL_Event event;
    bool is_running   = true;
    bool is_debugging = true;
    uint32_t total_elapsed = 0;

    while(is_running) {
        uint32_t t_frame_start = SDL_GetTicks();

        // Fetch
        uint16_t next_inst = Chip8_u16_little_to_big_endian(*((uint16_t*)(mem->main_memory + cpu->PC)));
        cpu->CIR = next_inst;
        cpu->PC += 2;

        int16_t signal = Chip8_ExecInstruction(cpu, mem);

        while(SDL_PollEvent(&event) != 0 || signal != 0xFF) {
            if(event.type == SDL_QUIT) is_running = false;
            if((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
                int keycode = event.key.keysym.sym;
                int key_idx = (keycode >= 'a' && keycode <= 'f') ? (keycode - 'a' + 10) :
                              (keycode >= 'A' && keycode <= 'F') ? (keycode - 'A' + 10) :
                              (keycode >= '0' && keycode <= '9') ? (keycode - '0') : -1;

                if(key_idx >= 0 && key_idx <= 15) {
                    if( event.type == SDL_KEYDOWN )
                        mem->key_states |=  (0x8000 >> key_idx);
                    else
                        mem->key_states &= ~(0x8000 >> key_idx);
                }

                if(signal != 0xFF) {
                    cpu->VX[signal] = key_idx;
                    signal = 0xFF;
                }
            }
        }

        // Display the contents of the screen buffer
        SDL_SetRenderTarget(renderer, screen_texture);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        for(size_t y = 0; y < mem->screen_h; y++) {
            for(size_t x = 0; x < mem->screen_w; x++) {
                uint8_t pixel = mem->screen_buffer[ y * mem->screen_w + x ];

                if(pixel == 0) continue;

                SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // Display info
        SDL_SetRenderTarget(renderer, info_texture);
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_Rect inst_list_region = {};
        inst_list_region.x = 0;
        inst_list_region.y = 0;
        inst_list_region.w = info_region.w * 6 / 10; 
        inst_list_region.h = info_region.h;

        {
            Chip8_DisplayContext ctx;
            ctx.renderer   = renderer;
            ctx.font       = display_font;
            ctx.dimensions = inst_list_region;

            //Chip8_Display_Surrounding_N_Instructions(&ctx, cpu, mem, 7);
        }

        SDL_Rect data_info_region = {};
        data_info_region.x = inst_list_region.w;
        data_info_region.y = 0;
        data_info_region.w = info_region.w - inst_list_region.w;
        data_info_region.h = info_region.h;

        SDL_RenderDrawRect(renderer, &inst_list_region);
        SDL_RenderDrawRect(renderer, &data_info_region);
        SDL_SetRenderTarget(renderer, NULL);

        SDL_RenderCopy(renderer, screen_texture, NULL, &display_region);
        SDL_RenderCopy(renderer, info_texture, &info_region, &info_region_dest);
        SDL_RenderPresent(renderer);

        uint32_t t_frame_end = SDL_GetTicks();
        uint32_t t_frame_dur = t_frame_end - t_frame_start;
        total_elapsed += t_frame_dur;

        if(total_elapsed >= CHIP8_TIMER_PERIOD) {
            if(cpu->delay_timer)
                cpu->delay_timer = min(0, cpu->delay_timer - floor(total_elapsed / CHIP8_TIMER_PERIOD));

            if(cpu->sound_timer) {
                cpu->sound_timer = min(0, cpu->delay_timer - floor(total_elapsed / CHIP8_TIMER_PERIOD));
            }
            total_elapsed = 0;
        }
    }
    SDL_DestroyTexture(screen_texture);
    SDL_DestroyTexture(info_texture);
}

void Chip8_Initialize(const uint8_t* program, const size_t size, Chip8_CPU* cpu, Chip8_Memory* mem)
{
    // Initialize memory
    mem->memory_size  = 4096;
    mem->screen_w     = CHIP8_SCREEN_WIDTH;
    mem->screen_h     = CHIP8_SCREEN_HEIGHT;
    mem->stack_size   = 24; // 2 bytes per address, means 12 successive function calls

    // Allocate the memory
    mem->ptr_stack     = (uint16_t*)(mem->main_memory + mem->memory_size);

    memset(mem->main_memory,   0x00, mem->memory_size);
    memset(mem->screen_buffer, 0x00, mem->screen_w * mem->screen_h);

    // Write the system font
    uint8_t system_font[16][5] = {
        { 0xF0, 0x90, 0x90, 0x90, 0xF0 }, { 0x20, 0x60, 0x20, 0x20, 0x70 }, // 0, 1
        { 0xF0, 0x10, 0xF0, 0x80, 0xF0 }, { 0xF0, 0x10, 0xF0, 0x10, 0xF0 }, // 2, 3 
        { 0x90, 0x90, 0xF0, 0x10, 0x10 }, { 0xF0, 0x80, 0xF0, 0x10, 0xF0 }, // 4, 5
        { 0xF0, 0x80, 0xF0, 0x90, 0xF0 }, { 0xF0, 0x10, 0x20, 0x40, 0x40 }, // 6, 7
        { 0xF0, 0x90, 0xF0, 0x90, 0xF0 }, { 0xF0, 0x90, 0xF0, 0x10, 0xF0 }, // 8, 9
        { 0xF0, 0x90, 0xF0, 0x90, 0x90 }, { 0xE0, 0x90, 0xE0, 0x90, 0xE0 }, // A, B
        { 0xF0, 0x80, 0x80, 0x80, 0xF0 }, { 0xE0, 0x90, 0x90, 0x90, 0xE0 }, // C, D
        { 0xF0, 0x80, 0xF0, 0x80, 0xF0 }, { 0xF0, 0x80, 0xF0, 0x80, 0x80 }, // E, F
    };
    memcpy(mem->main_memory, system_font, 16 * 5);

    // Chip-8 Programs are specified to start at 0x200
    memcpy(mem->main_memory + 0x200, program, size);

    // Initialize the CPU
    memset(cpu->VX, 0x00, 16);   // zero-initialize the registers
    cpu->PC = 0x200;
    cpu->I  = 0x00;
    cpu->delay_timer = 0x00;
}

inline uint16_t Chip8_u16_little_to_big_endian(const uint16_t val)
{
    return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
}

uint8_t min(uint8_t val, uint8_t min)
{
    return val < min ? min : val;
}

// TODO: Cleanup
void Chip8_Concat_Disassembly(char* buffer, size_t n, uint16_t instr)
{
    char instr_disassembly[1024] = {};

    // 'Decode'
    uint16_t opcode = (instr & 0xF000) >> 12; 
    uint16_t nnn    = (instr & 0x0FFF);       // Lower 3 nibbles, typically an address operand
    uint8_t  nn     = (instr & 0x00FF);       // Lower byte, typically a byte literal
    uint8_t  reg_x  = (instr & 0x0F00) >> 8;  // typically a register index
    uint8_t  reg_y  = (instr & 0x00F0) >> 4;  // typically a register index
    uint8_t  optype = (instr & 0x000F);       // nibble which differentiates inst's with same opcode

    // Execute
    switch(opcode) {
        case 0x0: {
            switch(nnn) {
                case 0x0E0: snprintf(instr_disassembly, n, "cls"); break;
                case 0x0EE: snprintf(instr_disassembly, n, "ret"); break;
            }
        } break; 
        case 0x1: snprintf(instr_disassembly, n, "jmp 0x%x", nnn);                 break;
        case 0x2: snprintf(instr_disassembly, n, "call 0x%x", nnn);                break;
        case 0x3: snprintf(instr_disassembly, n, "skipeq v%x, 0x%x", reg_x, nn);   break;
        case 0x4: snprintf(instr_disassembly, n, "skipneq v%x, 0x%x", reg_x, nn);  break;
        case 0x5: snprintf(instr_disassembly, n, "skipeq v%x, v%x", reg_x, reg_y); break;
        case 0x6: snprintf(instr_disassembly, n, "mov v%x, %x", reg_x, nn);        break;
        case 0x7: snprintf(instr_disassembly, n, "add v%x, v%x", reg_x, reg_y);    break;
        case 0x8: {
            switch(optype) {
                case 0x00: snprintf(instr_disassembly, n, "mov v%x, v%x", reg_x, reg_y); break;
                case 0x01: snprintf(instr_disassembly, n, "or v%x, v%x", reg_x, reg_y); break;
                case 0x02: snprintf(instr_disassembly, n, "and v%x, v%x", reg_x, reg_y); break;
                case 0x03: snprintf(instr_disassembly, n, "xor v%x, v%x", reg_x, reg_y); break;
                case 0x04: snprintf(instr_disassembly, n, "addsc v%x, v%x", reg_x, reg_y); break;
                case 0x05: snprintf(instr_disassembly, n, "subsc v%x, v%x", reg_x, reg_y); break;
                case 0x06: snprintf(instr_disassembly, n, "shr v%x", reg_x); break;
                case 0x07: snprintf(instr_disassembly, n, "subn v%x, v%x", reg_x, reg_y); break;
                case 0x0E: snprintf(instr_disassembly, n, "shl v%x", reg_x); break;
            }
        } break;
        case 0x9: snprintf(instr_disassembly, n, "skipneq v%x, v%x", reg_x, reg_y);  break;
        case 0xa: snprintf(instr_disassembly, n, "mov I, %x", nnn);  break;
        case 0xb: snprintf(instr_disassembly, n, "jmp v0, %x", nnn);  break;
        case 0xc: snprintf(instr_disassembly, n, "rnd v%x, %x", reg_x, nn);  break;
        case 0xd: snprintf(instr_disassembly, n, "drw v%x, v%x, %x", reg_x, reg_y, optype);  break;
        case 0xe: {
            switch(nn) {
                case 0x9e: snprintf(instr_disassembly, n, "skp v%x", reg_x);  break;
                case 0xa1: snprintf(instr_disassembly, n, "sknp v%x", reg_x); break;
            }
        }  break;
        case 0xf: {
            switch(nn) {
                case 0x07: snprintf(instr_disassembly, n, "mov dt, %x", reg_x); break;
                case 0x0a: snprintf(instr_disassembly, n, "intk v%x", reg_x); break;
                case 0x15: snprintf(instr_disassembly, n, "mov dt, v%x", reg_x); break;
                case 0x18: snprintf(instr_disassembly, n, "mov st, v%x", reg_x); break;
                case 0x1e: snprintf(instr_disassembly, n, "add I, v%x", reg_x); break;
                case 0x29: snprintf(instr_disassembly, n, "lds v%x", reg_x); break;
                case 0x33: snprintf(instr_disassembly, n, "bcd v%x", reg_x); break;
                case 0x55: snprintf(instr_disassembly, n, "mov [I], v%x", reg_x); break;
                case 0x65: snprintf(instr_disassembly, n, "mov v%x, [I]", reg_x); break;
            }
        } break;
        default: break;
    }
    strncat (buffer, instr_disassembly, n);
}

void Chip8_Display_Surrounding_N_Instructions(Chip8_DisplayContext* ctx, Chip8_CPU *cpu, 
                                              Chip8_Memory *mem, int N) 
{
    SDL_Color c = { 0xff, 0xff, 0xff, 0xff };
    char output_line[1024];

    SDL_SetRenderDrawColor(ctx->renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(ctx->renderer);

    SDL_SetRenderDrawColor(ctx->renderer, 0xff, 0xff, 0xff, 0xff);
    for(int idx = -N; idx < N + 1; idx++) {
        uint16_t* inst_addr = (uint16_t*)(mem->main_memory + cpu->PC) + idx;
        uint16_t  instr     = Chip8_u16_little_to_big_endian(*inst_addr);

        int n = snprintf(output_line, 1024, 
                         "%s %p 0x%04x ", idx == -1 ? ">" : " ", 
                         inst_addr, instr);

        Chip8_Concat_Disassembly(output_line, 1024 - n, instr);

        SDL_Surface* text_surface = TTF_RenderText_Solid(ctx->font, output_line, c);

        if(text_surface != NULL) {
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(ctx->renderer, text_surface);

            SDL_Rect text_rect;
            text_rect.x = 10;
            text_rect.y = 10 + ((idx + N) * 19);
            text_rect.w = text_surface->w;
            text_rect.h = text_surface->h;

            if(text_texture != NULL) {
                SDL_RenderCopy(ctx->renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

