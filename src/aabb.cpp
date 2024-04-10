#include "./aabb.hpp"
#include "./load_model.hpp"
#include <algorithm>
#include <iostream>
#include <vector>

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
    float min;
    float max;
    int span = end - start;
    std::sort(triangles.begin() + start, triangles.begin() + end,
              [coord](const TriangleForGLSL &a, const TriangleForGLSL &b) {
                  return get_coord(coord, a.min) < get_coord(coord, b.min);
              });
    min = get_coord(coord, triangles[start].min);
    max = std::numeric_limits<float>::min();
    for (int i = start; i < end; i++) {
        max = std::max(max, get_coord(coord, triangles[i].max));
    }
    if (span <= 8) {
        return Box(min, max, -1, -1, start, end, coord);
    }
    int mid = start + span / 2;
    boxes.push_back(
        triangles_to_box(boxes, triangles, start, mid, get_next_coord(coord)));
    int left = boxes.size() - 1;
    boxes.push_back(
        triangles_to_box(boxes, triangles, mid, end, get_next_coord(coord)));
    return Box(min, max, left, left + 1, start, end, coord);
}

AABB *triangles_to_aabb(std::vector<Box> &boxes,
                        std::vector<TriangleForGLSL> &triangles, int start,
                        int end, int coord) {
    float min;
    float max;
    int span = end - start;
    std::sort(triangles.begin() + start, triangles.begin() + end,
              [coord](const TriangleForGLSL &a, const TriangleForGLSL &b) {
                  return get_coord(coord, a.min) < get_coord(coord, b.min);
              });
    min = get_coord(coord, triangles[start].min);
    max = std::numeric_limits<float>::min();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::min();
    float min_z = std::numeric_limits<float>::max();
    float max_z = std::numeric_limits<float>::min();
    for (int i = start; i < end; i++) {
        max = std::max(max, triangles[i].max.x);
        max_y = std::max(max_y, triangles[i].max.y);
        max_z = std::max(max_z, triangles[i].max.z);
        min_y = std::min(min_y, triangles[i].min.y);
        min_z = std::min(min_z, triangles[i].min.z);
    }
    if (span <= 8) {
        boxes.push_back(Box(min, max, -1, -1, start, end, coord));
        return new AABB{max,
                        min,
                        max_y,
                        min_y,
                        max_z,
                        min_z,
                        static_cast<int>(boxes.size() - 1)};
    }
    int mid = start + span / 2;
    boxes.push_back(
        triangles_to_box(boxes, triangles, start, mid, get_next_coord(coord)));
    int left = boxes.size() - 1;
    int right = boxes.size();
    boxes.push_back(
        triangles_to_box(boxes, triangles, mid, end, get_next_coord(coord)));
    boxes.push_back(Box(min, max, left, right, start, end, coord));
    return new AABB{max,
                    min,
                    max_y,
                    min_y,
                    max_z,
                    min_z,
                    static_cast<int>(boxes.size() - 1)};
}

void print_box(std::vector<Box> boxes, int box_id, size_t depth,
               std::vector<TriangleForGLSL> &triangles) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    const Box *box = &boxes[box_id];
    std::cout << "min: " << box->min << ", max: " << box->max
              << ", start: " << box->start << ", end: " << box->end
              << std::endl;
    if (box->left_id != -1) {
        print_box(boxes, box->left_id, depth + 1, triangles);
        print_box(boxes, box->right_id, depth + 1, triangles);
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
