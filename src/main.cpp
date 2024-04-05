// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#include <algorithm>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <math.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "./tiny_gltf.h"
#include "./use_opengl.h"

struct Vec3 {
    double x;
    double y;
    double z;
};

Vec3 makeVec3(const std::vector<double> &vec) {
    return Vec3{vec[0], vec[1], vec[2]};
}

Vec3 makeVec3(const float *vec) { return Vec3{vec[0], vec[1], vec[2]}; }

struct Vec4 {
    double x1;
    double x2;
    double x3;
    double x4;
};

Vec4 makeVec4(const std::vector<double> &vec) {
    return Vec4{vec[0], vec[1], vec[2], vec[3]};
}

Vec4 makeVec4(const Vec3 &vec, double w) {
    return Vec4{vec.x, vec.y, vec.z, w};
}

struct Matrix4 {
    Vec4 v1;
    Vec4 v2;
    Vec4 v3;
    Vec4 v4;
};

Matrix4 makeMatrix4(const std::vector<double> &vec) {
    Matrix4 matrix{};
    matrix.v1 = makeVec4(vec);
    matrix.v2 = Vec4{vec[4], vec[5], vec[6], vec[7]};
    matrix.v3 = Vec4{vec[8], vec[9], vec[10], vec[11]};
    matrix.v4 = Vec4{vec[12], vec[13], vec[14], vec[15]};
    return matrix;
}

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

void printNode(OurNode node, size_t depth) {
    std::string indent = "";
    // for (size_t i = 0; i < depth; i++) {
    //     indent += "|";
    // }
    // std::cout << indent << "Node at (" << node.translation.x << ", "
    //           << node.translation.y << ", " << node.translation.z << ")"
    //           << std::endl;
    // std::cout << indent << "  Rotation: (" << node.rotation.x1 << ", "
    //           << node.rotation.x2 << ", " << node.rotation.x3 << ", "
    //           << node.rotation.x4 << ")" << std::endl;
    // std::cout << indent << "  Scale: (" << node.scale.x << ", " <<
    // node.scale.y
    //           << ", " << node.scale.z << ")" << std::endl;
    // std::cout << indent << "  Primitives:" << std::endl;
    for (const auto &primitive : node.primitives) {
        std::cout << indent << "    [[" << primitive.v1.x << ", "
                  << primitive.v1.y << ", " << primitive.v1.z << "], ["
                  << primitive.v2.x << ", " << primitive.v2.y << ", "
                  << primitive.v2.z << "], [" << primitive.v3.x << ", "
                  << primitive.v3.y << ", " << primitive.v3.z << "]],"
                  << std::endl;
    }
    // std::cout << indent << "  Children:" << std::endl;
    for (const auto &child : node.children) {
        printNode(child, depth + 1);
    }
}

