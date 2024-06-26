#include "./aabb.hpp"
#include "./load_model.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

PaddedVec3ForGLSL get_min(const std::vector<TriangleForGLSL *> &triangles, int start,
                    int end) {
    PaddedVec3ForGLSL min = PaddedVec3ForGLSL{std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(), 0};
    for (int i = start; i < end; i++) {
        min = PaddedVec3ForGLSL{std::min(min.x, triangles[i]->min.x),
                          std::min(min.y, triangles[i]->min.y),
                          std::min(min.z, triangles[i]->min.z), 0};
    }
    return min;
}

PaddedVec3ForGLSL get_max(const std::vector<TriangleForGLSL *> &triangles, int start,
                    int end) {
    PaddedVec3ForGLSL max = PaddedVec3ForGLSL{-std::numeric_limits<float>::max(),
                                  -std::numeric_limits<float>::max(),
                                  -std::numeric_limits<float>::max(), 0};
    for (int i = start; i < end; i++) {
        max = PaddedVec3ForGLSL{std::max(max.x, triangles[i]->max.x),
                          std::max(max.y, triangles[i]->max.y),
                          std::max(max.z, triangles[i]->max.z), 0};
    }
    return max;
}

int get_next_coord(int coord) { return (coord + 1) % 3; }

float get_coord(int coord, const PaddedVec3ForGLSL &v) {
    if (coord == 0) {
        return v.x;
    } else if (coord == 1) {
        return v.y;
    } else {
        return v.z;
    }
}

Box triangles_to_box(std::vector<Box> &boxes,
                     std::vector<TriangleForGLSL *> &triangles, int start,
                     int end, int coord) {
    int span = end - start;

    if (span <= 8) {
        return Box(get_min(triangles, start, end),
                   get_max(triangles, start, end), -1, -1, start, end);
    }

    int mid = start + span / 2;
    std::nth_element(
        triangles.begin() + start, triangles.begin() + mid,
        triangles.begin() + end,
        [coord](const TriangleForGLSL *a, const TriangleForGLSL *b) {
            return get_coord(coord, a->min) < get_coord(coord, b->min);
        });

    boxes.emplace_back(
        triangles_to_box(boxes, triangles, start, mid, get_next_coord(coord)));
    int left = boxes.size() - 1;
    boxes.emplace_back(
        triangles_to_box(boxes, triangles, mid, end, get_next_coord(coord)));
    int right = boxes.size() - 1;

    return Box(PaddedVec3ForGLSL{std::min(boxes[left].min.x, boxes[right].min.x),
                           std::min(boxes[left].min.y, boxes[right].min.y),
                           std::min(boxes[left].min.z, boxes[right].min.z), 0},
               PaddedVec3ForGLSL{std::max(boxes[left].max.x, boxes[right].max.x),
                           std::max(boxes[left].max.y, boxes[right].max.y),
                           std::max(boxes[left].max.z, boxes[right].max.z), 0},
               left, right, start, end);
}

AABB *triangles_to_aabb(std::vector<Box> &boxes,
                        std::vector<TriangleForGLSL *> &triangles, int start,
                        int end, int coord) {
    int span = end - start;

    if (span <= 8) {
        PaddedVec3ForGLSL min = get_min(triangles, start, end);
        PaddedVec3ForGLSL max = get_max(triangles, start, end);
        boxes.emplace_back(Box(min, max, -1, -1, start, end));
        return new AABB{static_cast<int>(boxes.size() - 1)};
    }
    boxes.emplace_back(triangles_to_box(boxes, triangles, start, end, coord));
    return new AABB{static_cast<int>(boxes.size() - 1)};
}

void print_box(std::vector<Box> boxes, int box_id, size_t depth,
               std::vector<TriangleForGLSL *> &triangles) {
    for (size_t i = 0; i < depth; ++i) {
        std::cout << "  ";
    }
    const Box *box = &boxes[box_id];
    std::cout << "min x: " << box->min.x << ", y: " << box->min.y
              << ", z: " << box->min.z << ", max x: " << box->max.x
              << ", y: " << box->max.y << ", z: " << box->max.z
              << ", start: " << box->start << ", end: " << box->end
              << ", right_id: " << box->right_id
              << ", left_id: " << box->left_id << std::endl;
    if (box->left_id != -1) {
        print_box(boxes, box->right_id, depth + 1, triangles);
        print_box(boxes, box->left_id, depth + 1, triangles);
        return;
    }
    // print triangles
    for (int i = box->start; i < box->end; i++) {
        for (size_t j = 0; j < depth; ++j) {
            std::cout << "  ";
        }
        std::cout << "  ";
        print_triangle(Triangle{
            Vec3{triangles[i]->v1.x, triangles[i]->v1.y, triangles[i]->v1.z},
            Vec3{triangles[i]->v2.x, triangles[i]->v2.y, triangles[i]->v2.z},
            Vec3{triangles[i]->v3.x, triangles[i]->v3.y, triangles[i]->v3.z},
            Vec2{0.0f, 0.0f}, Vec2{0.0f, 0.0f}, Vec2{0.0f, 0.0f},
            std::numeric_limits<uint32_t>::max()});
    }
}
