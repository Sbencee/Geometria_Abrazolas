#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const double POINT_RADIUS = 10.0;
const int N_POINTS = 4;
const int SLIDER_WIDTH = 200;
const int SLIDER_HEIGHT = 20;
const int SLIDER_X = 300;
const int SLIDER_Y = 550;
const int CURVE_POINTS = 100; // A görbe pontjainak száma

/**
 * A simple point structure.
 */
typedef struct Point
{
    double x;
    double y;
} Point;

/**
 * C/SDL2 framework for experimentation with curves.
 */

// Függvény, amely segít a kör kirajzolásában
void DrawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);

        if (err <= 0) {
            ++y;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            --x;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

// de Casteljau algoritmus
Point deCasteljau(Point* points, int n, double t) {
    Point new_points[N_POINTS];
    for (int i = 0; i < n; ++i) {
        new_points[i] = points[i];
    }

    for (int k = 1; k < n; ++k) {
        for (int i = 0; i < n - k; ++i) {
            new_points[i].x = (1 - t) * new_points[i].x + t * new_points[i + 1].x;
            new_points[i].y = (1 - t) * new_points[i].y + t * new_points[i + 1].y;
        }
    }

    return new_points[0];
}

void DrawDeCasteljau(SDL_Renderer* renderer, Point* points, int n, double t) {
    Point new_points[N_POINTS];
    for (int i = 0; i < n; ++i) {
        new_points[i] = points[i];
    }

    for (int k = 1; k < n; ++k) {
        for (int i = 0; i < n - k; ++i) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLine(renderer, new_points[i].x, new_points[i].y, new_points[i + 1].x, new_points[i + 1].y);

            new_points[i].x = (1 - t) * new_points[i].x + t * new_points[i + 1].x;
            new_points[i].y = (1 - t) * new_points[i].y + t * new_points[i + 1].y;
        }
    }
}

void DrawCurve(SDL_Renderer* renderer, Point* points, int n) {
    Point curve_points[CURVE_POINTS];
    for (int i = 0; i < CURVE_POINTS; ++i) {
        double t = (double)i / (CURVE_POINTS - 1);
        curve_points[i] = deCasteljau(points, n, t);
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    for (int i = 1; i < CURVE_POINTS; ++i) {
        SDL_RenderDrawLine(renderer, curve_points[i - 1].x, curve_points[i - 1].y, curve_points[i].x, curve_points[i].y);
    }
}

void DrawSlider(SDL_Renderer* renderer, double t) {
    SDL_Rect slider_rect = {SLIDER_X, SLIDER_Y, SLIDER_WIDTH, SLIDER_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &slider_rect);

    int handle_x = SLIDER_X + (int)(t * SLIDER_WIDTH) - 5;
    SDL_Rect handle_rect = {handle_x, SLIDER_Y - 5, 10, SLIDER_HEIGHT + 10};
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &handle_rect);
}

int main(int argc, char* argv[])
{
    int error_code;
    SDL_Window* window;
    bool need_run;
    SDL_Event event;
    SDL_Renderer* renderer;

    int mouse_x, mouse_y;
    int i;

    Point* selected_point = NULL;
    Point points[N_POINTS];
    points[0].x = 200;
    points[0].y = 200;
    points[1].x = 400;
    points[1].y = 200;
    points[2].x = 200;
    points[2].y = 400;
    points[3].x = 400;
    points[3].y = 400;

    double t = 0.5; // Paraméter értéke
    bool dragging_slider = false;

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return error_code;
    }

    window = SDL_CreateWindow(
        "de Casteljau Algoritmus",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    need_run = true;
    while (need_run) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                SDL_GetMouseState(&mouse_x, &mouse_y);
                selected_point = NULL;
                for (int i = 0; i < N_POINTS; ++i) {
                    double dx = points[i].x - mouse_x;
                    double dy = points[i].y - mouse_y;
                    double distance = sqrt(dx * dx + dy * dy);
                    if (distance < POINT_RADIUS) {
                        selected_point = points + i;
                    }
                }
                if (mouse_x >= SLIDER_X && mouse_x <= SLIDER_X + SLIDER_WIDTH &&
                    mouse_y >= SLIDER_Y && mouse_y <= SLIDER_Y + SLIDER_HEIGHT) {
                    dragging_slider = true;
                }
                break;
            case SDL_MOUSEMOTION:
                if (selected_point != NULL) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    selected_point->x = mouse_x;
                    selected_point->y = mouse_y;
                }
                if (dragging_slider) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    t = (double)(mouse_x - SLIDER_X) / SLIDER_WIDTH;
                    if (t < 0) t = 0;
                    if (t > 1) t = 1;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                selected_point = NULL;
                dragging_slider = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    need_run = false;
                }
                break;
            case SDL_QUIT:
                need_run = false;
                break;
            }
        }

        // NOTE: It has not optimized for efficient redraw!
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw the control points
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        for (int i = 0; i < N_POINTS; ++i) {
            DrawCircle(renderer, (int)points[i].x, (int)points[i].y, (int)POINT_RADIUS);
        }

        // Draw the segments (control lines)
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
        for (int i = 1; i < N_POINTS; ++i) {
            SDL_RenderDrawLine(renderer, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);
        }

        // Draw de Casteljau algorithm
        DrawDeCasteljau(renderer, points, N_POINTS, t);
	// Draw the curve
        DrawCurve(renderer, points, N_POINTS);

        // Draw the slider
        DrawSlider(renderer, t);

        // Display the results
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}