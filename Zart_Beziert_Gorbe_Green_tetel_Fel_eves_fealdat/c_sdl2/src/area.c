#include "area.h"
#include "bezier.h"
#include <math.h>

double calculate_area(Point points[], int steps, double* approximation_error) {
    double area = 0.0;
    double prev_area = 0.0;
    *approximation_error = 0.0;

    for (int i = 0; i < N_POINTS; ++i) {
        Point p0 = points[i];
        Point p1 = points[(i + 1) % N_POINTS];
        Point p2 = points[(i + 2) % N_POINTS];
        Point p3 = points[(i + 3) % N_POINTS];

        Point prev = bezier(p0, p1, p2, p3, 0);
        for (int j = 1; j <= steps; ++j) {
            double t = (double)j / steps;
            Point curr = bezier(p0, p1, p2, p3, t);
            area += (prev.x * curr.y - curr.x * prev.y);
            prev = curr;
        }
    }

    area = fabs(area) / 2.0;
    *approximation_error = fabs(area - prev_area);
    return area;
}
