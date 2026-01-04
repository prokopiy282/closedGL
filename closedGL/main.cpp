#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>

//todo: move with 2 modes (mouse and kb, tab toggle) 
//      make rate limiting for changing shape
//      dvd animation
//      linear interpolation between figures
//      fix makeVAO function and add memory to objects - check
//      fix up shader maker


#define INT_SIZE 4
#define FLOAT_SIZE 4
#define BYTE_ORDER_MARK "\xEF\xBB\xBF"


enum meshState {
    SQUARE,
    HOURGLASS,
    TRIANGLE,
    CROSS
};

meshState& operator++(meshState& state) {
    int newState = static_cast<int>(state);
    newState++;
    state = static_cast<meshState>(newState);
    return state;
}

meshState shape = SQUARE;

//warning: only null construct this
struct VAOobject {

    VAOobject() = default;

    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    unsigned int* vaoPtr = &vao;
    unsigned int* vboPtr = &vbo;
    unsigned int* eboPtr = &ebo;
    std::vector<float> verticesVec;
    std::vector<unsigned int> indicesVec; //limits max mesh size bcs uint. though the current state of tech probably limits it before that
    
    size_t getVertSize() {
        return verticesVec.size() * FLOAT_SIZE;
    }

    size_t getIndSize() {
        return indicesVec.size() * INT_SIZE;
    }

    size_t getIndCount() {
        return indicesVec.size();
    }

    float* getVertDumbArray() {
        return &verticesVec[0]; //cromulent vulnerability 1
    }

    unsigned int* getIndDumbArray() {
        return &indicesVec[0]; //cromulent vulnerability 2
    }

};

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

        if (shape == CROSS) {
            shape = SQUARE;
        }
        else {
            ++shape;
        }
    }
};

template <class dataType>
//TODO: this is bad and horrible and is an afront to god himself.
// update: god is alforgiving it seems
void importArray(const char* path, std::vector<dataType>& array, std::string consoleName) {

    std::ifstream arrayFile(path);

    if (arrayFile) {
        std::cout << consoleName << " array file imported succesfully" << std::endl;
    }
    else {
        std::cout << consoleName << "array file import failed" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << arrayFile.rdbuf();
    std::string arrayString = buffer.str();

    arrayFile.close();

    std::string_view firstThreeBytes = std::string_view(arrayString).substr(0, 3);
    if (firstThreeBytes == BYTE_ORDER_MARK) {
        arrayString.erase(0, 3);
    }

    std::string temporaryString = "";
    for (auto it = arrayString.begin(); it < arrayString.end(); it++) {

        if (*it == '\n') {
            continue;
        }

        else if (*it == ' ') {

            if (temporaryString == "") {
                continue;
            }

            array.push_back(static_cast<dataType>(std::atof(temporaryString.c_str())));
            //std::cout << "string: " << temporaryString << std::endl;
            //std::cout << "vector: " << array[array.size()-1] << std::endl;
            temporaryString = "";
            continue;
        }

        temporaryString.push_back(*it);
    }

}


void genBuffers(VAOobject& object, const char* verticesPath, const char* indiciesPath, std::string consoleName) { 

    std::cout << "importing " << consoleName << ": " << std::endl;

    importArray(verticesPath, object.verticesVec, "mesh");
    std::cout << "mesh size: " << object.getVertSize() << std::endl;

    importArray(indiciesPath, object.indicesVec, "indices");
    std::cout << "indices size: " << object.getIndSize() << std::endl;

    glGenVertexArrays(1, object.vaoPtr);
    glBindVertexArray(*object.vaoPtr);
    glEnableVertexAttribArray(0);

    std::cout << "vao int: " << *object.vaoPtr << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vao created without error" << std::endl;
    }


    glGenBuffers(1, object.vboPtr);
    glBindBuffer(GL_ARRAY_BUFFER, *object.vboPtr);
    glBufferData(GL_ARRAY_BUFFER, object.getVertSize(), object.getVertDumbArray(), GL_STATIC_DRAW);

    std::cout << "vbo int: " << *object.vboPtr << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vbo created without error" << std::endl;
    }


    glGenBuffers(1, object.eboPtr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *object.eboPtr);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, object.getIndSize(), object.getIndDumbArray(), GL_STATIC_DRAW);

    std::cout << "ebo int: " << object.eboPtr << std::endl;
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


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr int windowWidth = 640;
    constexpr int windowHeight = 640;

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


    VAOobject square;
    genBuffers(square, "squareVertices.mesh", "squareIndices.ind", "square");

    VAOobject hourglass;
    genBuffers(hourglass, "hourglassVertices.mesh", "hourglassIndices.ind", "hourglass");

    VAOobject triangle; 
    genBuffers(triangle, "triangleVertices.mesh", "triangleIndices.ind", "triangle");

    VAOobject cross;
    genBuffers(cross, "crossVertices.mesh", "crossIndices.ind", "cross");


    VAOobject vaoObjects[]{ square, hourglass, triangle, cross };//this list should not be hard-coded


    int logLength = 0;
    const int maxlogLength = 90;
    char* log = new char[maxlogLength];
    int itLives;

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //TODO: this may also need to be a function
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


    const float colorSpeed = 0.5f;
    int uniform_colorSpeed = glGetUniformLocation(shaderProgram, "colorSpeed");
    glUniform1f(uniform_colorSpeed, colorSpeed);


    glm::mat4 transformationMatrix = glm::mat4(1.0f);
    int uniform_transformationMatrix = glGetUniformLocation(shaderProgram, "transformationMatrix");
    glUniformMatrix4fv(uniform_transformationMatrix, 1, GL_FALSE, glm::value_ptr(transformationMatrix));


    const float fpsPollRate = 4000.0f;

    const float rotationalSpeed = 1.0f;





    glBindVertexArray((vaoObjects[shape]).vao);

    int lastShape = shape; 

    //glfwSetTime(0.0f);

    auto lastTime = std::chrono::system_clock::now();

    while (!glfwWindowShouldClose(window)) {

        if (lastShape != shape) {//this should be done in poll events
            lastShape = shape;
            glBindVertexArray((vaoObjects[shape]).vao);
        }

        shaderTime = static_cast<float>(glfwGetTime());
        glUniform1f(uniform_shaderTime, shaderTime);


        auto currentTime = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsed.count() >= fpsPollRate) {
            std::cout << "fps is:" << (fpsPollRate / (elapsed.count() / 1000.0f)) << std::endl;
            std::cout << "ms elapsed:" << elapsed.count() << std::endl;
            lastTime = currentTime;
        }

        if (elapsed.count() >= rotationalSpeed) {
            transformationMatrix = glm::rotate(transformationMatrix, glm::radians(1.0f), glm::vec3(0.0, 0.0, 1.0));
            glUniformMatrix4fv(uniform_transformationMatrix, 1, GL_FALSE, glm::value_ptr(transformationMatrix));
        }

        shaderTime = static_cast<float>(glfwGetTime());


        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, vaoObjects[shape].getIndCount(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        processInput(window);
        glfwPollEvents();
        
    }
    

}