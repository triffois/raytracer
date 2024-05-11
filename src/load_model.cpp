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
    return Matrix4{make_vec4(vec), Vec4{vec[4], vec[5], vec[6], vec[7]},
                   Vec4{vec[8], vec[9], vec[10], vec[11]},
                   Vec4{vec[12], vec[13], vec[14], vec[15]}};
}

Matrix4 mul_matrixes(const Matrix4 &m1, const Matrix4 &m2) {
    return Matrix4{Vec4{m1.v1.x * m2.v1.x + m1.v1.y * m2.v2.x +
                            m1.v1.z * m2.v3.x + m1.v1.w * m2.v4.x,
                        m1.v1.x * m2.v1.y + m1.v1.y * m2.v2.y +
                            m1.v1.z * m2.v3.y + m1.v1.w * m2.v4.y,
                        m1.v1.x * m2.v1.z + m1.v1.y * m2.v2.z +
                            m1.v1.z * m2.v3.z + m1.v1.w * m2.v4.z,
                        m1.v1.x * m2.v1.w + m1.v1.y * m2.v2.w +
                            m1.v1.z * m2.v3.w + m1.v1.w * m2.v4.w},
                   Vec4{m1.v2.x * m2.v1.x + m1.v2.y * m2.v2.x +
                            m1.v2.z * m2.v3.x + m1.v2.w * m2.v4.x,
                        m1.v2.x * m2.v1.y + m1.v2.y * m2.v2.y +
                            m1.v2.z * m2.v3.y + m1.v2.w * m2.v4.y,
                        m1.v2.x * m2.v1.z + m1.v2.y * m2.v2.z +
                            m1.v2.z * m2.v3.z + m1.v2.w * m2.v4.z,
                        m1.v2.x * m2.v1.w + m1.v2.y * m2.v2.w +
                            m1.v2.z * m2.v3.w + m1.v2.w * m2.v4.w},
                   Vec4{m1.v3.x * m2.v1.x + m1.v3.y * m2.v2.x +
                            m1.v3.z * m2.v3.x + m1.v3.w * m2.v4.x,
                        m1.v3.x * m2.v1.y + m1.v3.y * m2.v2.y +
                            m1.v3.z * m2.v3.y + m1.v3.w * m2.v4.y,
                        m1.v3.x * m2.v1.z + m1.v3.y * m2.v2.z +
                            m1.v3.z * m2.v3.z + m1.v3.w * m2.v4.z,
                        m1.v3.x * m2.v1.w + m1.v3.y * m2.v2.w +
                            m1.v3.z * m2.v3.w + m1.v3.w * m2.v4.w},
                   Vec4{m1.v4.x * m2.v1.x + m1.v4.y * m2.v2.x +
                            m1.v4.z * m2.v3.x + m1.v4.w * m2.v4.x,
                        m1.v4.x * m2.v1.y + m1.v4.y * m2.v2.y +
                            m1.v4.z * m2.v3.y + m1.v4.w * m2.v4.y,
                        m1.v4.x * m2.v1.z + m1.v4.y * m2.v2.z +
                            m1.v4.z * m2.v3.z + m1.v4.w * m2.v4.z,
                        m1.v4.x * m2.v1.w + m1.v4.y * m2.v2.w +
                            m1.v4.z * m2.v3.w + m1.v4.w * m2.v4.w}};
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

void print_triangle(const Triangle &t) {
    std::cout << "[";
    std::cout << "[";
    std::cout << t.v1.x << ", " << t.v1.y << ", " << t.v1.z;
    std::cout << "],";
    std::cout << "[";
    std::cout << t.v2.x << ", " << t.v2.y << ", " << t.v2.z;
    std::cout << "],";
    std::cout << "[";
    std::cout << t.v3.x << ", " << t.v3.y << ", " << t.v3.z;
    std::cout << "]";
    std::cout << "]," << std::endl;
}

void print_json_node(const OurNode &node) {
    for (const auto &primitive : node.primitives) {
        std::cout << "    ";
        print_triangle(primitive);
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
        std::cout << indent << "    ";
        print_triangle(primitive);
    }
    std::cout << indent << "  Children:" << std::endl;
    for (const auto &child : node.children) {
        print_node(child, depth + 1);
    }
}

uint32_t find_texture(const tinygltf::Texture *tex,
                      const tinygltf::Model &model) {
    if (tex == nullptr) {
        return 0;
    }

    if (tex->name.empty()) {
        return 0;
    }

    uint32_t res = -1;
    for (const auto &texture : model.textures) {
        res++;
        if (tex->name == texture.name) {
            return res;
        }
    }

    return res;
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

        uint32_t index_count = 0;
        uint32_t vertex_start = static_cast<uint32_t>(vertex_buffer.size());

        for (const auto &primitive : mesh.primitives) {
            if (primitive.indices == -1) {
                std::cout << "Warning: primitive.indices == -1; skipping"
                          << std::endl;
                continue;
            }
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
                        index_buffer.emplace_back(buf[index] + vertex_start);
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
                        index_buffer.emplace_back(buf[index] + vertex_start);
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
                        index_buffer.emplace_back(buf[index] + vertex_start);
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
                const float *texture_coords = nullptr;
                if (primitive.attributes.find("TEXCOORD_0") !=
                    primitive.attributes.end()) {
                    const tinygltf::Accessor &uv_accessor =
                        model.accessors[primitive.attributes.find("TEXCOORD_0")
                                            ->second];
                    const tinygltf::BufferView &uv_view =
                        model.bufferViews[uv_accessor.bufferView];
                    texture_coords = reinterpret_cast<const float *>(
                        &(model.buffers[uv_view.buffer]
                              .data[uv_accessor.byteOffset +
                                    uv_view.byteOffset]));
                }
                for (size_t i = 0; i < index_count; i += 3) {
                    Vec3 v1 = make_vec3(&positions[index_buffer[i] * 3]);
                    Vec3 v2 = make_vec3(&positions[index_buffer[i + 1] * 3]);
                    Vec3 v3 = make_vec3(&positions[index_buffer[i + 2] * 3]);
                    Vec2 uv1;
                    Vec2 uv2;
                    Vec2 uv3;
                    uint32_t texture_id = std::numeric_limits<uint32_t>::max();
                    uint32_t metallic_roughness_texture_id =
                        std::numeric_limits<uint32_t>::max();
                    double metallic_factor = 0.0;
                    double roughness_factor = 0.0;
                    double alpha_cutoff = 0.0;
                    bool double_sided = false;
                    if (texture_coords != nullptr) {
                        uv1 = Vec2{texture_coords[index_buffer[i] * 2],
                                   texture_coords[index_buffer[i] * 2 + 1]};
                        uv2 = Vec2{texture_coords[index_buffer[i + 1] * 2],
                                   texture_coords[index_buffer[i + 1] * 2 + 1]};
                        uv3 = Vec2{
                            texture_coords[index_buffer[i + 2] * 2],
                            texture_coords[index_buffer[i + 2] * 2 + 1],
                        };
                        texture_id =
                            model.materials[primitive.material]
                                .pbrMetallicRoughness.baseColorTexture.index;
                        metallic_roughness_texture_id =
                            model.materials[primitive.material]
                                .pbrMetallicRoughness.metallicRoughnessTexture
                                .index;
                    }
                    metallic_factor = model.materials[primitive.material]
                                          .pbrMetallicRoughness.metallicFactor;
                    roughness_factor =
                        model.materials[primitive.material]
                            .pbrMetallicRoughness.roughnessFactor;
                    alpha_cutoff =
                        model.materials[primitive.material].alphaCutoff;
                    double_sided =
                        model.materials[primitive.material].doubleSided;
                    Triangle triangle{v1,
                                      v2,
                                      v3,
                                      uv1,
                                      uv2,
                                      uv3,
                                      texture_id,
                                      metallic_roughness_texture_id,
                                      metallic_factor,
                                      roughness_factor,
                                      alpha_cutoff,
                                      double_sided};
                    new_node.primitives.emplace_back(triangle);
                }
            }
        }
    }
    parent->children.emplace_back(new_node);
}

