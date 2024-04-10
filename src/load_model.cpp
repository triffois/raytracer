#include "./load_model.hpp"
#include "./tiny_gltf.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

Vec3 make_vec3(const std::vector<double> &vec) {
    return Vec3{vec[0], vec[1], vec[2]};
}

Vec3 make_vec3(const float *vec) { return Vec3{vec[0], vec[1], vec[2]}; }

Vec4 make_vec4(const std::vector<double> &vec) {
    return Vec4{vec[0], vec[1], vec[2], vec[3]};
}

Vec4 make_vec4(const Vec3 &vec, double w) {
    return Vec4{vec.x, vec.y, vec.z, w};
}

Matrix4 make_matrix4(const std::vector<double> &vec) {
    Matrix4 matrix{};
    matrix.v1 = make_vec4(vec);
    matrix.v2 = Vec4{vec[4], vec[5], vec[6], vec[7]};
    matrix.v3 = Vec4{vec[8], vec[9], vec[10], vec[11]};
    matrix.v4 = Vec4{vec[12], vec[13], vec[14], vec[15]};
    return matrix;
}

Matrix4 mul_matrixes(Matrix4 m1, Matrix4 m2) {
    Matrix4 result{};
    result.v1.x = m1.v1.x * m2.v1.x + m1.v1.y * m2.v2.x + m1.v1.z * m2.v3.x +
                  m1.v1.w * m2.v4.x;
    result.v1.y = m1.v1.x * m2.v1.y + m1.v1.y * m2.v2.y + m1.v1.z * m2.v3.y +
                  m1.v1.w * m2.v4.y;
    result.v1.z = m1.v1.x * m2.v1.z + m1.v1.y * m2.v2.z + m1.v1.z * m2.v3.z +
                  m1.v1.w * m2.v4.z;
    result.v1.w = m1.v1.x * m2.v1.w + m1.v1.y * m2.v2.w + m1.v1.z * m2.v3.w +
                  m1.v1.w * m2.v4.w;

    result.v2.x = m1.v2.x * m2.v1.x + m1.v2.y * m2.v2.x + m1.v2.z * m2.v3.x +
                  m1.v2.w * m2.v4.x;
    result.v2.y = m1.v2.x * m2.v1.y + m1.v2.y * m2.v2.y + m1.v2.z * m2.v3.y +
                  m1.v2.w * m2.v4.y;
    result.v2.z = m1.v2.x * m2.v1.z + m1.v2.y * m2.v2.z + m1.v2.z * m2.v3.z +
                  m1.v2.w * m2.v4.z;
    result.v2.w = m1.v2.x * m2.v1.w + m1.v2.y * m2.v2.w + m1.v2.z * m2.v3.w +
                  m1.v2.w * m2.v4.w;

    result.v3.x = m1.v3.x * m2.v1.x + m1.v3.y * m2.v2.x + m1.v3.z * m2.v3.x +
                  m1.v3.w * m2.v4.x;
    result.v3.y = m1.v3.x * m2.v1.y + m1.v3.y * m2.v2.y + m1.v3.z * m2.v3.y +
                  m1.v3.w * m2.v4.y;
    result.v3.z = m1.v3.x * m2.v1.z + m1.v3.y * m2.v2.z + m1.v3.z * m2.v3.z +
                  m1.v3.w * m2.v4.z;
    result.v3.w = m1.v3.x * m2.v1.w + m1.v3.y * m2.v2.w + m1.v3.z * m2.v3.w +
                  m1.v3.w * m2.v4.w;

    result.v4.x = m1.v4.x * m2.v1.x + m1.v4.y * m2.v2.x + m1.v4.z * m2.v3.x +
                  m1.v4.w * m2.v4.x;
    result.v4.y = m1.v4.x * m2.v1.y + m1.v4.y * m2.v2.y + m1.v4.z * m2.v3.y +
                  m1.v4.w * m2.v4.y;
    result.v4.z = m1.v4.x * m2.v1.z + m1.v4.y * m2.v2.z + m1.v4.z * m2.v3.z +
                  m1.v4.w * m2.v4.z;
    result.v4.w = m1.v4.x * m2.v1.w + m1.v4.y * m2.v2.w + m1.v4.z * m2.v3.w +
                  m1.v4.w * m2.v4.w;

    return result;
}

