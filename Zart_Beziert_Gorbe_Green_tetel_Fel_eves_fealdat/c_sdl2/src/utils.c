#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

bool points_changed(Point old_points[], Point new_points[]) {
    for (int i = 0; i < N_POINTS; ++i) {
        if (old_points[i].x != new_points[i].x || old_points[i].y != new_points[i].y) {
            return true;
        }
    }
    return false;
}

void save_area_to_file(double area, double error) {
    FILE* file = fopen(FILENAME, "a");
    if (file == NULL) {
        printf("Hiba a fájl megnyitásakor!\n");
        return;
    }

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    fprintf(file, "[%s] Terület: %.2f pixel^2   Közelítési hiba: %.5f\n", timestamp, area, error);
    fclose(file);
}
