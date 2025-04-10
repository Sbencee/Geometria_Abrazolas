#include "graphics.h"
#include "bezier.h"

void render_scene(SDL_Renderer* renderer, Point points[], int steps) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < N_POINTS; ++i) {
        SDL_RenderDrawLine(renderer, points[i].x - POINT_RADIUS, points[i].y, points[i].x + POINT_RADIUS, points[i].y);
        SDL_RenderDrawLine(renderer, points[i].x, points[i].y - POINT_RADIUS, points[i].x, points[i].y + POINT_RADIUS);
    }

    SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < N_POINTS; ++i) {
        Point p0 = points[i];
        Point p1 = points[(i + 1) % N_POINTS];
        Point p2 = points[(i + 2) % N_POINTS];
        Point p3 = points[(i + 3) % N_POINTS];
        Point prev_point = p0;
        for (int j = 0; j <= steps; ++j) {
            double t = (double)j / steps;
            Point new_point = bezier(p0, p1, p2, p3, t);
            SDL_RenderDrawLine(renderer, prev_point.x, prev_point.y, new_point.x, new_point.y);
            prev_point = new_point;
        }
    }

    SDL_RenderPresent(renderer);
}
