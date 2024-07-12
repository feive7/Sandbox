#include <SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <filesystem>
const int SIZE = 64;
const int SCREEN_SIZE = 640;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* iconTexture = nullptr;
int tile_size = SCREEN_SIZE / SIZE;
int pal_size = SCREEN_SIZE / 21;
int current_color = 0;
std::vector<SDL_Color> color;
std::vector<SDL_Color> pallette;
std::vector<std::vector<int>> Map(SIZE, std::vector<int>(SIZE, 0));

SDL_Color hsv_to_rgb(float h, float s, float v) {
    float r, g, b;
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
    }
    return SDL_Color{ static_cast<Uint8>(r * 255), static_cast<Uint8>(g * 255), static_cast<Uint8>(b * 255), 255 };
}

void init_colors() {
    color.push_back({ 0, 0, 0, 255 });
    for (int i = 0; i < 11; ++i) {
        color.push_back(hsv_to_rgb(i / 11.0f, 1, 1));
    }
    for (int i = 255; i >= 33; i -= 255 / 11) {
        color.push_back({ static_cast<Uint8>(i), static_cast<Uint8>(i), static_cast<Uint8>(i), 255 });
    }
    pallette.assign(color.begin() + 1, color.end());
}

void draw_map() {
    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            SDL_Rect rect = { x * tile_size, y * tile_size, tile_size, tile_size };
            SDL_SetRenderDrawColor(renderer, color[Map[y][x]].r, color[Map[y][x]].g, color[Map[y][x]].b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int get_tile(int x, int y) {
    if (x >= 0 && x < SIZE && y >= 0 && y < SIZE) {
        return Map[y][x];
    }
    else if (y >= SIZE || x < 0 || x >= SIZE) {
        return 1;
    }
    else {
        return 0;
    }
}

void draw(SDL_Point cell, int color) {
    Map[cell.y][cell.x] = color;
}

void sand() {
    std::vector<std::vector<int>> buffer(SIZE, std::vector<int>(SIZE, 0));
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            int current_tile = get_tile(x, y);
            if (current_tile) {
                int below = get_tile(x, y + 1);
                if (!below) {
                    buffer[y + 1][x] = current_tile;
                }
                else {
                    std::vector<int> moves;
                    int left_below = get_tile(x - 1, y + 1);
                    int right_below = get_tile(x + 1, y + 1);
                    if (!right_below) {
                        moves.push_back(1);
                    }
                    if (!left_below) {
                        moves.push_back(-1);
                    }
                    if (!moves.empty()) {
                        std::uniform_int_distribution<> dis(0, moves.size() - 1);
                        int move = moves[dis(gen)];
                        buffer[y + 1][x + move] = current_tile;
                    }
                    else {
                        buffer[y][x] = current_tile;
                    }
                }
            }
        }
    }
    Map = buffer;
}

void draw_menu() {
    SDL_Rect menu_rect = { 0, SCREEN_SIZE, SCREEN_SIZE, 30 };
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &menu_rect);
    for (int i = 0; i < pallette.size(); ++i) {
        SDL_Rect color_rect = { pal_size * i, SCREEN_SIZE + (5 * (i != current_color)), pal_size, 25 };
        SDL_SetRenderDrawColor(renderer, pallette[i].r, pallette[i].g, pallette[i].b, 255);
        SDL_RenderFillRect(renderer, &color_rect);
    }
}

void select_color(int params) {
    int old_color = current_color;
    if (params >= 0) {
        current_color = std::max(0, std::min(static_cast<int>(pallette.size()) - 1, params));
    }
    else {
        if (params == -1) {
            current_color = (current_color + 1) % pallette.size();
        }
        else if (params == -2) {
            current_color = (current_color - 1 + pallette.size()) % pallette.size();
        }
    }
    draw_menu();
    //std::cout << old_color << " -> " << current_color << std::endl;
}

SDL_Point cell(SDL_Point pos) {
    return { pos.x / tile_size, pos.y / tile_size };
}

void handle_mouse_down(SDL_Point pos, int button) {
    if (pos.x >= 0 && pos.x < SCREEN_SIZE) {
        if (pos.y >= 0 && pos.y < SCREEN_SIZE) {
            if (button == 1) {
                draw(cell(pos), current_color + 1);
            }
            else if (button == 2) {
                SDL_Point tile = cell(pos);
                select_color(Map[tile.y][tile.x] - 1);
            }
            else if (button == 4) {
                draw(cell(pos), 0);
            }
        }
        else if (pos.y >= SCREEN_SIZE && pos.y < SCREEN_SIZE + 30 && button == 1) {
            select_color(pos.x / pal_size);
        }
    }
}

void handle_key_down(SDL_KeyboardEvent& event) {
    if (event.keysym.sym == SDLK_ESCAPE) {
        SDL_Event quit_event;
        quit_event.type = SDL_QUIT;
        SDL_PushEvent(&quit_event);
    }
    else {
        switch (event.keysym.sym) {
        case SDLK_c:
            std::fill(Map.begin(), Map.end(), std::vector<int>(SIZE, 0));
            break;
        case SDLK_l:
            std::fill(Map[0].begin(), Map[0].end(), current_color + 1);
            break;
        case SDLK_RIGHT:
            select_color(-1);
            break;
        case SDLK_LEFT:
            select_color(-2);
            break;
            
        case SDLK_s:
            SDL_Surface* sshot = SDL_CreateRGBSurface(0, 512, 512, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
            SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
            SDL_SaveBMP(sshot, "export.bmp");
            SDL_FreeSurface(sshot);
            break;
        //default:
            //std::cout << SDL_GetKeyName(event.keysym.sym) << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    window = SDL_CreateWindow("Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_SIZE, SCREEN_SIZE + 30, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    init_colors();

    draw_menu();
    bool running = true;
    Uint32 frameDelay = 10;
    int mouse_buttons;
    bool mouse_down = false;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                handle_key_down(event.key);
            }
            else if (event.type == SDL_MOUSEWHEEL) {
                select_color(event.wheel.y > 0 ? -1 : -2);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                mouse_down = (event.type == SDL_MOUSEBUTTONDOWN) ? true : false;
            }
        }
        if (mouse_down) {
            int x, y;
            mouse_buttons = SDL_GetMouseState(&x, &y);
            handle_mouse_down({ x,y }, mouse_buttons);
        }
        sand();
        draw_map();
        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyTexture(iconTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
