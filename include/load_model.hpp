#ifndef INCLUDE_LOAD_MODEL_HPP_
#define INCLUDE_LOAD_MODEL_HPP_
#include <cstdint>
#include <string>
#include <vector>

#include "./tiny_gltf.h"

struct Vec3 {
    double x;
    double y;
    double z;
};

struct Vec4 {
    double x1;
    double x2;
    double x3;
    double x4;
};

struct Matrix4 {
    Vec4 v1;
    Vec4 v2;
    Vec4 v3;
    Vec4 v4;
};

struct Triangle {
    Vec3 v1;
    Vec3 v2;
    Vec3 v3;
    Vec3 min;
    Vec3 max;
};

struct OurNode {
    Vec3 translation;
    Vec4 rotation;
    Vec3 scale;
    Matrix4 matrix;
    std::vector<OurNode> children;
    std::vector<Triangle> primitives;
};

Vec3 make_vec3(const std::vector<double> &vec);

Vec3 make_vec3(const float *vec);

Vec4 make_vec4(const std::vector<double> &vec);

Vec4 make_vec4(const Vec3 &vec, double w);

Matrix4 make_matrix4(const std::vector<double> &vec);

void print_json_node(const OurNode &node);

void print_node(const OurNode &node, size_t depth = 0);

void load_node(OurNode *parent, const tinygltf::Node &node, uint32_t node_index,
               const tinygltf::Model &model,
               std::vector<uint32_t> &index_buffer,
               std::vector<Vec3> &vertex_buffer, float global_scale);

OurNode load_model(std::string filename);

#endif // INCLUDE_LOAD_MODEL_HPP_