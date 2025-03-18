#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const double POINT_RADIUS = 10.0;
const int N_POINTS = 3;

/**
 * A simple point structure.
 */
typedef struct Point
{
  double x;
  double y;
} Point;

/**
 * Kiszámítja a Bessel parabola középső pontjára tartozó érintőt.
 */
void bessel_tangent(Point points[], Point* tangent_point) {
  double x1 = points[0].x, y1 = points[0].y;
  double x2 = points[1].x, y2 = points[1].y;
  double x3 = points[2].x, y3 = points[2].y;

  // Kiszámítjuk a parabola érintőjét a középső pontra
  tangent_point->x = x2;
  tangent_point->y = (y1 + y3) / 2.0;  // Egyszerűsített érintő számítás
}

/**
 * Kiszámítja a Bessel parabola pontját három vezérlőpont alapján.
 */
double bessel_parabola(double t, Point points[]) {
  double x0 = points[0].x, y0 = points[0].y;
  double x1 = points[1].x, y1 = points[1].y;
  double x2 = points[2].x, y2 = points[2].y;
  
  // Bessel parabola paraméterezése
  double t2 = t * t;
  double t3 = t2 * t;

  return ((1 - 2 * t + t2) * y0 + (2 * t - 3 * t2 + 1) * y1 + (-t + t2) * y2);
}

/**
 * C/SDL2 framework for experimentation with Bessel parabola and tangent.
 */
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
  points[2].x = 300;
  points[2].y = 400;

  Point tangent_point;

  error_code = SDL_Init(SDL_INIT_EVERYTHING);
  if (error_code != 0) {
    printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
    return error_code;
  }

  window = SDL_CreateWindow(
    "Bessel Parabola",
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

        // Redraw the screen with updated points and Bessel parabola
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw the control points
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        for (int i = 0; i < N_POINTS; ++i) {
          SDL_RenderDrawLine(renderer, points[i].x - POINT_RADIUS, points[i].y, points[i].x + POINT_RADIUS, points[i].y);
          SDL_RenderDrawLine(renderer, points[i].x, points[i].y - POINT_RADIUS, points[i].x, points[i].y + POINT_RADIUS);
        }

        // Draw the Bessel parabola
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        for (double t = 0; t <= 1; t += 0.01) {
          double y = bessel_parabola(t, points);
          SDL_RenderDrawPoint(renderer, (int)(points[1].x + t * (points[2].x - points[1].x)), (int)y);
        }

        // Calculate and draw the tangent at the middle point
        bessel_tangent(points, &tangent_point);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(renderer, (int)(tangent_point.x - 100), (int)tangent_point.y, (int)(tangent_point.x + 100), (int)tangent_point.y);

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
