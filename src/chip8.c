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

    srand(time(NULL));

    return chip8;
}

void c8_load_rom(Chip8 *chip8, const char *rom_path) {
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
                // 00E0 -> CLEAR SCREEN
                case 0x00E0:
                    INFO("Clearing Screen");
                    memset(chip8->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
                    chip8->needs_draw = true;
                    chip8->pc += 2;
                    break;

                // 00EE -> RETURN from SUBROUTINE
                case 0x00EE:
                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp];
                    chip8->pc += 2;
                    break;

                default:
                    INFO("Unknown Instruction 0x%04x", opcode);
                    chip8->pc += 2;
            }
            break;

        // 1NNN -> JUMP to NNN
        case 0x1000:
            chip8->pc = op_NNN(opcode);
            break;

        // 2NNN -> CALL SUBROUTINE at memory location NNN
        case 0x2000:
            chip8->stack[chip8->sp++] = chip8->pc;
            chip8->pc = op_NNN(opcode);
            break;

        // 3XNN -> SKIP next instruction if VX == NN
        case 0x3000:
            chip8->pc += 2;
            if(chip8->V[op_X(opcode)] == op_NN(opcode)) {
                chip8->pc += 2;
            }
            break;

        // 4XNN -> SKIP next instruction if VX != NN
        case 0x4000:
            chip8->pc += 2;
            if(chip8->V[op_X(opcode)] != op_NN(opcode)) {
                chip8->pc += 2;
            }
            break;

        // 5XY0 -> SKIP next instruction if VX == VY
        case 0x5000:
            chip8->pc += 2;
            if(chip8->V[op_X(opcode)] == chip8->V[op_Y(opcode)]) {
                chip8->pc += 2;
            }
            break;

        // 6XNN -> SET VX to NN
        case 0x6000:
            chip8->V[op_X(opcode)] = op_NN(opcode);
            chip8->pc += 2;
            break;

        // 7XNN -> ADD NN to VX
        case 0x7000:
            chip8->V[op_X(opcode)] += op_NN(opcode);
            chip8->pc += 2;
            break;

        case 0x8000:
            switch(opcode & 0x000F) {
                // 8XY0 -> SET VX to VY
                case 0x0000:
                    chip8->V[op_X(opcode)] = chip8->V[op_Y(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XY1 -> SET VX to VX OR VY
                case 0x0001:
                    chip8->V[op_X(opcode)] |= chip8->V[op_Y(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XY2 -> SET VX to VX AND VY
                case 0x0002:
                    chip8->V[op_X(opcode)] &= chip8->V[op_Y(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XY3 -> SET VX to VX XOR VY
                case 0x0003:
                    chip8->V[op_X(opcode)] ^= chip8->V[op_Y(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XY4 -> ADD VY to VX
                // FLAG set if carry
                case 0x0004:
                    chip8->V[op_X(opcode)] += chip8->V[op_Y(opcode)];
                    if(chip8->V[op_X(opcode)] > 0xFF - chip8->V[op_Y(opcode)]) {
                        chip8->V[0xF] = 1;
                    } else {
                        chip8->V[0xF] = 0;
                    }
                    chip8->pc += 2;
                    break;

                // 8XY5 -> SET VX to VX - VY
                // FLAG set if borrow
                case 0x0005:
                    if(chip8->V[op_X(opcode)] < chip8->V[op_Y(opcode)]) {
                        chip8->V[0xF] = 1;
                    } else {
                        chip8->V[0xF] = 0;
                    }
                    chip8->V[op_X(opcode)] -= chip8->V[op_Y(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XYE -> RIGHT SHIFT VX by one
                // SET VF to LSB of VX before shift
                case 0x0006:
                    chip8->V[0xF] = chip8->V[op_X(opcode)] & 0x1;
                    chip8->V[op_X(opcode)] >>= 1;
                    chip8->pc += 2;
                    break;

                // 8XY7 -> SET VX to VY - VX
                // FLAG set if borrow
                case 0x0007:
                    if(chip8->V[op_Y(opcode)] < chip8->V[op_X(opcode)]) {
                        chip8->V[0xF] = 1;
                    } else {
                        chip8->V[0xF] = 0;
                    }
                    chip8->V[op_X(opcode)] =
                        chip8->V[op_Y(opcode)] - chip8->V[op_X(opcode)];
                    chip8->pc += 2;
                    break;

                // 8XYE -> LEFT SHIFT VX by one
                // SET VF to MSB of VX before shift
                case 0x000E:
                    chip8->V[0xF] = chip8->V[op_X(opcode)] >> 7;
                    chip8->V[op_X(opcode)] <<= 1;
                    chip8->pc += 2;
                    break;

                default:
                    INFO("Unknown Instruction 0x%04x", opcode);
                    chip8->pc += 2;
            }
            break;

        // 9XY0 -> SKIP next instruction if VX != VY
        case 0x9000:
            chip8->pc += 2;
            if(chip8->V[op_X(opcode)] != chip8->V[op_Y(opcode)]) {
                chip8->pc += 2;
            }
            break;

        // ANNN -> SET I to NNN
        case 0xA000:
            chip8->I = op_NNN(opcode);
            chip8->pc += 2;
            break;

        // BNNN -> JUMP to NNN + V0
        case 0xB000:
            chip8->pc = op_NNN(opcode) + chip8->V[0];
            break;

        // CXNN -> SET Vx to a random number masked by NN
        case 0xC000:
            chip8->V[op_X(opcode)] = (rand() % 256) & op_X(opcode);
            chip8->pc += 2;
            break;

        // DXYN -> DRAW N px tall sprite from memory location in I
        // at (x, y) = (VX, VY)
        case 0xD000:
            INFO("Drawing Sprite");
            uint8_t width = 8;
            uint8_t height = op_N(opcode);
            uint8_t x_coord = chip8->V[op_X(opcode)];
            uint8_t y_coord = chip8->V[op_Y(opcode)];
            uint8_t sprite_row;

            // colission flag
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

        case 0xE000:
            switch(opcode & 0x00FF) {
                // EX9E -> SKIP next instruction if key in VX is pressed
                case 0x009E:
                    chip8->pc += 2;
                    if(chip8->keypad[chip8->V[op_X(opcode)]] != 0) {
                        chip8->pc += 2;
                    }
                    break;

                // EXA1 -> SKIP next instruction if key in VX is not pressed
                case 0x00A1:
                    chip8->pc += 2;
                    if(chip8->keypad[chip8->V[op_X(opcode)]] == 0) {
                        chip8->pc += 2;
                    }
                    break;

                default:
                    INFO("Unknown Instruction 0x%04x", opcode);
                    chip8->pc += 2;
            }
            break;

        case 0xF000:
            switch(opcode & 0x00FF) {
                // FX07 -> SET VX to value of delay timer
                case 0x0007:
                    chip8->V[op_X(opcode)] = chip8->delay_timer;
                    chip8->pc += 2;
                    break;

                // FX0A -> Wait for a key press and store it in VX
                case 0x000A:
                    bool key_pressed = false;

                    for(uint32_t i = 0; i < NUM_KEYS; i++) {
                        if(chip8->keypad[i]) {
                            chip8->V[op_X(opcode)] = i;
                            key_pressed = true;
                        }
                    }

                    // if no key is pressed we return without incrementing pc
                    // and do the same thing till a key press is registered
                    // to simulate a blocking instruction
                    if(!key_pressed) {
                        return;
                    }

                    chip8->pc += 2;
                    break;

                // FX15 -> SET delay timer to VX
                case 0x0015:
                    chip8->delay_timer = chip8->V[op_X(opcode)];
                    chip8->delay_cycles = 0;
                    chip8->pc += 2;
                    break;

                // FX18 -> SET sound timer to VX
                case 0x0018:
                    chip8->sound_timer = chip8->V[op_X(opcode)];
                    chip8->sound_cycles = 0;
                    chip8->pc += 2;
                    break;

                // FX1E -> ADD VX to I
                // FLAG set if range overflow (> 0xFFF)
                case 0x001E:
                    {
                        uint16_t res = chip8->I + chip8->V[op_X(opcode)];
                        chip8->I = res;
                        if(res > 0xFFF) {
                            chip8->V[0xF] = 1;
                        } else {
                            chip8->V[0xF] = 0;
                        }
                        chip8->pc += 2;
                        break;
                    }

                // FX29 -> SET I to addr of sprite in VX
                case 0x0029:
                    chip8->I = chip8->V[op_X(opcode)] * 0x05;
                    chip8->pc += 2;
                    break;

                // FX33 -> Store Binary coded decimal representation of VX
                // at addr I, I + 1, I + 2
                case 0x0033:
                    {
                        uint8_t target = op_X(opcode);
                        chip8->ram[chip8->I] = chip8->V[target] / 100;
                        chip8->ram[chip8->I + 1] = (chip8->V[target] / 10) % 10;
                        chip8->ram[chip8->I] = chip8->V[target] % 10;
                        chip8->pc += 2;
                    }
                    break;

                // FX55 -> Store V0 to VX in memory addr starting at I
                case 0x0055:
                    for(uint8_t i = 0; i <= op_X(opcode); i++) {
                        chip8->ram[chip8->I + 1] = chip8->V[i];
                    }
                    chip8->pc += 2;
                    break;

                // FX65 -> Store values at memory addrs starting at I in V0 to VX
                case 0x0655:
                    for(uint8_t i = 0; i <= op_X(opcode); i++) {
                        chip8->V[i] = chip8->ram[chip8->I + 1];
                    }
                    chip8->pc += 2;
                    break;

                default:
                    INFO("Unknown Instruction 0x%04x", opcode);
                    chip8->pc += 2;
            }
            break;

        default:
            // maybe SUPER-CHIP instruction
            UNIMPLEMENTED("Instruction 0x%04x", opcode);
    }
}

void c8_tick_timers(Chip8 *chip8) {
    if(chip8->delay_cycles >= 9) {
        if(chip8->delay_timer > 0) {
            chip8->delay_timer--;
        }
        chip8->delay_cycles = 0;
    }
    if(chip8->sound_cycles >= 9) {
        if(chip8->sound_timer > 0) {
            chip8->sound_timer--;
        }
        chip8->sound_cycles = 0;
    }
}
