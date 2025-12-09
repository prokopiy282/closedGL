#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <cmath>

#define FLOAT_SIZE 64
#define INT_SIZE 64


enum meshState : int {
    SQUARE = 0,
    TRIANGLE = 1,
    HOURGLASS = 2,
    CROSS = 3
};

meshState& operator++(meshState& state) {
    int newState = static_cast<int>(state);
    newState++;
    state = static_cast<meshState>(newState);
    return state;
}

enum meshState shape = SQUARE;


void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        ++shape;
    }
};


void makeVAO(unsigned int* vao, unsigned int* vbo, unsigned int* ebo, float* vertices, unsigned int* indices) {

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    glEnableVertexAttribArray(0);

    std::cout << "vao int: " << *vao << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vao created without error" << std::endl;
    }


    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, FLOAT_SIZE, vertices, GL_STATIC_DRAW);

    std::cout << "vbo int: " << *vbo << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vbo created without error" << std::endl;
    }


    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, INT_SIZE, indices, GL_STATIC_DRAW);

    std::cout << "ebo int: " << *ebo << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "ebo created without error" << std::endl;
    }


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vertex attrips pointed without error" << std::endl;
    }

}

//shader type is only for debug purposes, leave empty str if you dont care. r and i values wont stop being a bane of my existence
std::string getShaderSource(const char* path, std::string shaderType) {

    std::ifstream ShaderFile(path);

    if (ShaderFile) {
        std::cout << shaderType << " shader file imported succesfully" << std::endl;
    }
    else {
        std::cout << shaderType << " shader file import failed" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << ShaderFile.rdbuf();
    std::string ShaderString = buffer.str();

    ShaderFile.close();

    std::cout << shaderType << " shader: \n" << ShaderString << std::endl;

    return ShaderString;
}

template <class dataType>
void importArray(const char* path, dataType* array, int* size, std::string consoleName) {

    std::ifstream arrayFile(path);

    if (arrayFile) {
        std::cout << consoleName << " array file imported succesfully" << std::endl;
    }
    else {
        std::cout << consoleName << "array file import failed" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << arrayFile.rdbuf();
    std::string ShaderString = buffer.str();

    arrayFile.close();

}


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr int windowWidth = 640;
    constexpr int windowHeight = 480;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "black magic", NULL, NULL);
    if (!window) {
        std::cout << "window creation broke" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "glad init failed" << std::endl;
        return -1;
    }


    std::string vertexShaderSourceString = getShaderSource("vertex.vert", "vertex");
    const char* vertexShaderSource = vertexShaderSourceString.c_str();

    std::string fragmentShaderSourceString = getShaderSource("fragment.frag", "fragment");
    const char* fragmentShaderSource = fragmentShaderSourceString.c_str();


    glViewport(0, 0, windowWidth, windowHeight);

    float vertices[] = {
     0.5f, 0.5f, 0.0f,
     0.5f,-0.5f, 0.0f,
    -0.5f,-0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    };

    unsigned int indices[] = {
        0,1,3,
        1,2,3
    };

    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int* vaoPtr = &vao;
    unsigned int* vboPtr = &vbo;
    unsigned int* eboPtr = &ebo;

    makeVAO(vaoPtr, vboPtr, eboPtr, vertices, indices);

    std::cout << "vao int: " << vao << std::endl;
    std::cout << "vbo int: " << vbo << std::endl;
    std::cout << "ebo int: " << ebo << std::endl;

    int logLength = 0;
    const int maxlogLength = 90;
    char* log = new char[maxlogLength];
    int itLives;

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    std::cout << "vertex shader status:";
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";

    }
    else {
        glGetShaderInfoLog(vertexShader, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }


    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    std::cout << "fragment shader status:";
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";
    }
    else {
        glGetShaderInfoLog(fragmentShader, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    std::cout << "shader Program status:";
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";

    }
    else {
        glGetProgramInfoLog(shaderProgram, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    //TODO: shader sets
    static int uniform_windowSize = glGetUniformLocation(shaderProgram, "windowSize");
    glUniform2f(uniform_windowSize, windowWidth, windowHeight);

    float shaderTime = 0.0f; //TODO: add deltatime. also maybe a 32 bit float is a bit wasteful to store ints
    int uniform_shaderTime = glGetUniformLocation(shaderProgram, "shaderTime");
    glUniform1f(uniform_shaderTime, shaderTime);

    const float gammaCorrection = 0.45f; //2.2 gamma my beloved
    int uniform_gammaCorrection = glGetUniformLocation(shaderProgram, "gammaCorrection");
    glUniform1f(uniform_gammaCorrection, gammaCorrection);

    const float temporalResolution = 64.0f; //maybe casts IN the shader are not a good idea. also does deltatime if equal to fps
    int uniform_temporalResolution = glGetUniformLocation(shaderProgram, "temporalResolution");
    glUniform1f(uniform_temporalResolution, temporalResolution);



    auto lastTime = std::chrono::system_clock::now();


    glBindVertexArray(vao);



    while (!glfwWindowShouldClose(window)) {

        shaderTime++;
        glUniform1f(uniform_shaderTime, shaderTime);

        if (std::remainder(shaderTime, temporalResolution) == 0) {
            auto currentTime = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
            std::cout << "fps is:" << (temporalResolution/(elapsed.count()/1000.0f)) << std::endl;
            std::cout << "ms elapsed:" << elapsed.count() << std::endl;//why is it going up
            lastTime = currentTime;
        }

        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        processInput(window);
        glfwPollEvents();
        
    }
    

}