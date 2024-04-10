#include "./load_model.hpp"
#include <algorithm>
#include <vector>

struct Box {
    Box(float min, float max, Box *left, Box *right, size_t start, size_t end,
        char coord)
        : min(min), max(max), left(left), right(right), start(start), end(end),
          coord(coord) {}
    float min;
    float max;
    Box *left;
    Box *right;
    size_t start;
    size_t end;
    char coord;
};

char get_next_coord(char coord);

float get_coord(char coord, const Vec3ForGLSL &v);

Box *triangles_to_aabb(std::vector<TriangleForGLSL> &triangles, size_t start,
                       size_t end, char coord);

void print_aabb(Box *box, size_t depth,
                std::vector<TriangleForGLSL> &triangles);
