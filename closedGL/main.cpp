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


    std::ifstream vertexShaderFile("vertex.vert"); 
    if (vertexShaderFile) {
        std::cout << "vertex shader file imported succesfully" << std::endl;
    }
    else {
        std::cout << "vertex shader file import failed" << std::endl;
        return -1;
    }
    std::stringstream vertexbuffer;
    vertexbuffer << vertexShaderFile.rdbuf();
    std::string vertexShaderString = vertexbuffer.str();
    const char* vertexShaderSource = vertexShaderString.c_str();
    vertexShaderFile.close();
    std::cout << "vertex shader: \n" << vertexShaderSource << std::endl;


    std::ifstream fragmentShaderFile("fragment.frag");
    if (fragmentShaderFile) {
        std::cout << "fragment shader file imported succesfully" << std::endl;
    }
    else {
        std::cout << "fragment shader file import failed" << std::endl;
        return -1;
    }
    std::stringstream fragmentbuffer;
    fragmentbuffer << fragmentShaderFile.rdbuf();
    std::string fragmentShaderString = fragmentbuffer.str();
    const char* fragmentShaderSource = fragmentShaderString.c_str();
    fragmentShaderFile.close();
    std::cout << "fragment shader: \n" << fragmentShaderSource << std::endl;




    glViewport(0, 0, windowWidth, windowHeight);

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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


    int uniform_WindowSize = glGetUniformLocation(shaderProgram, "WindowSize");
    glUniform2f(uniform_WindowSize, windowWidth, windowHeight);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    




    while (!glfwWindowShouldClose(window)) {


        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);

        processInput(window);
        glfwPollEvents();
        
    }
    

}