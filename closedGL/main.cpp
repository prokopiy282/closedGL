#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <functional>


#include "input.h"

//todo: move with 2 modes (mouse and kb, tab toggle) 
//      dvd animation
//      linear interpolation between figures


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
meshState& operator--(meshState& state) {
    int newState = static_cast<int>(state);
    newState--;
    state = static_cast<meshState>(newState);
    return state;
}

meshState shape = SQUARE;




struct ShaderObject {
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
};

//shader type is only for debug purposes, leave empty str if you dont care. r and i values wont stop being a bane of my existence
std::string getShaderSource(const char* path, const std::string shaderType) {

    std::ifstream shaderFile(path);

    if (shaderFile) {
        std::cout << shaderType << " shader file imported succesfully" << std::endl;
    }
    else {
        std::cout << shaderType << " shader file import failed" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    std::string shaderString = buffer.str();

    shaderFile.close();

    std::cout << shaderType << " shader: \n" << shaderString << std::endl;

    return shaderString;
}


void constructShaders(const char* vertexPath, const char* fragmentPath, ShaderObject& object) {

    std::string vertexShaderString = getShaderSource(vertexPath, "vertex");
    const char* vertexShaderSource = vertexShaderString.c_str();

    std::string fragmentShaderString = getShaderSource(fragmentPath, "fragment");
    const char* fragmentShaderSource = fragmentShaderString.c_str();
    

    int logLength = 0;
    const int maxlogLength = 90;
    char* log = new char[maxlogLength];
    int itLives;

    object.vertexShader = glCreateShader(GL_VERTEX_SHADER); //TODO: this may also need to be a function
    glShaderSource(object.vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(object.vertexShader);
    std::cout << "vertex shader status:";
    glGetShaderiv(object.vertexShader, GL_COMPILE_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";

    }
    else {
        glGetShaderInfoLog(object.vertexShader, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }


    object.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(object.fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(object.fragmentShader);
    std::cout << "fragment shader status:";
    glGetShaderiv(object.fragmentShader, GL_COMPILE_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";
    }
    else {
        glGetShaderInfoLog(object.fragmentShader, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }


    object.shaderProgram = glCreateProgram();
    glAttachShader(object.shaderProgram, object.vertexShader);
    glAttachShader(object.shaderProgram, object.fragmentShader);
    glLinkProgram(object.shaderProgram);
    glUseProgram(object.shaderProgram);
    std::cout << "shader Program status:";
    glGetProgramiv(object.shaderProgram, GL_LINK_STATUS, &itLives);
    if (itLives == GL_TRUE)
    {
        std::cout << " is happy :3 \n";

    }
    else {
        glGetProgramInfoLog(object.shaderProgram, maxlogLength, &logLength, log);
        std::cout << "\n" << log << std::endl;
    }

    glDeleteShader(object.vertexShader);
    glDeleteShader(object.fragmentShader);
}


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


void genBuffers(VAOobject& object, const char* verticesPath, const char* indiciesPath, const std::string consoleName) { 

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

    std::cout << "ebo int: " << *object.eboPtr << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "ebo created without error" << std::endl;
    }


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL); //vertex coords
    glEnableVertexAttribArray(0);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (3 * sizeof(float)), (void*)(6 * sizeof(float))); //texture coords
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vertex attrips pointed without error" << std::endl;
    }

}


void processInput(GLFWwindow* window, std::vector < std::function<void(void)> > eventQueue) {
    Input::pollKeyboard(window);
    for (int i = 0; i < eventQueue.size(); i++) {
        eventQueue[i]();
    }
    if (Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }
    if (Input::isKeyPressed(GLFW_KEY_R)) {
        if (shape == CROSS) {
            shape = SQUARE;
        }
        else {
            ++shape;
        }
    }
    if (Input::isKeyPressed(GLFW_KEY_T)) {
        if (shape == SQUARE) {
            shape = CROSS;
        }
        else {
            --shape;
        }
    }
    
};


class Event {
protected:
    std::chrono::system_clock::time_point lastTime;
public:
    virtual void event() {
        std::cout << "error: event not found :(" << std::endl;
    }

    virtual std::function<void(void)> getFunc() {
        //return std::bind(&Event::event, this); 
        return [this]() {this->event(); };
    }

    

};


class DVDAnimation : public Event {
private:
    glm::mat4* sharedTranslationMatrix;
    float xPos = 0.32346f;
    float yPos = 0.5545f; //screen is square in modelspace
    float xDir = 1.0f;
    float yDir = 1.0f;
    static float dvdSpeed;
    float theWall = 1.0f;
    
public:

    DVDAnimation(glm::mat4* sharedTranslationMatrix) {

        lastTime = std::chrono::system_clock::now(); //should be a shared resource
        this->sharedTranslationMatrix = sharedTranslationMatrix;

    }

    void event() {

        if (abs(xPos) >= theWall) { //this number has ALL the magic
            xDir = -xDir;
        }
        if (abs(yPos) >= theWall) {
            yDir = -yDir;
        }
        

        auto currentTime = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        lastTime = currentTime;

        xPos += xDir * dvdSpeed * elapsed.count(); //evil cast
        yPos += yDir * dvdSpeed * elapsed.count();

        *sharedTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, 0.0f));

    }

};
float DVDAnimation::dvdSpeed = 0.001f;


