#include "graphics.h"
#include "bezier.h"
#include "area.h"
#include "utils.h"
#include <stdio.h>   // printf, fflush, stdout
#include <math.h>    // sqrt
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    Point* selected_point = NULL;
    int mouse_x, mouse_y;
    int steps = 100;
    double approximation_error = 0.0;

    Point points[N_POINTS] = {
        {200, 200}, {400, 200}, {400, 400}, {200, 400}
    };
    Point last_points[N_POINTS];
    memcpy(last_points, points, sizeof(points));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Zárt Bézier-görbe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool need_run = true;
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
                            selected_point = &points[i];
                            break;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    selected_point = NULL;
                    break;
                case SDL_MOUSEMOTION:
                    if (selected_point != NULL) {
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        selected_point->x = mouse_x;
                        selected_point->y = mouse_y;

                        if (points_changed(last_points, points)) {
                            double area = calculate_area(points, steps, &approximation_error);
                            save_area_to_file(area, approximation_error);
                            memcpy(last_points, points, sizeof(points));
                            printf("\rTerulet: %.2f    Hiba: %.5f       ", area, approximation_error);
                            fflush(stdout);
                        }
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_q) {
                        need_run = false;
                    }
                    break;
                case SDL_QUIT:
                    need_run = false;
                    break;
            }
        }

        render_scene(renderer, points, steps);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