Matrix4 compose_matrix(const Vec3 &translation, const Vec4 &rotation,
                       const Vec3 &scale) {
    Matrix4 matrix;

    // Translation matrix
    matrix.v1 = {1.0, 0.0, 0.0, translation.x};
    matrix.v2 = {0.0, 1.0, 0.0, translation.y};
    matrix.v3 = {0.0, 0.0, 1.0, translation.z};
    matrix.v4 = {0.0, 0.0, 0.0, 1.0};

    // Scale matrix
    Matrix4 scale_matrix;
    scale_matrix.v1 = {scale.x, 0.0, 0.0, 0.0};
    scale_matrix.v2 = {0.0, scale.y, 0.0, 0.0};
    scale_matrix.v3 = {0.0, 0.0, scale.z, 0.0};
    scale_matrix.v4 = {0.0, 0.0, 0.0, 1.0};

    // Quaternion to rotation matrix
    double q0 = rotation.w;
    double q1 = rotation.x;
    double q2 = rotation.y;
    double q3 = rotation.z;

    // First row of the rotation matrix
    double r00 = 2 * (q0 * q0 + q1 * q1) - 1;
    double r01 = 2 * (q1 * q2 - q0 * q3);
    double r02 = 2 * (q1 * q3 + q0 * q2);

    // Second row of the rotation matrix
    double r10 = 2 * (q1 * q2 + q0 * q3);
    double r11 = 2 * (q0 * q0 + q2 * q2) - 1;
    double r12 = 2 * (q2 * q3 - q0 * q1);

    // Third row of the rotation matrix
    double r20 = 2 * (q1 * q3 - q0 * q2);
    double r21 = 2 * (q2 * q3 + q0 * q1);
    double r22 = 2 * (q0 * q0 + q3 * q3) - 1;

    // Create the rotation matrix
    Matrix4 rot_matrix;
    rot_matrix.v1 = {r00, r01, r02, 0.0};
    rot_matrix.v2 = {r10, r11, r12, 0.0};
    rot_matrix.v3 = {r20, r21, r22, 0.0};
    rot_matrix.v4 = {0.0, 0.0, 0.0, 1.0};

    // Combine the matrices
    matrix = mul_matrixes(matrix, rot_matrix);
    matrix = mul_matrixes(matrix, scale_matrix);

    return matrix;
}

void print_json_node(const OurNode &node) {
    for (const auto &primitive : node.primitives) {
        std::cout << "    [[" << primitive.v1.x << ", " << primitive.v1.y
                  << ", " << primitive.v1.z << "], [" << primitive.v2.x << ", "
                  << primitive.v2.y << ", " << primitive.v2.z << "], ["
                  << primitive.v3.x << ", " << primitive.v3.y << ", "
                  << primitive.v3.z << "]]," << std::endl;
    }
    for (const auto &child : node.children) {
        print_json_node(child);
    }
}

void print_node(const OurNode &node, size_t depth) {
    std::string indent = "";
    for (size_t i = 0; i < depth; i++) {
        indent += "|";
    }
    std::cout << indent << "Node at (" << node.translation.x << ", "
              << node.translation.y << ", " << node.translation.z << ")"
              << std::endl;
    std::cout << indent << "  Rotation: (" << node.rotation.x << ", "
              << node.rotation.y << ", " << node.rotation.z << ", "
              << node.rotation.w << ")" << std::endl;
    std::cout << indent << "  Scale: (" << node.scale.x << ", " << node.scale.y
              << ", " << node.scale.z << ")" << std::endl;
    std::cout << indent << "  Primitives:" << std::endl;
    for (const auto &primitive : node.primitives) {
        std::cout << indent << "    [[" << primitive.v1.x << ", "
                  << primitive.v1.y << ", " << primitive.v1.z << "], ["
                  << primitive.v2.x << ", " << primitive.v2.y << ", "
                  << primitive.v2.z << "], [" << primitive.v3.x << ", "
                  << primitive.v3.y << ", " << primitive.v3.z << "]],"
                  << std::endl;
    }
    std::cout << indent << "  Children:" << std::endl;
    for (const auto &child : node.children) {
        print_node(child, depth + 1);
    }
}

