#pragma once
#include <stdbool.h>
#include "types.h"
#include <stddef.h>  // size_t
#include <stdio.h>


void get_timestamp(char* buffer, size_t size);
bool points_changed(Point old_points[], Point new_points[]);
void save_area_to_file(double area, double error);
