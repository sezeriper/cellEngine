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
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

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
    int size = 0;
    int length = 0;
    T* gridPtr = nullptr;

    virtual int bound(int a) { return a; }
    
public:

    grid() : size(0), length(0), gridPtr(nullptr) {}

    grid(int size) { resize(size); }
    
    grid(grid<T>&& rGrid) : gridPtr(rGrid.gridPtr), size(rGrid.size), length(rGrid.length) {
        rGrid.gridPtr = nullptr; 
        rGrid.size = 0;
        rGrid.length = 0;
    }

    void fill(T data) { std::fill_n(gridPtr, length, data); }

    void resize(int newSize) {
        if(newSize != size) {
            int newLength = newSize*newSize;
            T* newGrid = new T[newLength]();

            std::copy(gridPtr, gridPtr+((newSize > size) ? length : newLength),  newGrid);
            delete[] gridPtr;

            size = newSize;
            length = newLength;
            gridPtr = newGrid;
        }
    }

    int getSize() { return size; }
    int getLength() { return length; }
    T* getPtr()   { return gridPtr; }

    void setSize(int size) {
        size = size;
        length = size*size;
    }
    void setPtr(T* ptr) { gridPtr = ptr; }

    grid<T>& operator=(grid<T>& other) {
        if(this != &other) {

            if(size != other.size)
                resize(other.size);

            std::copy(other.gridPtr, other.gridPtr+length, gridPtr);
        }
        return *this;
    }

    grid<T>& operator=(grid<T>&& other) {
        gridPtr = other.gridPtr;
        size = other.size;
        length = other.length;

        other.gridPtr = nullptr;
        other.size = 0;
        other.length = 0;

        return *this;
    }

    T& operator()(int x, int y) { return gridPtr[bound(y)*size+bound(x)]; }

    ~grid() { delete[] gridPtr; }
};

template<class T>
class endlessGrid : public grid<T> {
private:
    virtual int bound(int a) {
        if(a >= this->size) return this->bound(a-this->size);
        else if(a < 0) return this->bound(a+this->size);
        else return a;
    }
public:
    endlessGrid(int size) : grid<T>(size) {}
};

template<class T>
class borderGrid : public grid<T> {
private:
    virtual int bound(int a) {
        if(a >= this->size) return this->size-1;
        else if(a < 0) return 0;
        else return a;
    }
public:
    borderGrid() : grid<T>() {}
    borderGrid(int size) : grid<T>(size) {}
};

class cellEngine {
private:
    class shader {
    private:
        GLint programID;
        GLint projectionLocation;

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

            GLint succes;

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

        void init(const std::string& shadersPath) {
            programID = compile(shadersPath);
            enable();
        }

        shader() {}
    };

    class colorGrid {
    private:
        shader* cellShader;

        GLuint vao;
        GLuint vbo[2];
        GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
        
    public:
        borderGrid<glm::u8vec3> Colors;

        void init(int size, shader* shaderProgram) {
            cellShader = shaderProgram;
            Colors.resize(size);

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

            grid<glm::uvec2> positions(size);

            for(int i = 0; i < size; i++) {
                for(int j = 0; j < size; j++) {
                    positions(i, j) = glm::uvec2(i, j);
                }
            }

            glNamedBufferStorage(vbo[0], Colors.getLength() * sizeof(glm::uvec2), positions.getPtr(), GL_MAP_READ_BIT);
            glNamedBufferStorage(vbo[1], Colors.getLength() * sizeof(glm::u8vec3), nullptr, flags);
        
            glBindVertexArray(vao);

            Colors.setPtr((glm::u8vec3*)glMapNamedBufferRange(vbo[1], 0, Colors.getLength() * sizeof(glm::u8vec3), flags));
        }

        colorGrid() {}

        void render() {
            glm::mat4 proj = glm::ortho(0.0f, (float)Colors.getSize(), (float)Colors.getSize(), 0.0f);
            cellShader->setProjection(proj);

            glDrawArrays(GL_POINTS, 0, Colors.getLength());
        }

        ~colorGrid() {
            glUnmapNamedBuffer(vbo[1]);
            glBindVertexArray(0);
            glDeleteBuffers(2, &vbo[0]);
            glDeleteVertexArrays(1, &vao);
        }
    };

    class window {
    private:
        GLFWwindow *window_ptr = nullptr;

        double scroll;
        bool updated = false;

        bool keys[512];

        static void errorCallback(int error, const char* description) {

            std::cerr << "Error(" << error << "): " << description << "\n";
        }

        static void keyCallback(GLFWwindow* window_p, int key, int scancode, int action, int mods) {
            window* windowPtr = (window*)glfwGetWindowUserPointer(window_p);

            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window_p, GL_TRUE);
            else if(action == GLFW_PRESS)
                windowPtr->keys[key] = true;
            else if(action == GLFW_RELEASE)
                windowPtr->keys[key] = false;
        }

        static void scroll_callback(GLFWwindow* window_p, double xoffset, double yoffset) {
            window* windowPtr = (window*)glfwGetWindowUserPointer(window_p);
            windowPtr->scroll = yoffset;
            windowPtr->updated = true;
        }
    public:
        void init(const unsigned int width, const unsigned int height, const char* title) {

            if(!glfwInit()) {
                std::cerr << "glfw isn't ok\n";
            }

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

        window() {}

        bool shouldClose() const {
            return glfwWindowShouldClose(window_ptr);
        }

        void close() {
            glfwDestroyWindow(window_ptr);
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

        ~window() {
            glfwDestroyWindow(window_ptr);
            glfwTerminate();
        }
    };

public:
    shader Shader;
    window Window;
    colorGrid Cells;

    void init(int width, int height, const char* title, int gridSize) {
        Window.init(width, height, title);
        Shader.init(".\\src\\shaders\\shader");
        Cells.init(gridSize, &Shader);
    }

    virtual void update() = 0;

    void mainLoop() {

        while(!Window.shouldClose()) {
            double _time = Window.getTime();

            update();

            Cells.render();
            Window.update();

            double frameTime = Window.getTime() - _time;
            std::chrono::duration<double, std::milli> sleepDuration((1000.0/15.0) - frameTime);
            std::this_thread::sleep_for(sleepDuration);
        }
    }

    bool Key(int key) {
        return Window.getKey(key);
    }
};