void loadNode(OurNode *parent, const tinygltf::Node &node, uint32_t nodeIndex,
              const tinygltf::Model &model, std::vector<uint32_t> &indexBuffer,
              std::vector<Vec3> &vertexBuffer, float globalscale) {
    auto newNode = OurNode{};

    // Generate local node matrix
    auto translation = Vec3{0.0F, 0.0F, 0.0F};
    if (node.translation.size() == 3) {
        translation = makeVec3(node.translation);
    }
    newNode.translation = translation;
    if (node.rotation.size() == 4) {
        Vec4 quad = makeVec4(node.rotation);
        newNode.rotation = quad;
    }
    auto scale = Vec3{1.0f, 1.0f, 1.0f};
    if (node.scale.size() == 3) {
        scale = makeVec3(node.scale);
    }
    newNode.scale = scale;
    if (node.matrix.size() == 16) {
        newNode.matrix = makeMatrix4(node.matrix);
        if (globalscale != 1.0F) {
            // newNode->matrix = glm::scale(newNode->matrix,
            // glm::vec3(globalscale));
        }
    }

    // Node with children
    if (!node.children.empty()) {
        for (const auto &child : node.children) {
            loadNode(&newNode, model.nodes[child], child, model, indexBuffer,
                     vertexBuffer, globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh > -1) {
        const tinygltf::Mesh mesh = model.meshes[node.mesh];
        std::vector<Triangle> *primitives = new std::vector<Triangle>();

        uint32_t indexCount = 0;
        uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());

        for (const auto &primitive : mesh.primitives) {
            {
                const tinygltf::Accessor &accessor =
                    model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    model.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);
                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    auto *buf = new uint32_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    auto *buf = new uint16_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    auto *buf = new uint8_t[accessor.count];
                    memcpy(buf,
                           &buffer.data[accessor.byteOffset +
                                        bufferView.byteOffset],
                           accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
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
                const tinygltf::BufferView &bufferView =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer =
                    model.buffers[bufferView.buffer];
                const float *positions = reinterpret_cast<const float *>(
                    &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                // print indexBuffer
                for (size_t i = 0; i < indexBuffer.size(); i+=3) {
                    Vec3 v1 = makeVec3(&positions[indexBuffer[i] * 3]);
                    Vec3 v2 = makeVec3(&positions[indexBuffer[i + 1] * 3]);
                    Vec3 v3 = makeVec3(&positions[indexBuffer[i + 2] * 3]);
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
        newNode.primitives = *primitives;
    }
    parent->children.push_back(newNode);
}

int main(int argc, const char *argv[]) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;

    std::string filename = argv[1];
    std::string err;
    std::string warn;
    bool fileLoaded =
        loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
    // for binary glTF(.glb):
    // bool fileLoaded =
    //     loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }
    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }
    if (!fileLoaded) {
        printf("Failed to parse glTF\n");
        return -1;
    }

    const tinygltf::Scene &scene =
        gltfModel
            .scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

    std::vector<uint32_t> indexBuffer;
    std::vector<Vec3> vertexBuffer;
    float scale = 1.0f;

    OurNode rootNode{};
    rootNode.translation = Vec3{0.0f, 0.0f, 0.0f};
    rootNode.scale = Vec3{1.0f, 1.0f, 1.0f};

    for (const auto &node_idx : scene.nodes) {
        const tinygltf::Node node = gltfModel.nodes[node_idx];
        loadNode(&rootNode, node, node_idx, gltfModel, indexBuffer,
                 vertexBuffer, scale);
    }

    std::cout << "[" << std::endl;
    printNode(rootNode, 0);
    std::cout << "]" << std::endl;

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *readShader(std::string filename) {
    std::ifstream file(filename);
    std::string shader((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    char *cstr = new char[shader.length() + 1];
    std::strcpy(cstr, shader.c_str());
    return cstr;
}

int main_raytracer(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <shader file>" << std::endl;
        return 1;
    }
    std::string shaderPath = argv[1];

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                          "MYOWNRAYTRACER!!!", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *shaderSource = readShader(shaderPath);
    glShaderSource(fragmentShader, 1, &shaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, -1.0f, 3.0f, 0.0f, 3.0f, -1.0f, 0.0f,
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered
    // VBO as the vertex attribute's bound vertex buffer object so afterwards we
    // can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally
    // modify this VAO, but this rarely happens. Modifying other VAOs requires a
    // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
    // VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's
                                // no need to bind it every time, but we'll do
                                // so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Set up uniforms
        float timeValue = glfwGetTime();
        int screenSizeLocation =
            glGetUniformLocation(shaderProgram, "iResolution");
        // glUniform2f(screenSizeLocation, SCR_WIDTH, SCR_HEIGHT);
        // This only gets the original window size, not the actual size of the
        // window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glUniform2f(screenSizeLocation, width, height);
        int timeLocation = glGetUniformLocation(shaderProgram, "iTime");
        glUniform1f(timeLocation, timeValue);
        int frameLocation = glGetUniformLocation(shaderProgram, "iFrame");
        glUniform1i(frameLocation, frame++);

        glUseProgram(shaderProgram);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}
