#include "./load_model.hpp"
#include "./tiny_gltf.h"
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
    std::cout << indent << "  Rotation: (" << node.rotation.x1 << ", "
              << node.rotation.x2 << ", " << node.rotation.x3 << ", "
              << node.rotation.x4 << ")" << std::endl;
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
        if (global_scale != 1.0F) {
            // newNode->matrix = glm::scale(newNode->matrix,
            // glm::vec3(globalscale));
        }
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
                for (size_t i = 0; i < index_buffer.size(); i += 3) {
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

    OurNode rootNode{};
    rootNode.translation = Vec3{0.0f, 0.0f, 0.0f};
    rootNode.scale = Vec3{1.0f, 1.0f, 1.0f};

    for (const auto &node_idx : scene.nodes) {
        const tinygltf::Node node = gltf_model.nodes[node_idx];
        load_node(&rootNode, node, node_idx, gltf_model, index_buffer,
                  vertex_buffer, scale);
    }

#ifdef DEBUG_PRINT
    std::cout << "[" << std::endl;
    printNode(rootNode, 0);
    std::cout << "]" << std::endl;
#endif

    return rootNode;
}