void load_node(OurNode *parent, const tinygltf::Node &node, uint32_t node_index,
               const tinygltf::Model &model,
               std::vector<uint32_t> &index_buffer,
               std::vector<Vec3> &vertex_buffer, float global_scale) {
    auto new_node = OurNode{};

    // Generate local node matrix
    auto translation = Vec3{0.0F, 0.0F, 0.0F};
    if (node.translation.size() == 3) {
        translation = make_vec3(node.translation);
    }
    new_node.translation = translation;
    if (node.rotation.size() == 4) {
        Vec4 quad = make_vec4(node.rotation);
        new_node.rotation = quad;
    }
    auto scale = Vec3{1.0f, 1.0f, 1.0f};
    if (node.scale.size() == 3) {
        scale = make_vec3(node.scale);
    }
    new_node.scale = scale;
    if (node.matrix.size() == 16) {
        new_node.matrix = make_matrix4(node.matrix);
    } else {
        new_node.matrix = compose_matrix(translation, new_node.rotation, scale);
    }

    // Node with children
    if (!node.children.empty()) {
        for (const auto &child : node.children) {
            load_node(&new_node, model.nodes[child], child, model, index_buffer,
                      vertex_buffer, global_scale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        std::vector<Triangle> *primitives = new std::vector<Triangle>();

        uint32_t index_count = 0;
        uint32_t vertex_start = static_cast<uint32_t>(vertex_buffer.size());

        for (const auto &primitive : mesh.primitives) {
            {
                const tinygltf::Accessor &accessor =
                    model.accessors[primitive.indices];
                const tinygltf::BufferView &buffer_view =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    model.buffers[buffer_view.buffer];

                index_count = static_cast<uint32_t>(accessor.count);
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    auto *buf = new uint32_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        buffer_view.byteOffset],
                           accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        index_buffer.push_back(buf[index] + vertex_start);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    auto *buf = new uint16_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        buffer_view.byteOffset],
                           accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        index_buffer.push_back(buf[index] + vertex_start);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    auto *buf = new uint8_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        buffer_view.byteOffset],
                           accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        index_buffer.push_back(buf[index] + vertex_start);
                    }
                    delete[] buf;
                    break;
                }
                default:
                    std::cerr << "Index component type "
                              << accessor.componentType << " not supported!"
                              << std::endl;
                    return;
                }
            }
            {
                const tinygltf::Accessor &accessor =
                    model.accessors[primitive.attributes.find("POSITION")
                                        ->second];
                const tinygltf::BufferView &buffer_view =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    model.buffers[buffer_view.buffer];
                const float *positions = reinterpret_cast<const float *>(
                    &buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                // print indexBuffer
                for (size_t i = 0; i < index_count; i += 3) {
                    Vec3 v1 = make_vec3(&positions[index_buffer[i] * 3]);
                    Vec3 v2 = make_vec3(&positions[index_buffer[i + 1] * 3]);
                    Vec3 v3 = make_vec3(&positions[index_buffer[i + 2] * 3]);
                    Vec3 minv = Vec3{std::min(std::min(v1.x, v2.x), v3.x),
                                     std::min(std::min(v1.y, v2.y), v3.y),
                                     std::min(std::min(v1.z, v2.z), v3.z)};
                    Vec3 maxv = Vec3{std::max(std::max(v1.x, v2.x), v3.x),
                                     std::max(std::max(v1.y, v2.y), v3.y),
                                     std::max(std::max(v1.z, v2.z), v3.z)};
                    primitives->push_back(Triangle{v1, v2, v3, minv, maxv});
                }
            }
        }
        new_node.primitives = *primitives;
    }
    parent->children.push_back(new_node);
}

OurNode load_model(std::string filename) {
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF loader;

    std::string err;
    std::string warn;
    bool file_loaded;
    if (filename.substr(filename.size() - 4) != ".glb") {
        file_loaded =
            loader.LoadASCIIFromFile(&gltf_model, &err, &warn, filename);
    } else {
        file_loaded =
            loader.LoadBinaryFromFile(&gltf_model, &err, &warn, filename);
    }
    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }
    if (!err.empty()) {
        throw std::runtime_error(err.c_str());
    }
    if (!file_loaded) {
        throw std::runtime_error("Failed to parse glTF");
    }

    const tinygltf::Scene &scene =
        gltf_model
            .scenes[gltf_model.defaultScene > -1 ? gltf_model.defaultScene : 0];

    std::vector<uint32_t> index_buffer;
    std::vector<Vec3> vertex_buffer;
    float scale = 1.0f;

    OurNode root_node{};
    root_node.translation = Vec3{0.0f, 2.0f, -2.0f};
    root_node.scale = Vec3{1.0f, 1.0f, 1.0f};
    root_node.rotation = Vec4{0.0f, 0.22f, 0.8f, 0.0f};
    root_node.matrix = compose_matrix(root_node.translation, root_node.rotation,
                                      root_node.scale);

    for (const auto &node_idx : scene.nodes) {
        const tinygltf::Node node = gltf_model.nodes[node_idx];
        load_node(&root_node, node, node_idx, gltf_model, index_buffer,
                  vertex_buffer, scale);
    }