OurNode load_model(std::string filename) {
    tinygltf::Model gltf_model;
    tinygltf::TinyGLTF loader;
    OurNode root_node{};

    std::string err;
    std::string warn;
    bool file_loaded;
    if (filename.substr(filename.size() - 4) != ".glb") {
        file_loaded =
            loader.LoadASCIIFromFile(&gltf_model, &err, &warn, filename);
        if (gltf_model.images.size() == 0) {
        } else
            for (auto &image : gltf_model.images) {
                root_node.images.emplace_back(image);
            }
    } else {
        file_loaded =
            loader.LoadBinaryFromFile(&gltf_model, &err, &warn, filename);
        if (gltf_model.images.size() == 0) {
        } else
            for (auto &image : gltf_model.images) {
                root_node.images.emplace_back(image);
            }
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

    root_node.translation = Vec3{0.0f, 0.0f, -0.0f};
    root_node.scale = Vec3{1.0f, 1.0f, 1.0f};
    root_node.rotation = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
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
    return Vec3ForGLSL{
        static_cast<float>(matrix.v1.x * vector3.x + matrix.v1.y * vector3.y +
                           matrix.v1.z * vector3.z + matrix.v1.w * 1.0f),
        static_cast<float>(matrix.v2.x * vector3.x + matrix.v2.y * vector3.y +
                           matrix.v2.z * vector3.z + matrix.v2.w * 1.0f),
        static_cast<float>(matrix.v3.x * vector3.x + matrix.v3.y * vector3.y +
                           matrix.v3.z * vector3.z + matrix.v3.w * 1.0f),
        0};
}

Vec3ForGLSL transform4(const Matrix4 &matrix, const Vec3ForGLSL &vector) {
    return Vec3ForGLSL{
        static_cast<float>(matrix.v1.x * vector.x + matrix.v1.y * vector.y +
                           matrix.v1.z * vector.z + matrix.v1.w * 1.0f),
        static_cast<float>(matrix.v2.x * vector.x + matrix.v2.y * vector.y +
                           matrix.v2.z * vector.z + matrix.v2.w * 1.0f),
        static_cast<float>(matrix.v3.x * vector.x + matrix.v3.y * vector.y +
                           matrix.v3.z * vector.z + matrix.v3.w * 1.0f),
        0};
}

Vec3ForGLSL v3_min(const Vec3ForGLSL &v1, const Vec3ForGLSL &v2,
                   const Vec3ForGLSL &v3) {
    return Vec3ForGLSL{std::min(v1.x, std::min(v2.x, v3.x)),
                       std::min(v1.y, std::min(v2.y, v3.y)),
                       std::min(v1.z, std::min(v2.z, v3.z)), 0};
}

Vec3ForGLSL v3_max(const Vec3ForGLSL &v1, const Vec3ForGLSL &v2,
                   const Vec3ForGLSL &v3) {
    return Vec3ForGLSL{std::max(v1.x, std::max(v2.x, v3.x)),
                       std::max(v1.y, std::max(v2.y, v3.y)),
                       std::max(v1.z, std::max(v2.z, v3.z)), 0};
}

std::vector<TriangleForGLSL *> node_to_triangles(const OurNode &node) {
    std::vector<TriangleForGLSL *> triangles = {};
    for (const auto &primitive : node.primitives) {
        Vec3ForGLSL v1_transformed = transform4(node.matrix, primitive.v1);
        Vec3ForGLSL v2_transformed = transform4(node.matrix, primitive.v2);
        Vec3ForGLSL v3_transformed = transform4(node.matrix, primitive.v3);
        Vec3ForGLSL min_transformed =
            v3_min(v1_transformed, v2_transformed, v3_transformed);
        Vec3ForGLSL max_transformed =
            v3_max(v1_transformed, v2_transformed, v3_transformed);
        Vec2ForGLSL uv1 = Vec2ForGLSL{static_cast<float>(primitive.uv1.x),
                                      static_cast<float>(primitive.uv1.y)};
        Vec2ForGLSL uv2 = Vec2ForGLSL{static_cast<float>(primitive.uv2.x),
                                      static_cast<float>(primitive.uv2.y)};
        Vec2ForGLSL uv3 = Vec2ForGLSL{static_cast<float>(primitive.uv3.x),
                                      static_cast<float>(primitive.uv3.y)};
        uint32_t texture_id = primitive.texture_id;
        uint32_t metallic_roughness_texture_id =
            primitive.metallic_roughness_texture_id;
        float metallic_factor = static_cast<float>(primitive.metallic_factor);
        float roughness_factor = static_cast<float>(primitive.roughness_factor);
        float alpha_cutoff = static_cast<float>(primitive.alpha_cutoff);
        uint32_t double_sided = static_cast<uint32_t>(primitive.double_sided);
        triangles.emplace_back(new TriangleForGLSL{
            v1_transformed, v2_transformed, v3_transformed, min_transformed,
            max_transformed, uv1, uv2, uv3, texture_id,
            metallic_roughness_texture_id, metallic_factor, roughness_factor,
            alpha_cutoff, double_sided});
    }
    for (const auto &child : node.children) {
        std::vector<TriangleForGLSL *> new_triangles = node_to_triangles(child);
        for (auto &triangle : new_triangles) {
            triangle->v1 = transform4(node.matrix, triangle->v1);
            triangle->v2 = transform4(node.matrix, triangle->v2);
            triangle->v3 = transform4(node.matrix, triangle->v3);
            triangle->min = v3_min(triangle->v1, triangle->v2, triangle->v3);
            triangle->max = v3_max(triangle->v1, triangle->v2, triangle->v3);
        }
        triangles.reserve(triangles.size() + new_triangles.size());
        triangles.insert(triangles.end(),
                         std::make_move_iterator(new_triangles.begin()),
                         std::make_move_iterator(new_triangles.end()));
    }
    return triangles;
}
