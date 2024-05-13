// http://www.viva64.com
#include <ostream>
#include <vector>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define DEBUG_PRINT

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "./aabb.hpp"
#include "./controls.hpp"
#include "./load_model.hpp"
#include "./use_opengl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *read_shader(std::string filename)
{
    std::ifstream file(filename);
    std::string shader((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    char *cstr = new char[shader.length() + 1];
    std::strcpy(cstr, shader.c_str());
    return cstr;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0]
                  << " <shader file> [<gltf_file>...] [<glb_file>...]"
                  << std::endl;
        return 1;
    }
    std::string shader_path = argv[1];
    std::vector<TriangleForGLSL *> triangles;
    std::vector<tinygltf::Image> textures;
#ifdef DEBUG_PRINT
    auto start_model = std::chrono::high_resolution_clock::now();
#endif
    for (int i = 2; i < argc; ++i)
    {
        std::string path = argv[i];
        OurNode model = load_model(path);
        std::vector<TriangleForGLSL *> new_triangles = node_to_triangles(model);
        triangles.reserve(triangles.size() + new_triangles.size());
        triangles.insert(triangles.end(),
                         std::make_move_iterator(new_triangles.begin()),
                         std::make_move_iterator(new_triangles.end()));
        for (size_t j = 0; j < model.images.size(); ++j)
        {
            textures.emplace_back(model.images[j]);
        }
    }
#ifdef DEBUG_PRINT
    auto end_model = std::chrono::high_resolution_clock::now();
    std::cout << "Model loading took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_model - start_model)
                     .count()
              << "ms" << std::endl;
#endif

#ifdef DEBUG_PRINT_EXTENDED
    std::cout << "[" << std::endl;
    for (auto &t : triangles)
    {
        std::cout << "  ";
        Triangle triangle =
            Triangle{Vec3{t->v1.x, t->v1.y, t->v1.z}, Vec3{t->v2.x, t->v2.y, t->v2.z},
                     Vec3{t->v3.x, t->v3.y, t->v3.z}};
        print_triangle(triangle);
    }
    std::cout << "]" << std::endl;
#endif

#ifdef DEBUG_PRINT
    auto start_aabb = std::chrono::high_resolution_clock::now();
#endif
    std::vector<Box> boxes;
    AABB *aabb = triangles_to_aabb(boxes, triangles, 0, triangles.size(), 0);
#ifdef DEBUG_PRINT
    auto end_aabb = std::chrono::high_resolution_clock::now();
    std::cout << "AABB construction took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_aabb - start_aabb)
                     .count()
              << "ms" << std::endl;
#endif
#ifdef DEBUG_PRINT_EXTENDED
    print_box(boxes, aabb->root_id, 0, triangles);
#endif

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                          "MYOWNRAYTRACER!!!", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    int success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << info_log << std::endl;
    }
    // fragment shader
#ifdef DEBUG_PRINT
    auto start_fragment = std::chrono::high_resolution_clock::now();
#endif
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *shader_source = read_shader(shader_path);
    glShaderSource(fragment_shader, 1, &shader_source, NULL);
    glCompileShader(fragment_shader);
    // check for shader compile errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << info_log << std::endl;
    }
#ifdef DEBUG_PRINT
    auto end_fragment = std::chrono::high_resolution_clock::now();
    std::cout << "Fragment shader compilation took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_fragment - start_fragment)
                     .count()
              << "ms" << std::endl;
#endif
    // link shaders
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    // check for linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << info_log << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    delete[] shader_source;

    // configure textures
#ifdef DEBUG_PRINT
    auto start_texture = std::chrono::high_resolution_clock::now();
#endif
    if (textures.size() != 0)
    {
        float max_h;

        float max_w;
        std::vector<Vec2> ratios;
        for (int i = 0; i < textures.size(); i++)
        {
            if (textures[i].height > max_h)
            {
                max_h = textures[i].height;
            }
            if (textures[i].width > max_w)
            {
                max_w = textures[i].width;
            }
        }
        float ratio = max_h / max_w;
        Vec2 temp;
        for (int i = 0; i < textures.size(); i++)
        {
            temp.x = textures[i].width / max_w;
            temp.y = textures[i].height / max_h;
            ratios.push_back(temp);
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, max_h, max_w,
                       textures.size());
        for (size_t i = 0; i < textures.size(); ++i)
        {
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, textures[i].width,
                            textures[i].height, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            textures[i].image.data());
            std::cout << "Texture " << i << " size: " << textures[i].width << "x" << textures[i].height << std::endl;
        }
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if(ratios.size()%2 != 0){
            temp.x = 1;
            temp.y = 1;
            ratios.push_back(temp);
            }
        GLuint tex_ratios;
        glGenBuffers(1, &tex_ratios);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, tex_ratios);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ratios.size() * sizeof(Vec2),
                     ratios.data(), GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, tex_ratios);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

#ifdef DEBUG_PRINT
    auto end_texture = std::chrono::high_resolution_clock::now();
    std::cout << "Texture loading into opengl took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_texture - start_texture)
                     .count()
              << "ms" << std::endl;
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -1.0f,
        -1.0f,
        0.0f,
        -1.0f,
        3.0f,
        0.0f,
        3.0f,
        -1.0f,
        0.0f,
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
    // SSBO for vectors
    // triangles
    // copy triangles to array
    TriangleForGLSL *triangle_array = new TriangleForGLSL[triangles.size()];
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        triangle_array[i] = *triangles[i];
    }
    for (auto t : triangles)
    {
        delete t;
    }
#ifdef DEBUG_PRINT
    auto start_ssbo = std::chrono::high_resolution_clock::now();
#endif
    GLuint ssbo_triangles;
    glGenBuffers(1, &ssbo_triangles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 triangles.size() * sizeof(TriangleForGLSL), triangle_array,
                 GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_triangles);
    GLuint ssbo_boxes;
    glGenBuffers(1, &ssbo_boxes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_boxes);
    glBufferData(GL_SHADER_STORAGE_BUFFER, boxes.size() * sizeof(Box),
                 boxes.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_boxes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
#ifdef DEBUG_PRINT
    auto end_ssbo = std::chrono::high_resolution_clock::now();
    std::cout << "SSBO creation took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_ssbo - start_ssbo)
                     .count()
              << "ms" << std::endl;
#endif
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        process_input(window);
        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs(window);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shader_program);
        glBindVertexArray(VAO); // seeing as we only have a single VAO
        // there's
        // no need to bind it every time, but we'll
        // do
        // so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released,
        // mouse
        // moved etc.)
        //
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Set up uniforms
        float time_value = glfwGetTime();
        int screen_size_location =
            glGetUniformLocation(shader_program, "iResolution");
        // glUniform2f(screenSizeLocation, SCR_WIDTH, SCR_HEIGHT);
        // This only gets the original window size, not the actual size of
        // the
        // window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glUniform2f(screen_size_location, width, height);
        int timeLocation = glGetUniformLocation(shader_program, "iTime");
        glUniform1f(timeLocation, time_value);
        int frame_location = glGetUniformLocation(shader_program, "iFrame");
        glUniform1i(frame_location, frame++);
        int triangle_count_location =
            glGetUniformLocation(shader_program, "triangle_count");
        glUniform1i(triangle_count_location, triangles.size());
        int mvpLocation = glGetUniformLocation(shader_program, "MVP");
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &MVP[0][0]);

        // AABB
        int root_id_location = glGetUniformLocation(shader_program, "root_id");
        glUniform1i(root_id_location, aabb->root_id);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    delete aabb;
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina
    // displays.
    glViewport(0, 0, width, height);
}