class Rotation : public Event {
private:
    glm::mat4* rotationMatrix;
    static float rotationalSpeed;

public:

    Rotation(glm::mat4* sharedRotationMatrix) {

        lastTime = std::chrono::system_clock::now(); //should be a shared resource
        rotationMatrix = sharedRotationMatrix;

    }

    void event() {

        auto currentTime = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        lastTime = currentTime;

        if (elapsed.count() >= rotationalSpeed) {
            *rotationMatrix = glm::rotate(*rotationMatrix, glm::radians(1.0f), glm::vec3(0.0, 0.0, 1.0));
        }

    }

};
float Rotation::rotationalSpeed = 1.0f;


class sendTransformationMatrix : public Event {
private:

    glm::mat4 transformationMatrix;
    glm::mat4* scalingMatrix;
    glm::mat4* rotationMatrix;
    glm::mat4* translationMatrix;
    static float rotationalSpeed;
    int uniform_transformationMatrix;

public:

    sendTransformationMatrix(glm::mat4* sharedScalingMatrix, glm::mat4* sharedRotationMatrix, glm::mat4* sharedTranslationMatrix, ShaderObject shaders) {
        scalingMatrix = sharedScalingMatrix;
        rotationMatrix = sharedRotationMatrix;
        translationMatrix = sharedTranslationMatrix;
        transformationMatrix = (*translationMatrix) * (*rotationMatrix) * (*scalingMatrix);
        uniform_transformationMatrix = glGetUniformLocation(shaders.shaderProgram, "transformationMatrix");
        glUniformMatrix4fv(uniform_transformationMatrix, 1, GL_FALSE, glm::value_ptr(transformationMatrix));
    }

    void event() {
        transformationMatrix = (*translationMatrix) * (*rotationMatrix) * (*scalingMatrix);
        glUniformMatrix4fv(uniform_transformationMatrix, 1, GL_FALSE, glm::value_ptr(transformationMatrix));
    }

};


class FPSCounter : public Event {
private:
    static float fpsPollRate;
    static bool showFPS;

public:

