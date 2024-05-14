#include "./load_model.hpp"
#include <algorithm>
#include <vector>

struct Box {
    Box(PaddedVec3ForGLSL min, PaddedVec3ForGLSL max, int left_id, int right_id, int start,
        int end)
        : min(min), max(max), left_id(left_id), right_id(right_id),
          start(start), end(end) {}
    PaddedVec3ForGLSL min;
    PaddedVec3ForGLSL max;
    int left_id;
    int right_id;
    int start;
    int end;
};

struct AABB {
    int root_id;
};

void print_triangle(const Triangle &t);

int get_next_coord(int coord);

float get_coord(int coord, const PaddedVec3ForGLSL &v);

Box triangles_to_box(std::vector<Box> &boxes,
                     std::vector<TriangleForGLSL *> &triangles, int start,
                     int end, int coord);

AABB *triangles_to_aabb(std::vector<Box> &boxes,
                        std::vector<TriangleForGLSL *> &triangles, int start,
                        int end, int coord);

void print_box(std::vector<Box> boxes, int box_id, size_t depth,
               std::vector<TriangleForGLSL *> &triangles);
