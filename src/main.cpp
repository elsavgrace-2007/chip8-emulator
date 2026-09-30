#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <cstdint>
#include <iostream>

const int SCALE = 10; // Each pixel is 10x10 screen pixels
const int WIDTH = 64*SCALE;
const int HEIGHT = 32*SCALE;

// Keyboard mapping
uint8_t keymap[16] = {
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

void audio_callback(void* userdata, uint8_t* stream, int len){
    static uint32_t sample_index = 0;
    int16_t* audio_buffer = (int16_t*) stream;
    int samples = len/2;

    bool* beeping = (bool*) userdata;
    for(int i=0; i<samples; i++){
        if(*beeping){
            // Generating 440Hz sqaure wave
            int16_t value = ((sample_index++ / 100) % 2) ? 3000 : -3000;
            audio_buffer[i] = value;
        }
        else{
            audio_buffer[i] = 0; // Silence
            sample_index = 0;
        }
    }
}

void draw_graphics(SDL_Renderer* renderer, Chip8& chip8, int color_scheme){
    int window_width;
int window_height;

SDL_GetRendererOutputSize(renderer, &window_width, &window_height);

int pixel_width = window_width / 64;
int pixel_height = window_height / 32;
    // Clear screen
    if(color_scheme == 0){
    // Classic
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}
else if(color_scheme == 1){
    // Neon Cyan
    SDL_SetRenderDrawColor(renderer, 5, 10, 30, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
}
else if(color_scheme == 2){
    // Neon Matrix
    SDL_SetRenderDrawColor(renderer, 2, 20, 8, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 57, 255, 20, 255);
}
else if(color_scheme == 3){
    // Neon Pink
    SDL_SetRenderDrawColor(renderer, 30, 5, 25, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 20, 147, 255);
}
else if(color_scheme == 4){
    // Neon Orange
    SDL_SetRenderDrawColor(renderer, 35, 8, 2, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
}
else if(color_scheme == 5){
    // Electric Blue
    SDL_SetRenderDrawColor(renderer, 3, 5, 35, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
}
else if(color_scheme == 6){
    // Neon Purple
    SDL_SetRenderDrawColor(renderer, 20, 3, 35, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 191, 0, 255, 255);
}
else if(color_scheme == 7){
    // Hot Pink
    SDL_SetRenderDrawColor(renderer, 40, 3, 20, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 102, 255);
}
    for(int y=0; y<32; y++){
        for(int x=0; x<64; x++){
            if(chip8.display[x + (y*64)] == 1){
                SDL_Rect rect = {
    x * pixel_width,
    (31-y) * pixel_height,
    pixel_width,
    pixel_height
};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void handle_input(Chip8& chip8, bool& running, int& cycles_per_frame, int& color_scheme, bool& paused, bool& muted, SDL_Window* window){
    SDL_Event event;
 
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT) running = false;
        if(event.type == SDL_KEYDOWN){
            if(event.key.keysym.sym == SDLK_m){
    muted = !muted;
    std::cout << (muted ? "Sound muted" : "Sound unmuted") << std::endl;
}
            if(event.key.keysym.sym == SDLK_ESCAPE) running = false;
  if(event.key.keysym.sym == SDLK_F11){
    Uint32 flags = SDL_GetWindowFlags(window);

    if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP){
        if(SDL_SetWindowFullscreen(window, 0) != 0){
            std::cerr << "Failed to exit fullscreen: "
                      << SDL_GetError() << std::endl;
        }
    } else {
        if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0){
            std::cerr << "Failed to enter fullscreen: "
                      << SDL_GetError() << std::endl;
        }
    }
}

            if(event.key.keysym.sym == SDLK_F7){
    chip8.cosmo_polo_telemetry();
}
            if(event.key.keysym.sym == SDLK_EQUALS){
    cycles_per_frame += 2;
}

if(event.key.keysym.sym == SDLK_MINUS){
    if(cycles_per_frame > 2){
        cycles_per_frame -= 2;
    }
}
if(event.key.keysym.sym == SDLK_F5){
    chip8.save_state("save_state.ch8");
}

if(event.key.keysym.sym == SDLK_F6){
    chip8.load_state("save_state.ch8");
}
if(event.key.keysym.sym == SDLK_c){
    color_scheme++;

    if(color_scheme > 7){
        color_scheme = 0;
    }
}
if(event.key.keysym.sym == SDLK_p){
    paused = !paused;
}
            // Check which Chip-8 key was pressed
            for(int i=0; i<16; i++){
                if(event.key.keysym.sym == keymap[i]) chip8.key[i] = 1;
            }
        }
        if(event.type == SDL_KEYUP){
            for(int i=0; i<16; i++){
                if(event.key.keysym.sym == keymap[i]) chip8.key[i] = 0;
            }
        }
    } 
}

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    // Audio setup
    bool beeping = false;
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;
    want.userdata = &beeping;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(audio_device == 0) std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
    else SDL_PauseAudioDevice(audio_device, 0);

    SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if(!window){
        std::cerr << "Window error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer){
        std::cerr << "Renderer error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Chip8 chip8;
    chip8.load_rom(argv[1]);
    
    bool running = true;
    bool paused = false;
    bool muted = false;
    int cycles_per_frame = 10;
    int color_scheme = 0;
    while(running){
    handle_input(chip8, running, cycles_per_frame, color_scheme, paused,muted,window);

    if(!paused){
    for(int i = 0; i < cycles_per_frame; i++){
        chip8.emulate_cycle();
    }

    chip8.update_timers();

    beeping = (chip8.get_sound_timer() > 0) && !muted;
}

draw_graphics(renderer, chip8, color_scheme);

    SDL_Delay(16);
}
    if(audio_device != 0) SDL_CloseAudioDevice(audio_device);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}