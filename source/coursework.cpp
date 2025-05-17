#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Function prototypes
void handleKeyboardInput(GLFWwindow* window);
void handleMouseInput(GLFWwindow* window);
bool initializeWindow(GLFWwindow*& window);
void setupRendering();
void renderScene(GLFWwindow* window, unsigned int shaderID, const Camera& camera, const std::vector<Object>& objects, const Model& cube, const Light& lightSources);
void cleanup(GLFWwindow* window, unsigned int shaderID, const Model& cube);

// Frame timers
float previousTime = 0.0f;  // time of previous iteration of the loop
float deltaTime = 0.0f;  // time elapsed since the previous frame

Camera camera(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f));

struct Object
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    float angle = 0.0f;
    std::string name;
};

int main(void)
{
    // =========================================================================
    // Window creation - you shouldn't need to change this code
    // -------------------------------------------------------------------------
    GLFWwindow* window = nullptr;
    if (!initializeWindow(window))
    {
        return -1;
    }
    // -------------------------------------------------------------------------
    // End of window creation
    // =========================================================================

    // Setup OpenGL rendering settings
    setupRendering();

    // Compile shader program
    unsigned int shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    //lightShaderID = LoadShaders("lightVertexShader.glsl", "lightFragmentShader.glsl");

    // Load models
    Model cube("../assets/cube.obj");
    Model sphere("../assets/sphere.obj");

    // Load the textures
    cube.addTexture("../assets/crate.jpg", "diffuse");

    // Define cube object lighting properties
    cube.ka = 1.0f;
    cube.kd = 0.0f;
    cube.ks = 0.0f;
    cube.Ns = 20.0f;

    // Add light sources
    Light lightSources;
    lightSources.addDirectionalLight(glm::vec3(1.0f, -1.0f, 0.0f),  // direction
        glm::vec3(1.0f, 1.0f, 0.0f));  // colour

    // House-shaped cube positions (2x2x2 base + 2-cube roof)
    glm::vec3 housePositions[] = {
        // Base: 2x2x2 cube (8 cubes)
        glm::vec3(-0.5f, -0.5f, -0.5f),  // Bottom layer
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),  // Top layer
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        // Roof: 2 cubes forming a triangular peak
        glm::vec3(0.0f,  1.5f, -0.5f),  // Roof front
        glm::vec3(0.0f,  1.5f,  0.5f)   // Roof back
    };

    // Add cubes to objects vector
    std::vector<Object> objects;
    Object object;
    object.name = "cube";
    for (unsigned int i = 0; i < 10; i++)
    {
        object.position = housePositions[i];
        object.rotation = glm::vec3(1.0f, 1.0f, 1.0f);
        object.scale = glm::vec3(0.5f, 0.5f, 0.5f);
        object.angle = Maths::radians(20.0f * i);
        objects.push_back(object);
    }

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        renderScene(window, shaderID, camera, objects, cube, lightSources);
    }

    // Cleanup
    cleanup(window, shaderID, cube);
    return 0;
}

void initializeWindowSettings(GLFWwindow* window)
{
    // Ensure we can capture keyboard inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Capture mouse inputs
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);
}

bool initializeWindow(GLFWwindow*& window)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Computer Graphics Coursework", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    initializeWindowSettings(window);
    return true;
}

void setupRendering()
{
    // Enable depth test
    glEnable(GL_DEPTH_TEST);

    // Use back face culling
    glEnable(GL_CULL_FACE);
}

void renderScene(GLFWwindow* window, unsigned int shaderID, const Camera& camera, const std::vector<Object>& objects, const Model& cube, const Light& lightSources)
{
    // Update timer
    float time = glfwGetTime();
    deltaTime = time - previousTime;
    previousTime = time;

    // Get inputs
    handleKeyboardInput(window);
    handleMouseInput(window);

    // Clear the window
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate view and projection matrices
    Camera updatedCamera = camera;
    updatedCamera.target = updatedCamera.eye + updatedCamera.front;
    updatedCamera.quaternionCamera();

    // Activate shader
    glUseProgram(shaderID);

    // Send light source properties to the shader
    lightSources.toShader(shaderID, updatedCamera.view);

    // Send view matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "V"), 1, GL_FALSE, &updatedCamera.view[0][0]);

    // Loop through objects
    for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
    {
        // Calculate model matrix
        glm::mat4 translate = Maths::translate(objects[i].position);
        glm::mat4 scale = Maths::scale(objects[i].scale);
        glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
        glm::mat4 model = translate * rotate * scale;

        // Send the MVP and MV matrices to the vertex shader
        glm::mat4 MV = updatedCamera.view * model;
        glm::mat4 MVP = updatedCamera.projection * MV;
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

        // Draw the model
        if (objects[i].name == "cube")
            cube.draw(shaderID);
    }

    // Draw light sources
    // lightSources.draw(lightShaderID, camera.view, camera.projection, sphere);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void cleanup(GLFWwindow* window, unsigned int shaderID, const Model& cube)
{
    // Cleanup
    cube.deleteBuffers();
    glDeleteProgram(shaderID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void handleKeyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Move the camera using WSAD keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.front;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.eye -= 5.0f * deltaTime * camera.right;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.eye += 5.0f * deltaTime * camera.right;
}

void handleMouseInput(GLFWwindow* window)
{
    // Get mouse cursor position and reset to centre
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Update yaw and pitch angles
    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    camera.pitch += 0.005f * float(768 / 2 - yPos);

    // Calculate camera vectors from the yaw and pitch angles
    camera.calculateCameraVectors();
}