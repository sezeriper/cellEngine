#pragma once
#include <iostream>
#include <time.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

const std::string getFile(const std::string& filePath) {

    std::ifstream file(filePath);
    std::stringstream stringStream;

    if(!file.is_open()) std::cerr << "error: file is not here: " << filePath << "\n";

    for(std::string line; getline(file, line);) {
        stringStream << line << "\n";
    }

    return stringStream.str();
}

template<class T>
class grid {
protected:
    int m_rows = 0;
    int m_cols = 0;
    int length = 0;
    T* gridPtr = nullptr;
    
public:
    grid() {}
    ~grid() {
        delete[] gridPtr;
        gridPtr = nullptr;
    }

    grid(int rows, int cols) { resize(rows, cols); }
    
    grid(grid<T>&& rGrid) : gridPtr(rGrid.gridPtr), m_rows(rGrid.m_rows), m_cols(rGrid.m_cols), length(rGrid.length) {
        rGrid.gridPtr = nullptr; 
        rGrid.m_rows = 0;
        rGrid.m_cols = 0;
        rGrid.length = 0;
    }

    void fill(T data) { std::fill_n(gridPtr, length, data); }

    void resize(int rows, int cols) {
        if (rows == m_rows && cols == m_cols) return;

        if (rows * cols == m_rows * m_cols) std::swap(m_rows, m_cols);

        m_rows = rows;
        m_cols = cols;
        length = rows * cols;

        gridPtr = new T[length];
    }

    grid<T>& operator=(grid<T>& other) {
        if(this != &other) {

            resize(other.size);
            std::copy_n(other.gridPtr, length, gridPtr);
        }
        return *this;
    }

    grid<T>& operator=(grid<T>&& other) noexcept {
        gridPtr = other.gridPtr;
        m_rows = other.m_rows;
        m_cols = other.m_cols;
        length = other.length;

        other.gridPtr = nullptr;
        other.m_rows = 0;
        other.m_cols = 0;
        other.length = 0;

        return *this;
    }

    const int getLength() const { return length; }
    const T* getPtr() const { return gridPtr; }

    const T get(int x, int y) { return gridPtr[y * m_cols + x]; }
    void set(int x, int y, T val) { gridPtr[y * m_cols + x] = val; }
};

class Shader {
private:
    GLint programID;
    GLint projectionLocation;

public:
    Shader(const std::string shadersPath) {
        programID = compile(shadersPath);
        enable();
    }

    void errorCheck(GLuint ID) {

        GLint result = GL_FALSE;
        int infoLength;

        glGetShaderiv(ID, GL_COMPILE_STATUS, &result);
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLength);

        if(infoLength > 0) {
            std::vector<char> ErrorMessage(infoLength+1);
            glGetShaderInfoLog(ID, infoLength, nullptr, &ErrorMessage[0]);
            std::cerr << &ErrorMessage[0];
        }
    }

    GLint compile(const std::string& shadersPath) {

        const std::string vertexShaderCode(getFile(shadersPath + ".vs"));
        const std::string geometryShaderCode(getFile(shadersPath + ".gs"));
        const std::string fragmentShaderCode(getFile(shadersPath + ".fs"));

        const GLchar* vertexShaderCode_ptr = vertexShaderCode.c_str();
        const GLchar* geometryShaderCode_ptr = geometryShaderCode.c_str();
        const GLchar* fragmentShaderCode_ptr = fragmentShaderCode.c_str();

        GLint program        = glCreateProgram();
        GLint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
        GLint geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertexShader, 1, &vertexShaderCode_ptr, nullptr);
        glCompileShader(vertexShader);
        errorCheck(vertexShader);

        glShaderSource(geometryShader, 1, &geometryShaderCode_ptr, nullptr);
        glCompileShader(geometryShader);
        errorCheck(geometryShader);

        glShaderSource(fragmentShader, 1, &fragmentShaderCode_ptr, nullptr);
        glCompileShader(fragmentShader);
        errorCheck(fragmentShader);

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

    void setProjection(glm::mat4& matrix) {
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &matrix[0][0]);
    }
};

class ColorGrid : public grid<glm::u8vec3> {
private:
    Shader* cellShader;

    GLuint vao;
    GLuint vbo[2];
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
        
public:

    ColorGrid(int rows, int cols, Shader* shaderProgram): cellShader(shaderProgram) {
        resize(rows, cols);

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

        grid<glm::uvec2> positions(rows, cols);

        for(int i = 0; i < cols; i++) {
            for(int j = 0; j < rows; j++) {
                positions.set(i, j, glm::uvec2(i, j));
            }
        }

        glNamedBufferStorage(vbo[0], length * sizeof(glm::uvec2), positions.getPtr(), GL_MAP_READ_BIT);
        glNamedBufferStorage(vbo[1], length * sizeof(glm::u8vec3), nullptr, flags);
        
        glBindVertexArray(vao);

        gridPtr = (glm::u8vec3*)glMapNamedBufferRange(vbo[1], 0, length * sizeof(glm::u8vec3), flags);
    }

    void render() {
        glm::mat4 proj = glm::ortho(0.0f, (float)m_rows, (float)m_cols, 0.0f);
        cellShader->setProjection(proj);

        glDrawArrays(GL_POINTS, 0, length);
    }

    ~ColorGrid() {
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

    static void keyCallback(GLFWwindow* window_p, int key, int scancode, int action, int mods) {
        Window* windowPtr = (Window*)glfwGetWindowUserPointer(window_p);

        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window_p, GL_TRUE);
        else if(action == GLFW_PRESS)
            windowPtr->keys[key] = true;
        else if(action == GLFW_RELEASE)
            windowPtr->keys[key] = false;
    }

    static void scroll_callback(GLFWwindow* window_p, double xoffset, double yoffset) {
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

        std::memset(&keys[0], false, sizeof(keys));
    }

    Window() : keys(), scroll(0.0) {}

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

    void update() {
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
    Window* window = nullptr;
    Shader* shader = nullptr;
    ColorGrid* cells = nullptr;

    cellEngine() { }

    virtual void start() = 0;
    virtual void update() = 0;

    void setup(int width, int height, int cellSize, const char* title) {
        window = new Window(width * cellSize, height * cellSize, title);
        shader = new Shader(".\\src\\shaders\\shader");
        cells = new ColorGrid(width, height, shader);
    }

    void mainLoop() {
        start();
        while (!window->shouldClose()) {
            double _time = window->getTime();

            update();

            cells->render();
            window->update();

            double frameTime = window->getTime() - _time;
            //std::chrono::duration<double, std::milli> sleepDuration((1000.0/5.0) - frameTime);
            //std::this_thread::sleep_for(sleepDuration);
            std::cout << 1.0 / frameTime << "\n";
        }
    }
};