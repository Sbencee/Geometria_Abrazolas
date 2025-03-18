#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <float.h> // A NaN és Inf kezelése

const double POINT_RADIUS = 10.0;
const int N_POINTS = 4;
const int DEGREE = 3;  // A B-spline fokozata

/**
 * A simple point structure.
 */
typedef struct Point {
    double x;
    double y;
} Point;

typedef struct ControlPoint {
    Point point;
    double weight; // Súly a racionális B-splinehoz
} ControlPoint;

/**
 * B-spline basis function (Cox-de Boor recursion formula).
 */
double basis_function(int i, int p, double u, double *knot_vector) {
    if (p == 0) {
        return (knot_vector[i] <= u && u < knot_vector[i + 1]) ? 1.0 : 0.0;
    } else {
        double denom1 = knot_vector[i + p] - knot_vector[i];
        double denom2 = knot_vector[i + p + 1] - knot_vector[i + 1];
        double term1 = denom1 != 0 ? (u - knot_vector[i]) / denom1 : 0.0;
        double term2 = denom2 != 0 ? (knot_vector[i + p + 1] - u) / denom2 : 0.0;

        return term1 * basis_function(i, p - 1, u, knot_vector) + term2 * basis_function(i + 1, p - 1, u, knot_vector);
    }
}

/**
 * A racionális B-spline görbe kiszámítása.
 */
Point rational_bspline(double u, int p, ControlPoint *points, int n, double *knot_vector) {
    double numerator_x = 0.0, numerator_y = 0.0, denominator = 0.0;
    for (int i = 0; i < n; ++i) {
        double N = basis_function(i, p, u, knot_vector);
        numerator_x += points[i].point.x * points[i].weight * N;
        numerator_y += points[i].point.y * points[i].weight * N;
        denominator += points[i].weight * N;
    }

    // Elkerüljük a nullával való osztást, ha a nevező nulla
    if (denominator == 0.0) {
        numerator_x = 0.0;
        numerator_y = 0.0;
        denominator = 1.0; // Elkerüljük a NaN-t
    }

    Point result = { numerator_x / denominator, numerator_y / denominator };
    return result;
}

/**
 * A görbe hosszának közelítése.
 * Az egyszerűsített megközelítés a szakaszok hosszának összegzésével történik.
 */
double curve_length_approximation(ControlPoint *points, int n, double *knot_vector, int degree) {
    double length = 0.0;
    double prev_x = 0, prev_y = 0;
    Point prev_point = rational_bspline(knot_vector[0], degree, points, n, knot_vector);
    
    // A görbét lépésről lépésre közelítjük, az u paramétert változtatjuk
    for (double u = 0.01; u <= 1.0; u += 0.01) {
        Point p = rational_bspline(u, degree, points, n, knot_vector);
        
        // Távolság kiszámítása az előző és a jelenlegi pont között
        double dx = p.x - prev_point.x;
        double dy = p.y - prev_point.y;
        length += sqrt(dx * dx + dy * dy);
        
        // Frissítjük a pontot
        prev_point = p;
    }

    // Ha valami probléma történt, a hiba kezelésére
    if (isnan(length) || isinf(length)) {
        printf("Error: Invalid length calculation (NaN or Inf encountered).\n");
        return -1.0;
    }

    return length;
}

/**
 * Interaktív pontok kezelése és renderelés.
 */
int main(int argc, char* argv[]) {
    int error_code;
    SDL_Window* window;
    bool need_run;
    SDL_Event event;
    SDL_Renderer* renderer;

    int mouse_x, mouse_y;
    int i;

    ControlPoint points[N_POINTS]; // Itt deklaráljuk a vezérlőpontokat
    ControlPoint* selected_point = NULL;

    // Alapértelmezett vezérlőpontok és súlyok
    points[0].point.x = 200; points[0].point.y = 200; points[0].weight = 1.0;
    points[1].point.x = 400; points[1].point.y = 200; points[1].weight = 1.0;
    points[2].point.x = 200; points[2].point.y = 400; points[2].weight = 1.0;
    points[3].point.x = 400; points[3].point.y = 400; points[3].weight = 1.0;

    // Csomóértékek (Uniform csomókat használunk egyszerűség kedvéért)
    double knot_vector[N_POINTS + DEGREE + 1]; // A csomóértékek deklarálása a main-ben
    for (int i = 0; i < N_POINTS + DEGREE + 1; ++i) {
        knot_vector[i] = (double)i / (N_POINTS + DEGREE);  // Egyenletes csomóértékek generálása
    }

    error_code = SDL_Init(SDL_INIT_EVERYTHING);
    if (error_code != 0) {
        printf("[ERROR] SDL initialization error: %s\n", SDL_GetError());
        return error_code;
    }

    window = SDL_CreateWindow(
        "Racionalis B-spline",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    need_run = true;
    double prev_x = 0, prev_y = 0;  // Kezdeti értékek a prev_x és prev_y változókhoz

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
                        selected_point = &points[i];
                    }
                }
                break;
            case SDL_MOUSEMOTION:
                if (selected_point != NULL) {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    selected_point->point.x = mouse_x;
                    selected_point->point.y = mouse_y;
                }
                // Redraw the scene
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                SDL_RenderClear(renderer);

                // Draw the control points and lines
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
                for (int i = 0; i < N_POINTS; ++i) {
                    SDL_RenderDrawLine(renderer, points[i].point.x - POINT_RADIUS, points[i].point.y,
                        points[i].point.x + POINT_RADIUS, points[i].point.y);
                    SDL_RenderDrawLine(renderer, points[i].point.x, points[i].point.y - POINT_RADIUS,
                        points[i].point.x, points[i].point.y + POINT_RADIUS);
                }

                // Draw the B-spline curve
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
                for (double u = 0.0; u <= 1.0; u += 0.01) {
                    Point p = rational_bspline(u, DEGREE, points, N_POINTS, knot_vector);
                    if (u > 0.0) {
                        SDL_RenderDrawLine(renderer, prev_x, prev_y, p.x, p.y);
                    }
                    prev_x = p.x;
                    prev_y = p.y;
                }

                // Calculate and display the curve length approximation
                double curve_length = curve_length_approximation(points, N_POINTS, knot_vector, DEGREE);
                if (curve_length >= 0) {
                    printf("Curve Length Approximation: %.2f\n", curve_length);
                } else {
                    printf("Error calculating curve length.\n");
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
