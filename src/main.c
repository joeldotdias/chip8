#include "chip8.h"

#include <time.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: chip8 <ROM file>\n");
        exit(1);
    }

    bool logging = argc > 2 && strcmp(argv[2], "DEBUG") == 0;

    Chip8 *chip8 = chip8_init();
    chip8_load_rom(chip8, argv[1]);

    SDL_Window *c8_window;
    SDL_Renderer *c8_renderer;
    SDL_Texture *c8_texture;
    uint32_t pixels[SCREEN_HEIGHT * SCREEN_WIDTH];
    memset(pixels, 0, sizeof(pixels));

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    c8_window = SDL_CreateWindow("CHIP 8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 1024, 512, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if(!c8_window) {
        fprintf(stderr, "Couldn't create SDL window: %s\n", SDL_GetError());
        exit(1);
    }

    c8_renderer = SDL_CreateRenderer(c8_window, -1, 0);
    if(!c8_renderer) {
        fprintf(stderr, "Couldn't create SDL renderer: %s\n", SDL_GetError());
        exit(1);
    }

    c8_texture =
        SDL_CreateTexture(c8_renderer, SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if(!c8_renderer) {
        fprintf(stderr, "Couldn't create SDL renderer: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_SetRenderDrawColor(c8_renderer, 0, 0, 0, 0);
    SDL_RenderClear(c8_renderer);
    SDL_RenderPresent(c8_renderer);

    SDL_Event e;
    bool quit = false;
    struct timespec ts = {0, 1200 * 1000};

    while(!quit) {
        c8_exec_instruction(chip8, logging);

        if(chip8->needs_draw) {
            for(int i = 0; i < 2048; i++) {
                uint8_t pixel = chip8->screen[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            SDL_UpdateTexture(c8_texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(c8_renderer);
            SDL_RenderCopy(c8_renderer, c8_texture, NULL, NULL);
            SDL_RenderPresent(c8_renderer);

            chip8->needs_draw = false;
        }

        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                }
            }

            if(e.type == SDL_QUIT) {
                quit = true;
            }
        }

        nanosleep(&ts, NULL);
    }

    SDL_DestroyWindow(c8_window);
    SDL_DestroyRenderer(c8_renderer);
    SDL_DestroyTexture(c8_texture);

    free(chip8);

    return 0;
}