    void event() {

        auto currentTime = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        lastTime = currentTime;

        if (elapsed.count() >= fpsPollRate && showFPS) {
            std::cout << "fps is:" << (fpsPollRate / (elapsed.count() / 1000.0f)) << std::endl;
            std::cout << "ms elapsed:" << elapsed.count() << std::endl;
            lastTime = currentTime;
        }

    }

}; //broken
float FPSCounter::fpsPollRate = 4000.0f;
bool FPSCounter::showFPS = true;



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



    glViewport(0, 0, windowWidth, windowHeight);


    VAOobject square;
    genBuffers(square, "resources/mesh/squareVertices.mesh", "resources/mesh/squareIndices.ind", "square");

    VAOobject hourglass;
    genBuffers(hourglass, "resources/mesh/hourglassVertices.mesh", "resources/mesh/hourglassIndices.ind", "hourglass");

    VAOobject triangle; 
    genBuffers(triangle, "resources/mesh/triangleVertices.mesh", "resources/mesh/triangleIndices.ind", "triangle");

    VAOobject cross;
    genBuffers(cross, "resources/mesh/crossVertices.mesh", "resources/mesh/crossIndices.ind", "cross");


    VAOobject vaoObjects[]{ square, hourglass, triangle, cross };//this list should not be hard-coded


    ShaderObject shaders;
    constructShaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag", shaders);


    //TODO: shader sets
    const int uniform_windowSize = glGetUniformLocation(shaders.shaderProgram, "windowSize");
    glUniform2f(uniform_windowSize, windowWidth, windowHeight);

    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.0f, 1.0f);
    const int uniform_projectionMatrix = glGetUniformLocation(shaders.shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(uniform_projectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    const int uniform_viewMatrix = glGetUniformLocation(shaders.shaderProgram, "viewMatrix");
    glUniformMatrix4fv(uniform_viewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
    

    float shaderTime = 0.0f; //TODO: add deltatime. also maybe a 32 bit float is a bit wasteful to store ints
    int uniform_shaderTime = glGetUniformLocation(shaders.shaderProgram, "shaderTime");
    glUniform1f(uniform_shaderTime, shaderTime);

    const float gammaCorrection = 0.45f; //2.2 gamma my beloved
    int uniform_gammaCorrection = glGetUniformLocation(shaders.shaderProgram, "gammaCorrection");
    glUniform1f(uniform_gammaCorrection, gammaCorrection);

    const float colorSpeed = 0.5f;
    int uniform_colorSpeed = glGetUniformLocation(shaders.shaderProgram, "colorSpeed");
    glUniform1f(uniform_colorSpeed, colorSpeed);


    //*slaps hood*
    //this bad boy can handle so much events

    std::vector<std::function<void(void)>> eventQueue;

    glm::mat4 transformationMatrix = glm::mat4(1.0f);
    glm::mat4 scalingMatrix = glm::mat4(1.0f);
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    glm::mat4 translationMatrix = glm::mat4(1.0f);

    DVDAnimation DVDAnimationObject(&translationMatrix);
    eventQueue.push_back(DVDAnimationObject.getFunc());

    Rotation rotationObject(&rotationMatrix);
    eventQueue.push_back(rotationObject.getFunc());

    sendTransformationMatrix sendTransformationMatrixObject(&scalingMatrix, &rotationMatrix, &translationMatrix, shaders);
    eventQueue.push_back(sendTransformationMatrixObject.getFunc());

    FPSCounter fpsCounterObject;
    eventQueue.push_back(fpsCounterObject.getFunc());


    glBindVertexArray((vaoObjects[shape]).vao);

    int lastShape = shape; 
     


    //boilest plate
    int texWidth, texHeight, texNrChannels;
    unsigned char* texData = stbi_load("container.jpg", &texWidth, &texHeight, &texNrChannels, 0);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);
    





    while (!glfwWindowShouldClose(window)) {

        if (lastShape != shape) {//this should be done in poll events
            lastShape = shape;
            glBindVertexArray((vaoObjects[shape]).vao);
        }

        shaderTime = static_cast<float>(glfwGetTime());
        glUniform1f(uniform_shaderTime, shaderTime);


        shaderTime = static_cast<float>(glfwGetTime());


        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, vaoObjects[shape].getIndCount(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        processInput(window,eventQueue);
        glfwPollEvents();
        
        
    }
    

}