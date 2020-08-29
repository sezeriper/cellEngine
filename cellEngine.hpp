#pragma once
#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include <functional>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

const GLchar* fs = R"(
#version 460

out vec4 outColor;
in vec4 gs_color;

void main() {
    outColor = gs_color/255;
}
)";

const GLchar* gs = R"(
#version 460

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in uvec3 vs_color[];
out vec4 gs_color;

uniform mat4 projection;

void makeRect(vec4 position) {
    gl_Position = projection * (position + vec4(0.0f, 0.0f, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(0.0f, 1.0f, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(1.0f, 0.0f, 0.0f, 0.0f));
    EmitVertex();
    gl_Position = projection * (position + vec4(1.0f, 1.0f, 0.0f, 0.0f));
    EmitVertex();

    EndPrimitive();
}

void main() {
    gs_color = vec4(vs_color[0], 1.0f);
    makeRect(gl_in[0].gl_Position);
}
)";

const GLchar* vs = R"(
#version 460

layout(location = 0) in ivec2 position;
layout(location = 1) in uvec3 color;

out uvec3 vs_color;

void main() {
    gl_Position = vec4(position, 0.0f, 1.0f);
    vs_color = color;
}
)";

template<class T>
class Grid {
protected:
    int m_rows = 0;
    int m_cols = 0;
    int size = 0;
    T* data = nullptr;
    
public:
    Grid() {}
    ~Grid() {
        delete[] data;
    }

    Grid(const int rows, const int cols) { resize(rows, cols); }
    
    Grid(Grid<T>&& rGrid) : m_rows(rGrid.m_rows), m_cols(rGrid.m_cols), size(rGrid.size), data(rGrid.data) {
        rGrid.m_rows = 0;
        rGrid.m_cols = 0;
        rGrid.size = 0;
        rGrid.data = nullptr; 
    }

    void fill(const T& value) { std::fill_n(data, size, value); }

    void resize(const int rows, const int cols) {
        if (rows == m_rows && cols == m_cols) return;

        if (rows * cols == m_rows * m_cols) {
            std::swap(m_rows, m_cols);
            return;
        }

        m_rows = rows;
        m_cols = cols;
        size = rows * cols;

        delete[] data;
        data = new T[size];
    }

    Grid<T>& operator=(const Grid<T>& other) {
        if(this != &other) {

            resize(other.size);
            std::copy_n(other.data, size, data);
        }
        return *this;
    }

    Grid<T>& operator=(Grid<T>&& other) {
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        size = other.size;
        data = other.data;

        other.m_rows = 0;
        other.m_cols = 0;
        other.size = 0;
        other.data = nullptr;

        return *this;
    }

    int get_size() const { return size; }
    const T* get_data() const { return data; }

    T get(const int x, const int y) const { return data[y * m_cols + x]; }
    void set(const int x, const int y, T val) { data[y * m_cols + x] = val; }
};

class Shader {
private:
    GLuint programID;
    GLint projectionLocation;

    void errorCheck(GLuint ID) {

        GLint result = GL_FALSE;
        GLint infosize;

        glGetShaderiv(ID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infosize);

        if(infosize > 0) {
            std::vector<char> ErrorMessage(infosize + 1);
            glGetShaderInfoLog(ID, infosize, nullptr, &ErrorMessage[0]);
            std::cerr << &ErrorMessage[0];
        }
    }
public:
    Shader() {
        programID = compile();
        enable();
    }

    GLuint compile() {
        GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
        GLuint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertexShader, 1, &vs, nullptr);
        glCompileShader(vertexShader);
        errorCheck(vertexShader);

        glShaderSource(geometryShader, 1, &gs, nullptr);
        glCompileShader(geometryShader);
        errorCheck(geometryShader);

        glShaderSource(fragmentShader, 1, &fs, nullptr);
        glCompileShader(fragmentShader);
        errorCheck(fragmentShader);

        GLuint program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, geometryShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(geometryShader);
        glDeleteShader(fragmentShader);

        projectionLocation = glGetUniformLocation(program, "projection");

        return program;
    }
public:
    void enable() {
        glUseProgram(programID);
    }

    void disable() {
        glUseProgram(0);
    }

    void setProjection(const glm::mat4& matrix) const {
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &matrix[0][0]);
    }
};

class ColorGrid : public Grid<glm::u8vec3> {
private:
    const Shader& cellShader;

    GLuint vao;
    GLuint vbo[2];
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
        
public:

    ColorGrid(const int rows, const int cols, const Shader& shaderProgram): cellShader(shaderProgram) {
        m_rows = rows;
        m_cols = cols;
        size = rows * cols;
        
        glGenBuffers(2, &vbo[0]);

        glCreateVertexArrays(1, &vao);

        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);

        glVertexArrayAttribIFormat(vao, 0, 2, GL_UNSIGNED_INT, 0);
        glVertexArrayAttribIFormat(vao, 1, 3, GL_UNSIGNED_BYTE, 0);

