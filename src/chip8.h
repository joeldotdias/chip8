#ifndef CHIP8_H
#define CHIP8_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"

#define RAM_SIZE 4096
#define STACK_SIZE 16
#define NUM_GPRS 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_KEYS 16
#define FONTSET_SIZE 80

#define PROG_START_ADDR 0x200
#define PROG_END_ADDR 0x1000
#define PROG_REGION_SIZE (PROG_END_ADDR - PROG_START_ADDR)

typedef struct Chip8 {
    uint8_t ram[RAM_SIZE]; // 4k of memory
    uint16_t stack[STACK_SIZE];

    // registers
    uint8_t V[NUM_GPRS]; // GPR -> [V0 - V14], Carry -> V15
    uint16_t I;          // index register
    uint16_t pc;         // program counter
    uint16_t opcode;     // current opcode

    uint8_t delay_timer;
    uint8_t sound_timer;

    // graphics buffer
    uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];

    // keypad
    bool keypad[NUM_KEYS];
} Chip8;

static const uint8_t FONTSET[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static const uint8_t KEYMAP[NUM_KEYS] = {
    SDLK_x, // 0
    SDLK_1, // 1
    SDLK_2, // 2
    SDLK_3, // 3
    SDLK_q, // 4
    SDLK_w, // 5
    SDLK_e, // 6
    SDLK_a, // 7
    SDLK_s, // 8
    SDLK_d, // 9
    SDLK_z, // A
    SDLK_c, // B
    SDLK_4, // C
    SDLK_r, // D
    SDLK_f, // E
    SDLK_v  // F
};

Chip8 *chip8_init();
void chip8_load_rom(Chip8 *chip8, const char *rom_path);

#define c8_malloc(size) c8_malloc(size, __FILE__, __LINE__)
#define c8_calloc(nmemb, size) c8_calloc(nmemb, size, __FILE__, __LINE__)

#endif
