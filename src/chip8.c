#include "chip8.h"

Chip8 *chip8_init() {
    Chip8 *chip8 = c8_calloc(1, sizeof *chip8);

    for(size_t i = 0; i < FONTSET_SIZE; i++) {
        chip8->ram[i] = FONTSET[i];
    }

    for(size_t i = 0; i < NUM_KEYS; i++) {
        chip8->keypad[i] = false;
    }

    chip8->pc = PROG_START_ADDR;
    chip8->needs_draw = false;

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

void c8_exec_instruction(Chip8 *chip8, bool dbg) {
    uint16_t opcode = (chip8->ram[chip8->pc] << 8) | chip8->ram[chip8->pc + 1];

    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0x00E0:
                    INFO("CLEAR SCREEN 0x00E0");
                    memset(chip8->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
                    chip8->needs_draw = true;
                    chip8->pc += 2;
                    break;

                default:
                    UNIMPLEMENTED("Instruction 0x%04x", opcode);
            }
            break;

        case 0x1000:
            INFO("JUMP 0x1NNN");
            chip8->pc = op_NNN(opcode);
            break;

        case 0x6000:
            {
                INFO("LOAD VX reg immediate 0x6XNN");
                uint8_t target_reg = op_X(opcode);
                chip8->V[target_reg] = op_NN(opcode);
                chip8->pc += 2;
                break;
            }

        case 0x7000:
            {
                INFO("ADD Vx reg immediate 0x7XNN");
                uint8_t target_reg = op_X(opcode);
                chip8->V[target_reg] += op_NN(opcode);
                chip8->pc += 2;
                break;
            }

        case 0xA000:
            INFO("LDI 0xANNN");
            chip8->I = op_NNN(opcode);
            chip8->pc += 2;
            break;

        case 0xD000:
            INFO("DRAW SPRITE 0xDXYN");
            uint8_t width = 8;
            uint8_t height = op_N(opcode);
            uint8_t x_coord = chip8->V[op_X(opcode)];
            uint8_t y_coord = chip8->V[op_Y(opcode)];
            uint8_t sprite_row;

            // set colission flag
            chip8->V[0xF] = 0;

            // TODO: add checks to ensure x & y are within bounds
            for(uint8_t curr_y = 0; curr_y < height; curr_y++) {
                sprite_row = chip8->ram[chip8->I + curr_y];
                for(uint8_t curr_x = 0; curr_x < width; curr_x++) {
                    if((sprite_row & (0x80 >> curr_x)) != 0) {
                        uint16_t draw_at = (x_coord + curr_x + ((y_coord + curr_y) * 64));
                        if(chip8->screen[draw_at] == 1) {
                            chip8->V[0xF] = 1;
                        }
                        chip8->screen[draw_at] ^= 1;
                    }
                }
            }

            chip8->needs_draw = true;
            chip8->pc += 2;

            break;

        default:
            UNIMPLEMENTED("Instruction 0x%04x", opcode);
    }
}