        glVertexArrayVertexBuffer(vao, 0, vbo[0], 0, sizeof(glm::uvec2));
        glVertexArrayVertexBuffer(vao, 1, vbo[1], 0, sizeof(glm::u8vec3));

        glVertexArrayAttribBinding(vao, 0, 0);
        glVertexArrayAttribBinding(vao, 1, 1);

        Grid<glm::uvec2> positions(rows, cols);

        for(int i = 0; i < cols; i++) {
            for(int j = 0; j < rows; j++) {
                positions.set(i, j, glm::uvec2(i, j));
            }
        }

        glNamedBufferStorage(vbo[0], size * sizeof(glm::uvec2), positions.get_data(), GL_MAP_READ_BIT);
        glNamedBufferStorage(vbo[1], size * sizeof(glm::u8vec3), nullptr, flags);
        
        glBindVertexArray(vao);

        data = (glm::u8vec3*)glMapNamedBufferRange(vbo[1], 0, size * sizeof(glm::u8vec3), flags);
    }

    void render() const {
        const glm::mat4 proj = glm::ortho(0.0f, (float)m_rows, (float)m_cols, 0.0f);
        cellShader.setProjection(proj);

        glDrawArrays(GL_POINTS, 0, size);
    }

    ~ColorGrid() {
        data = nullptr;
        glUnmapNamedBuffer(vbo[1]);
        glBindVertexArray(0);
        glDeleteBuffers(2, &vbo[0]);
        glDeleteVertexArrays(1, &vao);
    }
};

class Window {
private:
    GLFWwindow *window_ptr = nullptr;

    double scroll = 0.0;
    bool updated = false;

    bool keys[512];

    static void errorCallback(int error, const char* description) {
        std::cerr << "Error(" << error << "): " << description << "\n";
    }

    static void keyCallback(GLFWwindow* window_p, int key, int, int action, int) {
        Window* windowPtr = (Window*)glfwGetWindowUserPointer(window_p);

        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window_p, GL_TRUE);
        else if(action == GLFW_PRESS)
            windowPtr->keys[key] = true;
        else if(action == GLFW_RELEASE)
            windowPtr->keys[key] = false;
    }

    static void scroll_callback(GLFWwindow* window_p, double, double yoffset) {
        Window* windowPtr = (Window*)glfwGetWindowUserPointer(window_p);
        windowPtr->scroll = yoffset;
        windowPtr->updated = true;
    }

public:
    Window(const unsigned int width, const unsigned int height, const char* title) {

        if(!glfwInit())
            std::cerr << "glfw is not ok\n";

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window_ptr = glfwCreateWindow(width, height, title, nullptr, nullptr);
            
        if(!window_ptr) {
            std::cerr << "can't create window\n";
            glfwTerminate();
        }

        glfwSetWindowUserPointer(window_ptr, this);
        glfwMakeContextCurrent(window_ptr);
        glfwSwapInterval(0);

        glfwSetErrorCallback(errorCallback);
        glfwSetKeyCallback(window_ptr, keyCallback);
        glfwSetScrollCallback(window_ptr, scroll_callback);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cerr << "glad ins't ok\n";
        }else 
            std::cout << "Opengl version: " << glGetString(GL_VERSION) << "\n";

        std::fill_n(&keys[0], sizeof(keys) / sizeof(bool), false);
    }

    void close() const {
        glfwSetWindowShouldClose(window_ptr, GLFW_TRUE);
    }

    bool shouldClose() const {
        return glfwWindowShouldClose(window_ptr);
    }

    double getTime() const {
        return glfwGetTime();
    }

    bool getKey(int key) const {
        return keys[key];
    }

    double getScroll() {
        if(updated) {
            updated = false;
            return scroll;
        }
        return 0;
    }

    void update() const {
        GLenum err = glGetError();

        if(err != GL_NO_ERROR) {
            std::cout << "OpenGL error: " << err << "\n";
        }

        glfwPollEvents();
        glfwSwapBuffers(window_ptr);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    ~Window() {
        glfwDestroyWindow(window_ptr);
        glfwTerminate();
    }
};

class cellEngine {
public:
    const Window window;
private:
    const Shader shader;
public:
    ColorGrid cells;

    std::function<void()> update;

    cellEngine(int width, int height, int cellSize, const char* title) :
        window(width * cellSize, height * cellSize, title),
        shader(),
        cells(width, height, shader) {}

    void mainLoop() {
        while (!window.shouldClose()) {
            double _time = window.getTime();

            update();

            cells.render();
            window.update();

            double frameTime = window.getTime() - _time;
            std::chrono::duration<double, std::milli> sleepDuration((1000.0/30.0) - frameTime);
            std::this_thread::sleep_for(sleepDuration);
            std::cout << 1.0 / frameTime << "\n";
        }
    }
};