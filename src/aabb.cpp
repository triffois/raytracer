#include "./aabb.hpp"
#include "./load_model.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

char get_next_coord(char coord) {
    if (coord == 'x') {
        return 'y';
    } else if (coord == 'y') {
        return 'z';
    } else {
        return 'x';
    }
}

float get_coord(char coord, const Vec3ForGLSL &v) {
    if (coord == 'x') {
        return v.x;
    } else if (coord == 'y') {
        return v.y;
    } else {
        return v.z;
    }
}

Box *triangles_to_aabb(std::vector<TriangleForGLSL> &triangles, size_t start,
                       size_t end, char coord) {
    float min;
    float max;
    size_t span = end - start;
    std::sort(triangles.begin() + start, triangles.begin() + end,
              [coord](const TriangleForGLSL &a, const TriangleForGLSL &b) {
                  return get_coord(coord, a.min) < get_coord(coord, b.min);
              });
    min = get_coord(coord, triangles[start].min);
    max = std::numeric_limits<float>::min();
    for (size_t i = start; i < end; i++) {
        max = std::max(max, get_coord(coord, triangles[i].max));
    }
    if (span <= 8) {
        return new Box(min, max, nullptr, nullptr, start, end, coord);
    }
    size_t mid = start + span / 2;
    Box *left = triangles_to_aabb(triangles, start, mid, get_next_coord(coord));
    Box *right = triangles_to_aabb(triangles, mid, end, get_next_coord(coord));
    return new Box(min, max, left, right, start, end, coord);
}

void print_aabb(Box *box, size_t depth,
                std::vector<TriangleForGLSL> &triangles) {
    for (size_t i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    std::cout << "min: " << box->min << ", max: " << box->max
              << ", start: " << box->start << ", end: " << box->end
              << std::endl;
    if (box->left) {
        print_aabb(box->left, depth + 1, triangles);
        print_aabb(box->right, depth + 1, triangles);
        return;
    }
    // print triangles
    for (size_t i = box->start; i < box->end; i++) {
        for (size_t j = 0; j < depth; j++) {
            std::cout << "  ";
        }
        std::cout << "  ";
        std::cout << "[[" << triangles[i].v1.x << ", " << triangles[i].v1.y
                  << ", " << triangles[i].v1.z << "], [" << triangles[i].v2.x
                  << ", " << triangles[i].v2.y << ", " << triangles[i].v2.z
                  << "], [" << triangles[i].v3.x << ", " << triangles[i].v3.y
                  << ", " << triangles[i].v3.z << "]]" << std::endl;
    }
}
