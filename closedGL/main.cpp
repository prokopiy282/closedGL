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


// 1. delete bloat
//  1.1 delete all meshes
//  1.2 leave 1 square full screen size
// 2. virtual screen buffer (just an array)
// 3. drawPixel()
// 4. drawHLine(), drawVLine(), drawCircle()
// 5. importBitmap()
// 6. drawBitmap()
// 7. make this a dependency for pure c


#define INT_SIZE 4
#define FLOAT_SIZE 4
#define BYTE_ORDER_MARK "\xEF\xBB\xBF"
#define PIXEL_SIZE 5
#define WINDOW_WIDTH PIXEL_SIZE*128
#define WINDOW_HEIGHT PIXEL_SIZE*64


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


void processInput(GLFWwindow* window, std::vector < std::function<void(void)> > eventQueue) {
    Input::pollKeyboard(window);
    for (int i = 0; i < eventQueue.size(); i++) {
        eventQueue[i]();
    }
    if (Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
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



int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "black magic", NULL, NULL);
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



    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


    const float aspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
   
    const float mesh[] = {
        -aspectRatio, -1.0f, 0.0f,  0.0f, 0.0f, //bottom left
        -aspectRatio,  1.0f, 0.0f,  0.0f, 1.0f, //top left
         aspectRatio,  1.0f, 0.0f,  1.0f, 1.0f, //top right
         aspectRatio, -1.0f, 0.0f,  1.0f, 0.0f, //bottom right
    };

    const int ind[]{
        0, 1, 2,
        2, 3, 0
    };

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);

    std::cout << "vao int: " << vao << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vao created without error" << std::endl;
    }

    unsigned int meshVBO;
    glGenBuffers(1, &meshVBO);
    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*5, mesh, GL_STATIC_DRAW);

    std::cout << "vbo int: " << meshVBO << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vbo created without error" << std::endl;
    }

    unsigned int meshEBO;
    glGenBuffers(1, &meshEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*3*2, ind, GL_STATIC_DRAW);

    std::cout << "ebo int: " << meshEBO << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "ebo created without error" << std::endl;
    }


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (5 * sizeof(float)), NULL); //vertex coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (5 * sizeof(float)), (void*)(3 * sizeof(float))); //texture coords
    glEnableVertexAttribArray(1);
    //glBindVertexArray(0);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vertex attrips pointed without error" << std::endl;
    }



    ShaderObject shaders;
    constructShaders("resources/shaders/vertex.vert", "resources/shaders/fragment.frag", shaders);


    //TODO: shader sets
    const int uniform_windowSize = glGetUniformLocation(shaders.shaderProgram, "windowSize");
    glUniform2f(uniform_windowSize, WINDOW_WIDTH, WINDOW_HEIGHT);


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

    sendTransformationMatrix sendTransformationMatrixObject(&scalingMatrix, &rotationMatrix, &translationMatrix, shaders);
    eventQueue.push_back(sendTransformationMatrixObject.getFunc());

    glBindVertexArray(vao);
     

    //boilest plate
    int texWidth, texHeight, texNrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* texData = stbi_load("resources/sprites/evilTeto.PNG", &texWidth, &texHeight, &texNrChannels, 0);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texData);


        
    while (!glfwWindowShouldClose(window)) {


        shaderTime = static_cast<float>(glfwGetTime());
        glUniform1f(uniform_shaderTime, shaderTime);

        


        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6 , GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        processInput(window,eventQueue);
        glfwPollEvents();
        
        
    }
    

}