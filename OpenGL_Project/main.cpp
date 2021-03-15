#define GLEW_STATIC

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.hpp"

#include <iostream>

// window
gps::Window myWindow;
GLFWwindow* window = NULL;
int retina_width, retina_height;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];
int sceneMode = 0;

//fog
int putFog = 0;


// models
gps::Model3D city;
gps::Model3D dissapearingCombatJet;
gps::Model3D freighter;
gps::Model3D transportShuttle;
gps::Model3D grass;
gps::Model3D ufo;
gps::Model3D alien;

GLfloat angle;
GLfloat angleTransport;

GLfloat freighterXModifier = -3.0f;
GLfloat alientYModifier = 0.9f;

bool doRenderJet = false;
bool show = false;

// skybox
gps::SkyBox skyBox;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    myBasicShader.useShaderProgram();
    //set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    //send matrix data to shader
    GLint projLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    //set Viewport transform
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

double pitch = 0.0f;
double yaw = -90.0f;
double lastX = 512, lastY = 384;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.5f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix 
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();

        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();

        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();

        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();

        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // compute normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    // rotate the light source
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix 
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    // rotate the light source
    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix 
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix 
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    // move the alien to the ground
    if (pressedKeys[GLFW_KEY_Z]) {
        alientYModifier -= 0.07f;
        if (alientYModifier < -1.0f)
            alientYModifier = 0.9f;
    }

    // move the alien back into the ship
    if (pressedKeys[GLFW_KEY_X]) {
        alientYModifier += 0.07f;
        if (alientYModifier > 0.9f)
            alientYModifier = -1.0f;
    }

    // move the freighter
    if (pressedKeys[GLFW_KEY_U]) {
        freighterXModifier -= 0.1f;
        if (freighterXModifier < -3.9f)
            freighterXModifier = 3.9f;
    }

    // move the freighter
    if (pressedKeys[GLFW_KEY_I]) {
        freighterXModifier += 0.1f;
        if (freighterXModifier > 3.9f)
            freighterXModifier = -3.9f;
    }

    // make the jet appear / dissapear
    if (pressedKeys[GLFW_KEY_M]) {
        doRenderJet = !doRenderJet;
        pressedKeys[GLFW_KEY_M] = false;
    }

    // visualize the 3 modes
    if (pressedKeys[GLFW_KEY_G]) {
        sceneMode += 1;
        switch (sceneMode) {
        case 0:
            // solid mode;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case 1:
            // wireframe objects;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 2:
            // polygonal and smooth;
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case 3:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_POLYGON_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            break;
        }
        if (sceneMode == 3) {
            sceneMode = -1;
        }
        pressedKeys[GLFW_KEY_G] = false;
    }

    // visualize the entire scene using animation (enter show mode)
    if (pressedKeys[GLFW_KEY_C]) {
        if (show)
            show = false;
        else
            show = true;
        pressedKeys[GLFW_KEY_C] = false;
    }

    //enable fog option
    if (pressedKeys[GLFW_KEY_F]) {
        putFog = 1;
        myBasicShader.useShaderProgram();       
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "putFog"), putFog);
        
    }
    
    //disable fog option
    if (pressedKeys[GLFW_KEY_V]) {
        putFog = 0;
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "putFog"), putFog);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    city.LoadModel("models/city/Nimbasa.obj");
    alien.LoadModel("models/alien/elite_static.obj");
    ufo.LoadModel("models/ufo/ufo.obj");
    grass.LoadModel("models/grass/grass.obj");
    dissapearingCombatJet.LoadModel("models/combat_jet/Futuristic_combat_jet.obj");
    freighter.LoadModel("models/freigther/Freigther_BI_Export.obj");
    transportShuttle.LoadModel("models/transport_shuttle/TransportShuttle_obj.obj");
    std::vector<const GLchar*> faces;
    faces.push_back("textures/skybox/right.tga");
    faces.push_back("textures/skybox/left.tga");
    faces.push_back("textures/skybox/top.tga");
    faces.push_back("textures/skybox/bottom.tga");
    faces.push_back("textures/skybox/back.tga");
    faces.push_back("textures/skybox/front.tga");
   skyBox.Load(faces);
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


    //SKYBOX SHADER
    skyboxShader.useShaderProgram();

    modelLoc = glGetUniformLocation(skyboxShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(skyboxShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(skyboxShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
}


void renderCity(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1 / 10000.0f, 1 / 10000.0f, 1 / 10000.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    city.Draw(shader);
}

void renderTransportShuttle(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angleTransport), glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -5.0f));
      model = glm::scale(model, glm::vec3(1 / 30.0f, 1 / 30.0f, 1 / 30.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    transportShuttle.Draw(shader);
}


void renderFreighter(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::translate(glm::mat4(1.0f), glm::vec3(freighterXModifier, 0.0f, -2.0f));
    model = glm::scale(model, glm::vec3(1 / 20.0f, 1 / 20.0f, 1 / 20.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    freighter.Draw(shader);
}


void renderJet(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix 
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.6f, -0.7f, -1.8f));
    model = glm::scale(model, glm::vec3(1 / 25.0f, 1 / 25.0f, 1 / 25.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    dissapearingCombatJet.Draw(shader);
}

void renderSkyBox(gps::Shader shader) {
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    skyBox.Draw(shader,view, projection);
}


void renderGrass(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1 / 20.0f, 1 / 20.0f, 1 / 20.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    grass.Draw(shader);
}

void renderUFO(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.7f, 1.4f, 0.0f));
    model = glm::scale(model, glm::vec3(1 / 230.0f, 1 / 230.0f, 1 / 230.0f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    ufo.Draw(shader);
}

void renderAlien(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix
    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.7f, alientYModifier, 0.0f));
    model = glm::scale(model, glm::vec3(1 / 330.0f, 1 / 330.0f, 1 / 330.0f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    alien.Draw(shader);
}

int times = 0;
float up = 0;

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene
    // initialize the view matrix by taking the current state of the camera
    view = myCamera.getViewMatrix();
    // if the presentation mode is requested
    if (show) {
        // we check if we reached a desired position with the camera
        if (up < -1.0f) {
            // the view will rotate on the Y axis -> circling the scene 
            view = glm::translate(view, glm::vec3(0, -1.0f, 0));
            view = glm::rotate(view, glm::radians((float)times++), glm::vec3(0, 1, 0));
            // after spinning a couple of times -> the presentation mode is over and the values are reinitialized
            if (times == 360) {
                show = false;
                times = 0;
                up = 0;
            }
        }
        else {
            // if not, we keep translating the camera on the Y axis -> move above the ground until it reaches a point where it stops
            up = up - 0.05f;
            view = glm::translate(view, glm::vec3(0, up, 0));
        }
    }

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // render all objects
    renderGrass(myBasicShader);
    renderSkyBox(skyboxShader);
    renderTransportShuttle(myBasicShader);
    renderCity(myBasicShader);
    renderFreighter(myBasicShader);
    if (doRenderJet) {
        renderJet(myBasicShader);
    }
    renderUFO(myBasicShader);
    renderAlien(myBasicShader);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    glCheckError();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();
        angleTransport+=0.2f;
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
