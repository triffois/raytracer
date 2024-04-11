#include "./aabb.hpp"
#include "./load_model.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <limits>

Vec3ForGLSL get_min(std::vector<TriangleForGLSL> triangles, int start, int end) {
    Vec3ForGLSL min = Vec3ForGLSL{std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max()};
    for (int i = start; i < end; i++) {
        min = Vec3ForGLSL{std::min(min.x, triangles[i].min.x),
                   std::min(min.y, triangles[i].min.y),
                   std::min(min.z, triangles[i].min.z)};
    }
    return min;
}

Vec3ForGLSL get_max(std::vector<TriangleForGLSL> triangles, int start, int end) {
    Vec3ForGLSL max = Vec3ForGLSL{-std::numeric_limits<float>::max(),
                           -std::numeric_limits<float>::max(),
                           -std::numeric_limits<float>::max()};
    for (int i = start; i < end; i++) {
        max = Vec3ForGLSL{std::max(max.x, triangles[i].max.x),
                   std::max(max.y, triangles[i].max.y),
                   std::max(max.z, triangles[i].max.z)};
    }
    return max;
}

int get_next_coord(int coord) { return (coord + 1) % 3; }

float get_coord(int coord, const Vec3ForGLSL &v) {
    if (coord == 0) {
        return v.x;
    } else if (coord == 1) {
        return v.y;
    } else {
        return v.z;
    }
}

Box triangles_to_box(std::vector<Box> &boxes,
                     std::vector<TriangleForGLSL> &triangles, int start,
                     int end, int coord) {
    int span = end - start;
    std::sort(triangles.begin() + start, triangles.begin() + end,
              [coord](const TriangleForGLSL &a, const TriangleForGLSL &b) {
                  return get_coord(coord, a.min) < get_coord(coord, b.min);
              });

    Vec3ForGLSL min = get_min(triangles, start, end);
    Vec3ForGLSL max = get_max(triangles, start, end);

    if (span <= 8) {
        return Box(min, max, -1, -1, start, end);
    }

    int mid = start + span / 2;
    boxes.push_back(
        triangles_to_box(boxes, triangles, start, mid, get_next_coord(coord)));
    int left = boxes.size() - 1;
    boxes.push_back(
        triangles_to_box(boxes, triangles, mid, end, get_next_coord(coord)));
    int right = boxes.size() - 1;
    return Box(min, max, left, right, start, end);
}

AABB *triangles_to_aabb(std::vector<Box> &boxes,
                        std::vector<TriangleForGLSL> &triangles, int start,
                        int end, int coord) {
    int span = end - start;

    Vec3ForGLSL min = get_min(triangles, start, end);
    Vec3ForGLSL max = get_max(triangles, start, end);

    if (span <= 8) {
        boxes.push_back(Box(min, max, -1, -1, start, end));
        return new AABB{max,
                        min,
                        static_cast<int>(boxes.size() - 1)};
    }
    boxes.push_back(triangles_to_box(boxes, triangles, start, end, coord));
    return new AABB{max,
                    min,
                    static_cast<int>(boxes.size() - 1)};
}

void print_box(std::vector<Box> boxes, int box_id, size_t depth,
               std::vector<TriangleForGLSL> &triangles) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    const Box *box = &boxes[box_id];
    std::cout << "min x: " << box->min.x << ", y: " << box->min.y
              << ", z: " << box->min.z << ", max x: " << box->max.x
              << ", y: " << box->max.y << ", z: " << box->max.z
              << ", start: " << box->start << ", end: " << box->end
              << ", right_id: " << box->right_id << ", left_id: " << box->left_id
              << std::endl;
    if (box->left_id != -1) {
        print_box(boxes, box->right_id, depth + 1, triangles);
        print_box(boxes, box->left_id, depth + 1, triangles);
        return;
    }
    // print triangles
    for (int i = box->start; i < box->end; i++) {
        for (int j = 0; j < depth; j++) {
            std::cout << "  ";
        }
        std::cout << "  ";
        print_triangle(Triangle{
            Vec3{triangles[i].v1.x, triangles[i].v1.y, triangles[i].v1.z},
            Vec3{triangles[i].v2.x, triangles[i].v2.y, triangles[i].v2.z},
            Vec3{triangles[i].v3.x, triangles[i].v3.y, triangles[i].v3.z}});
    }
}
