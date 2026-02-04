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

// 1. delete bloat +
//  1.1 delete all meshes +
//  1.2 leave 1 square full screen size +
// 2. virtual screen buffer (just an array) +
// 3. drawPixel() +!!!!!!
// 4. drawHLine(), drawVLine(), drawCircle() +
// 5. importBitmap() (from file i suppose?)
// 6. drawBitmap()
// 7. make this a dependency for pure c

//how do i change graphics independent from the draw-to-screen loop? mayhaps there should be a different thread?


#define INT_SIZE 4
#define FLOAT_SIZE 4
#define BYTE_ORDER_MARK "\xEF\xBB\xBF"
#define SSD1306_BLACK 0
#define SSD1306_WHITE 1
#define SSD1306_INVERSE 2
#define CHANNEL_COUNT 3
#define RGB_WHITE 255
#define RGB_BLACK 0

constexpr const int pixelSize = 10;
constexpr const int displayWidth = 128;
constexpr const int displayHeight = 64;
constexpr const int windowWidth = pixelSize * displayWidth;
constexpr const int windowHeight = pixelSize * displayHeight;


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

class DisplayObject : public Event {
private:
    char* frameBuffer = new char[displayWidth * displayHeight * CHANNEL_COUNT]; 
    char* unupdatedBuffer = new char[displayWidth * displayHeight / sizeof(char)];
    unsigned int texturePtr;

    inline void setBit(char& byte, int n, bool value) {
        if (n < 0 || n > 7) {
            return;
        }
        byte = ((byte & ~((char)1 << n)) | ((char)value << n));
    }

    inline void inverseBit(char& byte, int n) {
        if (n < 0 || n > 7) {
            return;
        }
        byte = (byte ^ ((char)1 << n));
    }

    inline char getBit(char& byte, int n) {
        if (n < 0 || n > 7) {
            return 0;
        }
        return ((byte >> n) & ((char)1));
    }

public:

    DisplayObject() {
        clearDisplay();
        display();
    }

    void display(){

        for (int i = 0; i < displayWidth * displayHeight; i++) { 
            int index = i / 8;
            int bit = i % 8;
            switch ( getBit( unupdatedBuffer[index] , bit ) ) {
            case true:
                for (int channel = 0; channel < 3; channel++) {
                    frameBuffer[i * CHANNEL_COUNT + channel] = RGB_WHITE;
                }
                break;

            case false:
                for (int channel = 0; channel < 3; channel++) {
                    frameBuffer[i * CHANNEL_COUNT + channel] = RGB_BLACK;
                }
                break;
            }
        }

        glGenTextures(1, &texturePtr);
        glBindTexture(GL_TEXTURE_2D, texturePtr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, displayWidth, displayHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, frameBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void event() {
        //this does not need this anymore
    }
    
    void clearDisplay() {
        for (int y = 0; y < displayHeight; y++) {
            for (int x = 0; x < displayWidth; x++) {
                drawPixel(x, y, SSD1306_BLACK);
            }
        }
    }

    void fillDisplay() {
        for (int y = 0; y < displayHeight; y++) {
            for (int x = 0; x < displayWidth; x++) {
                drawPixel(x, y, SSD1306_WHITE);
            }
        }
    }

    void drawPixel(int x, int y, uint16_t color) {
        if ((x >= 0) && (x < displayWidth) && (y >= 0) && (y < displayHeight)) {
            int index = (y * displayWidth + x) / 8;
            int bit = (y * displayWidth + x) % 8;
            switch (color) {

            case SSD1306_WHITE:
                
                setBit(unupdatedBuffer[index], bit, true);
                break;

            case SSD1306_BLACK:
                setBit(unupdatedBuffer[index], bit, false);
                break;

            case SSD1306_INVERSE:
                inverseBit(unupdatedBuffer[index], bit);
                break;
            }
        }
    }

    int width() {
        return displayWidth;
    }

    int height() {
        return displayHeight;
    }
    
    char getPixel(int x, int y) {
        int index = (y * displayWidth + x) / 8;
        int bit = (y * displayWidth + x) % 8;
        return getBit(unupdatedBuffer[index], bit);
    }

    //x is leftmost
    void drawHLine(int x, int y, int w, uint16_t color) {
        for (; x < x + w; x++,w--) {
            drawPixel(x, y, color);
        }
    }
    //y is leftmost
    void drawVLine(int x, int y, int h, uint16_t color) {
        for (; y < y + h; y++, h--) {
            drawPixel(x, y, color);
        }
    }

    //funnily enough adafruits gfx lib when doing drawHline and drawVline just uses drawLine (well, writeLine, but we neednt bother), and leaves optimizing HLine and VLine to the specific
    //hardware implementation
    void drawLine(int x0, int y0, int x1, int y1,uint16_t color) {

        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        int dx = x1 - x0;
        int dy = abs(y1 - y0);

        bool moreThanHalfRight = dy > dx;

        int sign =  1;
        if (y0 > y1) {
            sign = -1;
        }

        int error = dy / 2;

        for (; x0 <= x1; x0++) {
            if (moreThanHalfRight) {
                drawPixel(y0, x0, color);
            }
            else {
                drawPixel(x0, y0, color);
            }

            error -= dy;

            if (error < 0) {
                y0 += sign;
                error += dx;
            }
        }

    }

    void drawCircle(int x, int y, int r, uint16_t color) {

    }

    void drawBitmap(int x, int y, const uint8_t bitmap[], int w, int h, uint16_t color) {
        uint8_t readByte;
        int wholeByteBitmapWidth = (w + 7) / 8;

        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) { //omg i really do just have to learn bit manipulation am'n't i?
                if ( (i % 8) == 0) {
                    readByte = bitmap[(j * wholeByteBitmapWidth) + (i / 8)];
                }
                else {
                    readByte <<= 1;
                }

                if ((readByte & 0b10000000)>0) {
                    drawPixel(x + i, y + j, color);
                }
            }
        }
    }

