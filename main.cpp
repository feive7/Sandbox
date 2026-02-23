#include <iostream>
#include <vector>
#include "raylib.h"
const int SIZE = 64;
const int SCREEN_SIZE = 640;

int tile_size = SCREEN_SIZE / SIZE;
int pal_size = SCREEN_SIZE / 21;
int current_color = 0;
std::vector<Color> color;
std::vector<Color> pallette;
std::vector<std::vector<int>> Map(SIZE, std::vector<int>(SIZE, 0));

struct Cell {
    int x;
    int y;
};

void init_colors() {
	color.push_back({ 0,0,0,255 });
	for (int i = 0; i < 11; i++) {
		color.push_back(ColorFromHSV(360 * i / 11.0f, 1, 1));
	}
	for (int i = 255; i >= 33; i -= 255 / 11) {
		color.push_back(Color{ (unsigned char)i,(unsigned char)i,(unsigned char)i,255 });
	}
	pallette.assign(color.begin() + 1, color.end());
}

void draw_map() {
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < SIZE; y++) {
			Color cell_color = color[Map[y][x]];
			DrawRectangle(x * tile_size, y * tile_size, tile_size, tile_size, cell_color);
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

void draw(Cell cell, int color_id) {
	Map[cell.y][cell.x] = color_id;
}

void sand() {
	std::vector<std::vector<int>> buffer(SIZE, std::vector<int>(SIZE, 0));
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
                        int move = moves[GetRandomValue(0,moves.size()-1)];
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
    DrawRectangle(0, SCREEN_SIZE, SCREEN_SIZE, 30, GRAY);
    for (int i = 0; i < pallette.size(); i++) {
        DrawRectangle(pal_size * i, SCREEN_SIZE + (5 * (i != current_color)), pal_size, 25, pallette[i]);
    }
}

void select_color(int params) {
    int old_color = current_color;
    if (params >= 0) {
        current_color = std::max(0, std::min((int)(pallette.size() - 1), params));
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
}

Cell cell(Vector2 pos) {
    return { (int)(pos.x / tile_size), (int)(pos.y / tile_size) };
}

void handle_mouse() {
    Vector2 pos = GetMousePosition();
    if (pos.x >= 0 && pos.x < SCREEN_SIZE) {
        if (pos.y >= 0 && pos.y < SCREEN_SIZE) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                draw(cell(pos), current_color + 1);
            }
            else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
                Cell tile = cell(pos);
                select_color(Map[tile.y][tile.x] - 1);
            }
            else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                draw(cell(pos), 0);
            }
        }
        else if (pos.y >= SCREEN_SIZE && pos.y < SCREEN_SIZE + 30 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            select_color(pos.x / pal_size);
        }
    }
    float mwheel = GetMouseWheelMove();
    if (mwheel > 0) {
        select_color(-2);
    }
    else if (mwheel < 0) {
        select_color(-1);
    }
}

void handle_keyboard() {
    if (IsKeyPressed(KEY_C)) {
        std::fill(Map.begin(), Map.end(), std::vector<int>(SIZE, 0));
    }
    if (IsKeyPressed(KEY_L)) {
        std::fill(Map[0].begin(), Map[0].end(), current_color + 1);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        select_color(-1);
    }
    if (IsKeyPressed(KEY_LEFT)) {
        select_color(-2);
    }
    if (IsKeyPressed(KEY_S)) {
        TakeScreenshot("export.bmp");
    }
}

int main() {
	// Define window dimensions
	const int window_width = SCREEN_SIZE;
	const int window_height = SCREEN_SIZE + 30;

    // Create window
    InitWindow(window_width, window_height, "Raybox");
    SetTargetFPS(60);

    init_colors();

    while (!WindowShouldClose()) {
        handle_keyboard();
        handle_mouse();
        sand();

        BeginDrawing();
        draw_menu();
        draw_map();
        EndDrawing();
    }

	return 0;
}