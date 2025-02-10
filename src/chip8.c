#include "chip8.h"

Chip8 *chip8_init() {
    Chip8 *chip8 = c8_calloc(1, sizeof *chip8);

    for(size_t i = 0; i < FONTSET_SIZE; i++) {
        chip8->ram[i] = FONTSET[i];
    }

    for(size_t i = 0; i < NUM_KEYS; i++) {
        chip8->keypad[i] = false;
    }

    return chip8;
}

void chip8_load_rom(Chip8 *chip8, const char *rom_path) {
    FILE *rom = fopen(rom_path, "rb");
    if(!rom) {
        fprintf(stderr, "Couldn't open ROM file %s\n", rom_path);
        exit(1);
    }

    fseek(rom, 0, SEEK_END);
    size_t rom_len = ftell(rom);
    fseek(rom, 0, SEEK_SET);

    uint8_t *rom_buffer = c8_malloc(sizeof(uint8_t) * rom_len);
    size_t bytes_read = fread(rom_buffer, sizeof(uint8_t), rom_len, rom);
    if(bytes_read != rom_len) {
        fprintf(stderr, "Couldn't read ROM\n");
        exit(1);
    }

    if(rom_len > PROG_REGION_SIZE) {
        fprintf(stderr, "ROM is too big to fit into memory\n");
        exit(1);
    }

    for(size_t i = 0; i < rom_len; i++) {
        chip8->ram[i + PROG_START_ADDR] = rom_buffer[i];
    }

    fclose(rom);
    free(rom_buffer);
}
