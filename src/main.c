/**
 * Clouds with OpenGL and GLFW
 *
 * Description:
 *     This program showcases a dynamic cloud rendering using OpenGL and GLFW.
 *     The cloud effect is generated using a combination of procedural noise
 *     functions in the fragment shader.
 *
 * Author: Michael Knap
 * Date: 5/11/2023
 *
 * License:
 *     MIT License.
 *
 *  * Dependencies:
 *     - GLFW3 library
 *     - GLEW
 *     - OpenGL libraries
 */

#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Vertex shader to transform vertices and pass texture coordinates.
const char* vertex_shader_source =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) in vec2 in_position;\n"
    "layout(location = 1) in vec2 in_tex_coord;\n"
    "out vec2 tex_coord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(in_position, 0.0, 1.0);\n"
    "    tex_coord = in_tex_coord;\n"
    "}\n";

// Fragment shader to generate dynamic cloud rendering effect.
const char* fragment_shader_source =
    "#version 330 core\n"
    "in vec2 tex_coord;\n"
    "out vec4 frag_color;\n"
    "uniform float cloud_shift;\n"
    "\n"
    "float smooth_noise(vec2 st) {\n"
    "    vec2 i = floor(st);\n"
    "    vec2 f = smoothstep(vec2(0.0), vec2(1.0), fract(st));\n"
    "    float a = dot(i, vec2(1.0, 57.0)) + 1.0;\n"
    "    float b = dot(i + vec2(1.0, 0.0), vec2(1.0, 57.0)) + 1.0;\n"
    "    float c = dot(i + vec2(0.0, 1.0), vec2(1.0, 57.0)) + 1.0;\n"
    "    float d = dot(i + vec2(1.0, 1.0), vec2(1.0, 57.0)) + 1.0;\n"
    "    float result = mix(mix(fract(sin(a) * 43758.5453), fract(sin(b) * 43758.5453), f.x),\n"
    "                       mix(fract(sin(c) * 43758.5453), fract(sin(d) * 43758.5453), f.x), f.y);\n"
    "    return result;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 st = tex_coord * 5.0;\n"
    "    st.x += cloud_shift * 0.15;\n"
    "    float n = smooth_noise(st);\n"
    "    float n1 = smooth_noise(st * 2.0 - cloud_shift * 0.05) * 0.5;\n"
    "    float n2 = smooth_noise(st * 4.0 - cloud_shift * 0.1) * 0.25;\n"
    "    n += n1 + n2;\n"
    "    float cloud = smoothstep(0.3, 1.0, n);\n"
    "    vec3 sky_color = vec3(0.602, 0.808, 0.980);\n"
    "    vec3 cloud_color = vec3(0.97);\n"
    "    vec3 final_color = mix(cloud_color, sky_color, cloud);\n"
    "    frag_color = vec4(final_color, 1.0);\n"
    "}\n";

// Simple 2D vector type for positions.
typedef struct {
    float x, y;
} vec2;

// (-1,1)     (1,1)
//    +---------+
//    |       / |
//    |     /   |
//    |   /     |
//    | /       |
//    +---------+
// (-1,-1)     (1,-1)

// Vertex data for a full-screen quad (two triangles covering the screen).
// Each vertex has a position and texture coordinate.
float vertices[] = {
    // positions     // texture coords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};

// Helper function to compile a shader from source.
GLuint compile_shader(const char* source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    const GLchar* shader_source = source;
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Error compiling shader: %s\n", info_log);
        return 0;
    }
    return shader;
}

int main() {
    // Initialize GLFW library.
    if (!glfwInit()) {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context.
    GLFWwindow* window = glfwCreateWindow(1280, 960, "Clouds", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current.
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Compile the vertex and fragment shaders.
    GLuint vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

    // Link the shaders into a single program.
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        fprintf(stderr, "Error linking shader program: %s\n", info_log);
    }

    // Get the uniform location for cloud_shift in the shader.
    GLint cloud_shift_loc = glGetUniformLocation(shader_program, "cloud_shift");

    // Clean up shaders as they're no longer needed.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Generate a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO).
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind the VAO to capture all subsequent vertex attribute configurations.
    glBindVertexArray(VAO);

    // Bind the VBO to the array buffer and populate it with the vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data. First, the positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Then, the texture coordinates.
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind the VBO and VAO for now.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize cloud shift for the animation in fragment shader.
    float cloud_shift = 0.0f;

    // Render loop.
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen buffer.
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the compiled shader program.
        glUseProgram(shader_program);

        // Update the cloud shift value and send it to the shader.
        // cloud_shift should probably be reset at some point
        // as it continues to increment with every frame..
        cloud_shift += 0.02f;
        glUniform1f(cloud_shift_loc, cloud_shift);

        // Bind the VAO (with the quad data) and render.
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Swap the screen buffers and poll for events.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and terminate GLFW.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}