#ifdef DEBUG_PRINT
    std::cout << "[" << std::endl;
    print_node(rootNode);
    std::cout << "]" << std::endl;
#endif

    return root_node;
}

Vec3 add_vec3(const Vec3 &vec1, const Vec3 &vec2) {
    return Vec3{vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z};
}

Vec3ForGLSL transform4(const Matrix4 &matrix, const Vec3 &vector3) {
    Vec4 result = {};
    Vec4 vector = Vec4{vector3.x, vector3.y, vector3.z, 1.0f};
    result.x = matrix.v1.x * vector.x + matrix.v1.y * vector.y +
               matrix.v1.z * vector.z + matrix.v1.w * vector.w;
    result.y = matrix.v2.x * vector.x + matrix.v2.y * vector.y +
               matrix.v2.z * vector.z + matrix.v2.w * vector.w;
    result.z = matrix.v3.x * vector.x + matrix.v3.y * vector.y +
               matrix.v3.z * vector.z + matrix.v3.w * vector.w;
    result.w = matrix.v4.x * vector.x + matrix.v4.y * vector.y +
               matrix.v4.z * vector.z + matrix.v4.w * vector.w;
    return Vec3ForGLSL{static_cast<float>(result.x),
                       static_cast<float>(result.y),
                       static_cast<float>(result.z)};
}

Vec3ForGLSL transform4(const Matrix4 &matrix, const Vec3ForGLSL &vector3) {
    Vec4 result = {};
    Vec4 vector = Vec4{vector3.x, vector3.y, vector3.z, 1.0f};
    result.x = matrix.v1.x * vector.x + matrix.v1.y * vector.y +
               matrix.v1.z * vector.z + matrix.v1.w * vector.w;
    result.y = matrix.v2.x * vector.x + matrix.v2.y * vector.y +
               matrix.v2.z * vector.z + matrix.v2.w * vector.w;
    result.z = matrix.v3.x * vector.x + matrix.v3.y * vector.y +
               matrix.v3.z * vector.z + matrix.v3.w * vector.w;
    result.w = matrix.v4.x * vector.x + matrix.v4.y * vector.y +
               matrix.v4.z * vector.z + matrix.v4.w * vector.w;
    return Vec3ForGLSL{static_cast<float>(result.x),
                       static_cast<float>(result.y),
                       static_cast<float>(result.z)};
}

std::vector<TriangleForGLSL> node_to_triangles(const OurNode &node) {
    std::vector<TriangleForGLSL> triangles = {};
    for (const auto &primitive : node.primitives) {
        Vec3ForGLSL v1_transformed = transform4(node.matrix, primitive.v1);
        Vec3ForGLSL v2_transformed = transform4(node.matrix, primitive.v2);
        Vec3ForGLSL v3_transformed = transform4(node.matrix, primitive.v3);
        Vec3ForGLSL min_transformed = transform4(node.matrix, primitive.min);
        Vec3ForGLSL max_transformed = transform4(node.matrix, primitive.max);
        triangles.push_back(TriangleForGLSL{v1_transformed, v2_transformed,
                                            v3_transformed, min_transformed,
                                            max_transformed});
    }
    for (const auto &child : node.children) {
        std::vector<TriangleForGLSL> new_triangles = node_to_triangles(child);
        // print matrix
        for (auto &triangle : new_triangles) {
            triangle.v1 = transform4(node.matrix, triangle.v1);
            triangle.v2 = transform4(node.matrix, triangle.v2);
            triangle.v3 = transform4(node.matrix, triangle.v3);
            triangle.min = transform4(node.matrix, triangle.min);
            triangle.max = transform4(node.matrix, triangle.max);
        }
        triangles.reserve(triangles.size() +
                          distance(new_triangles.begin(), new_triangles.end()));
        triangles.insert(triangles.end(), new_triangles.begin(),
                         new_triangles.end());
    }
    return triangles;
}
