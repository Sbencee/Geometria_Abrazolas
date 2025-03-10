#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const double POINT_RADIUS = 10.0;
const int N_POINTS = 4;

typedef struct Point {
    double x;
    double y;
} Point;

// Hermite-polynomial parameterization
// t: a paraméter 0 és 1 között
double hermite(double t, double p0, double p1, double m0, double m1) {
    double h0 = 2 * t * t * t - 3 * t * t + 1;     // H0(t)
    double h1 = -2 * t * t * t + 3 * t * t;        // H1(t)
    double h2 = t * t * t - 2 * t * t + t;         // H2(t)
    double h3 = t * t * t - t * t;                 // H3(t)

    return h0 * p0 + h1 * p1 + h2 * m0 + h3 * m1;
}

int main(int argc, char* argv[]) {
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

    // Add tangents (derivatives at each point)
    Point tangents[N_POINTS];
    tangents[0].x = 100; // Tangent at P0
    tangents[0].y = 0;
    tangents[1].x = 100; // Tangent at P1
    tangents[1].y = 0;
    tangents[2].x = -100; // Tangent at P2
    tangents[2].y = 0;
    tangents[3].x = -100; // Tangent at P3
    tangents[3].y = 0;

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return error_code;
    }

    window = SDL_CreateWindow(
        "Hermite Curve",
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
                break;
            case SDL_MOUSEMOTION:
                if (selected_point != NULL) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    selected_point->x = mouse_x;
                    selected_point->y = mouse_y;
                }
                // Draw Hermite curve
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);

                // Draw the control points
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
                for (int i = 0; i < N_POINTS; ++i) {
                    SDL_RenderDrawLine(renderer, points[i].x - POINT_RADIUS, points[i].y, points[i].x + POINT_RADIUS, points[i].y);
                    SDL_RenderDrawLine(renderer, points[i].x, points[i].y - POINT_RADIUS, points[i].x, points[i].y + POINT_RADIUS);
                }

                // Draw the tangents (derivatives)
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                for (int i = 0; i < N_POINTS; ++i) {
                    SDL_RenderDrawLine(renderer, points[i].x, points[i].y,
                                       points[i].x + tangents[i].x, points[i].y + tangents[i].y);
                }

                // Draw the Hermite curve
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                for (double t = 0; t <= 1; t += 0.01) {
                    double x = hermite(t, points[0].x, points[1].x, tangents[0].x, tangents[1].x);
                    double y = hermite(t, points[0].y, points[1].y, tangents[0].y, tangents[1].y);
                    SDL_RenderDrawPoint(renderer, (int)x, (int)y);
                }

                // Display the results
                SDL_RenderPresent(renderer);
                break;
            case SDL_MOUSEBUTTONUP:
                selected_point = NULL;
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
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
