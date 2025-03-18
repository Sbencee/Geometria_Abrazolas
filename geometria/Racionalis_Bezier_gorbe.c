#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

const double POINT_RADIUS = 10.0;
const int N_POINTS = 4;
const int WEIGHT_SLIDER_WIDTH = 150;
const int WEIGHT_SLIDER_HEIGHT = 10;
const int SLIDER_Y_OFFSET = 50;
const double MAX_WEIGHT = 10.0;  // Maximum weight for sliders

/**
 * A simple point structure.
 */
typedef struct Point
{
  double x;
  double y;
} Point;

/**
 * A point structure with weight (for rational Bézier curve).
 */
typedef struct WeightedPoint
{
  Point point;
  double weight;
} WeightedPoint;

/**
 * Calculate the factorial of a number.
 */
int factorial(int n)
{
  if (n == 0 || n == 1)
    return 1;
  int result = 1;
  for (int i = 2; i <= n; i++)
    result *= i;
  return result;
}

/**
 * Calculate the Bernstein polynomial basis function.
 */
double bernstein(int i, int n, double t)
{
  return (factorial(n) / (factorial(i) * factorial(n - i))) * pow(t, i) * pow(1 - t, n - i);
}

/**
 * Calculate a rational Bézier curve point.
 */
Point rationalBezier(WeightedPoint points[], int n, double t)
{
  double num_x = 0.0, num_y = 0.0, denom = 0.0;

  for (int i = 0; i < n; ++i)
  {
    double b = bernstein(i, n - 1, t);
    num_x += points[i].weight * b * points[i].point.x;
    num_y += points[i].weight * b * points[i].point.y;
    denom += points[i].weight * b;
  }

  Point result = { num_x / denom, num_y / denom };
  return result;
}

/**
 * Update the weight of a point based on the slider position.
 */
void updateWeightFromClick(WeightedPoint* point, int mouse_x)
{
  double slider_min_x = point->point.x - WEIGHT_SLIDER_WIDTH / 2;
  double slider_max_x = point->point.x + WEIGHT_SLIDER_WIDTH / 2;
  
  if (mouse_x < slider_min_x)
    point->weight = 0.0;
  else if (mouse_x > slider_max_x)
    point->weight = MAX_WEIGHT;
  else
    point->weight = (mouse_x - slider_min_x) / WEIGHT_SLIDER_WIDTH * MAX_WEIGHT;
}

/**
 * Calculate the distance between two points.
 */
double distance(Point p1, Point p2)
{
  return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

/**
 * Approximate the length of the rational Bézier curve using numerical integration.
 */
double approximateCurveLength(WeightedPoint points[], int n)
{
  double length = 0.0;
  int steps = 100;  // Number of steps for the approximation
  Point prev_point = rationalBezier(points, n, 0.0);

  for (int i = 1; i <= steps; i++) {
    double t = (double)i / steps;
    Point curr_point = rationalBezier(points, n, t);
    length += distance(prev_point, curr_point);
    prev_point = curr_point;
  }

  return length;
}

/**
 * C/SDL2 framework for experimentation with curves.
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

  WeightedPoint* selected_point = NULL;
  WeightedPoint points[N_POINTS];
  points[0].point.x = 200;
  points[0].point.y = 200;
  points[0].weight = 1.0;
  points[1].point.x = 400;
  points[1].point.y = 200;
  points[1].weight = 1.0;
  points[2].point.x = 200;
  points[2].point.y = 400;
  points[2].weight = 1.0;
  points[3].point.x = 400;
  points[3].point.y = 400;
  points[3].weight = 1.0;

  error_code = SDL_Init(SDL_INIT_EVERYTHING);
  if (error_code != 0) {
    printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
    return error_code;
  }

  window = SDL_CreateWindow(
    "Rational Bézier Curve with Interactive Weights",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    800, 600, 0);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  double curve_length = 0.0;
  double last_curve_length = 0.0;
  bool length_updated = false;

  need_run = true;
  while (need_run) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
        SDL_GetMouseState(&mouse_x, &mouse_y);
        selected_point = NULL;
        for (int i = 0; i < N_POINTS; ++i) {
          double dx = points[i].point.x - mouse_x;
          double dy = points[i].point.y - mouse_y;
          double distance = sqrt(dx * dx + dy * dy);
          if (distance < POINT_RADIUS) {
            selected_point = points + i;
          }

          // Check if the user clicks on a slider to update the weight
          if (mouse_y > SLIDER_Y_OFFSET && mouse_y < SLIDER_Y_OFFSET + WEIGHT_SLIDER_HEIGHT) {
            double slider_min_x = points[i].point.x - WEIGHT_SLIDER_WIDTH / 2;
            double slider_max_x = points[i].point.x + WEIGHT_SLIDER_WIDTH / 2;

            if (mouse_x >= slider_min_x && mouse_x <= slider_max_x) {
              updateWeightFromClick(&points[i], mouse_x);
              length_updated = true; // Mark that the length needs to be updated
            }
          }
        }
        break;
      case SDL_MOUSEMOTION:
        if (selected_point != NULL) {
          SDL_GetMouseState(&mouse_x, &mouse_y);
          selected_point->point.x = mouse_x;
          selected_point->point.y = mouse_y;
        }
        // Redraw
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Draw the control points and weights
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        for (int i = 0; i < N_POINTS; ++i) {
          SDL_RenderDrawLine(renderer, points[i].point.x - POINT_RADIUS, points[i].point.y, points[i].point.x + POINT_RADIUS, points[i].point.y);
          SDL_RenderDrawLine(renderer, points[i].point.x, points[i].point.y - POINT_RADIUS, points[i].point.x, points[i].point.y + POINT_RADIUS);
          
          // Draw sliders for weight adjustment
          SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
          int slider_x = (int)(points[i].point.x - WEIGHT_SLIDER_WIDTH / 2);
          int slider_y = SLIDER_Y_OFFSET;
          SDL_RenderDrawRect(renderer, &(SDL_Rect){slider_x, slider_y, WEIGHT_SLIDER_WIDTH, WEIGHT_SLIDER_HEIGHT});
          
          // Draw weight value as a small line in the slider
          int weight_pos = slider_x + (int)((points[i].weight / MAX_WEIGHT) * WEIGHT_SLIDER_WIDTH);
          SDL_RenderDrawLine(renderer, weight_pos, slider_y, weight_pos, slider_y + WEIGHT_SLIDER_HEIGHT);
        }

        // Draw the segments
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, SDL_ALPHA_OPAQUE);
        for (int i = 1; i < N_POINTS; ++i) {
          SDL_RenderDrawLine(renderer, points[i - 1].point.x, points[i - 1].point.y, points[i].point.x, points[i].point.y);
        }

        // Draw the rational Bézier curve
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        for (double t = 0.0; t <= 1.0; t += 0.01) {
          Point bezier_point = rationalBezier(points, N_POINTS, t);
          SDL_RenderDrawPoint(renderer, (int)bezier_point.x, (int)bezier_point.y);
        }

        // Display the length if it was updated
        if (length_updated) {
          curve_length = approximateCurveLength(points, N_POINTS);
          if (curve_length != last_curve_length) {
            last_curve_length = curve_length;
            length_updated = false;  // Only update once
            printf("Curve Length Approximation: %.2f\n", curve_length);  // Print to console
          }
        }

        SDL_RenderPresent(renderer);
        break;
      case SDL_MOUSEBUTTONUP:
        selected_point = NULL;
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
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