    void drawBitmap(int x, int y, const uint8_t bitmap[], int w, int h, uint16_t color, uint16_t bg) {
        uint8_t readByte = bitmap[0];
        int wholeByteBitmapWidth = (w + 7) / 8;

        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) { 
                if ((i % 8) == 0) {
                    readByte = bitmap[(j * wholeByteBitmapWidth) + (i / 8)];
                }
                else {
                    readByte <<= 1;
                }

                drawPixel(x + i, y + j, ((readByte & 0b10000000) > 0) ? color : bg); //this fansy is straight outta adafruitGFX
                
                
            }
        }
    }

}; 

//test function
void loadTeto() {
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
}


int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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


    const float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 5, mesh, GL_STATIC_DRAW);

    std::cout << "vbo int: " << meshVBO << std::endl;
    if (glGetError() == GL_NO_ERROR) {
        std::cout << "vbo created without error" << std::endl;
    }

    unsigned int meshEBO;
    glGenBuffers(1, &meshEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * 2, ind, GL_STATIC_DRAW);

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
    glUniform2f(uniform_windowSize, windowWidth, windowHeight);


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

    DisplayObject display;
    eventQueue.push_back(display.getFunc());

    glBindVertexArray(vao);


    display.clearDisplay();
    display.drawPixel(25, 10, SSD1306_WHITE);
    display.drawPixel(5, 10, SSD1306_WHITE);
    display.drawPixel(34, 15, SSD1306_WHITE);
    display.drawPixel(35, 15, SSD1306_INVERSE);
    display.drawHLine(20, 34, 70, SSD1306_WHITE);
    display.drawVLine(25, 20, 20, SSD1306_INVERSE);
    display.drawLine(1, 2, 100, 63, SSD1306_INVERSE);

    const int testBitmapWidth = 15;
    const int testBitmapHeight = 11;
    uint8_t testBitmap[ ( (testBitmapWidth + (sizeof(uint8_t) - 1) ) / sizeof(uint8_t) ) * testBitmapHeight]{
        0b00000000,0b11110000,
        0b00000000,0b00011100,
        0b00000111,0b00000110,
        0b00000111,0b00000110,
        0b00000000,0b00001100,
        0b00000000,0b01111000,
        0b00000000,0b00001100,
        0b00000111,0b00000110,
        0b00000111,0b00000110,
        0b00000000,0b00011100,
        0b00000000,0b11110000
    };

    display.drawBitmap(50, 10, testBitmap, testBitmapWidth, testBitmapHeight, SSD1306_WHITE);

    display.drawBitmap(76, 15, testBitmap, testBitmapWidth, testBitmapHeight, SSD1306_WHITE, SSD1306_BLACK);

    display.drawBitmap(80, 22, testBitmap, testBitmapWidth, testBitmapHeight, SSD1306_WHITE, SSD1306_BLACK);

    display.display();

    //loadTeto();

        
    while (!glfwWindowShouldClose(window)) {


        shaderTime = static_cast<float>(glfwGetTime());
        glUniform1f(uniform_shaderTime, shaderTime);

        


        glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6 , GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);

        processInput(window,eventQueue);
        glfwPollEvents();
        
        
    }
    

}