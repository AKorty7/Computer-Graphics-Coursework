#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Global variables
static GLFWwindow* gWindow;
static unsigned int gShaderID;

// Function prototypes
void keyboardInput();
void mouseInput();

// Frame timers
float previousTime = 0.0f;
float deltaTime = 0.0f;

Camera camera(glm::vec3(0.0f, 2.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f));

struct Object {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float angle;
    std::string name;
    std::string texture;
};

int main() {
    // Window initialization (same as before)
    if (!glfwInit()) return -1;

    gWindow = glfwCreateWindow(1024, 768, "3D House with Floor", NULL, NULL);
    if (!gWindow) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(gWindow);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) return -1;

    // Enable depth test and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Input setup
    glfwSetInputMode(gWindow, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(gWindow, 1024 / 2, 768 / 2);

    // Shader setup
    gShaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Load models
    Model cube("../assets/cube.obj");

    // Load textures
    GLuint woodTexture = loadTexture("../assets/stone.jpg");
    GLuint brickTexture = loadTexture("../assets/crate.jpg");
    GLuint roofTexture = loadTexture("../assets/crate.jpg");

    // ==================== HOUSE CONSTRUCTION ====================
    std::vector<Object> objects;

    // 1. FLOOR (20x20 wooden platform)
    objects.push_back({
        glm::vec3(0.0f, -0.5f, 0.0f), // position
        glm::vec3(0.0f, 1.0f, 0.0f),   // rotation
        glm::vec3(20.0f, 0.1f, 20.0f), // scale
        0.0f,                          // angle
        "cube",                        // name
        "stone"                         // texture
        });

    // 2. MAIN HOUSE STRUCTURE (brick walls)
    objects.push_back({
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 2.0f, 3.0f),
        0.0f,
        "cube",
        "stone"
        });

    // 3. ROOF (angled planes)
    // Left roof plane
    objects.push_back({
        glm::vec3(0.0f, 3.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(3.2f, 0.2f, 3.2f),
        Maths::radians(45.0f),
        "cube",
        "roof"
        });

    // Right roof plane
    objects.push_back({
        glm::vec3(0.0f, 3.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(3.2f, 0.2f, 3.2f),
        Maths::radians(-45.0f),
        "cube",
        "roof"
        });

    // 4. DOOR
    objects.push_back({
        glm::vec3(0.0f, 0.5f, -1.51f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.8f, 1.5f, 0.1f),
        0.0f,
        "cube",
        "crate"
        });

    // 5. WINDOWS
    objects.push_back({
        glm::vec3(-1.5f, 1.5f, -1.51f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.8f, 0.8f, 0.1f),
        0.0f,
        "cube",
        "stone"
        });

    objects.push_back({
        glm::vec3(1.5f, 1.5f, -1.51f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.8f, 0.8f, 0.1f),
        0.0f,
        "cube",
        "crate"
        });

    // ==================== RENDERING LOOP ====================
    while (!glfwWindowShouldClose(gWindow)) {
        // Timing
        float currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        // Input
        keyboardInput();
        mouseInput();

        // Clear screen
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera setup
        camera.target = camera.eye + camera.front;
        camera.quaternionCamera();
        glUseProgram(gShaderID);

        // Draw all objects
        for (const auto& obj : objects) {
            // Select texture
            if (obj.texture == "stone") glBindTexture(GL_TEXTURE_2D, woodTexture);
            else if (obj.texture == "crate") glBindTexture(GL_TEXTURE_2D, brickTexture);
            else if (obj.texture == "crate") glBindTexture(GL_TEXTURE_2D, roofTexture);

            // Calculate transformations
            glm::mat4 model = Maths::translate(obj.position) *
                Maths::rotate(obj.angle, obj.rotation) *
                Maths::scale(obj.scale);
            glm::mat4 MVP = camera.projection * camera.view * model;

            // Send to shader and draw
            glUniformMatrix4fv(glGetUniformLocation(gShaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            cube.draw(gShaderID);
        }

        glfwSwapBuffers(gWindow);
        glfwPollEvents();
    }

    // Cleanup
    cube.deleteBuffers();
    glDeleteProgram(gShaderID);
    glfwTerminate();
    return 0;
}

// Input functions (same as previous implementation)
void keyboardInput() {
    if (glfwGetKey(gWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(gWindow, true);

    float moveSpeed = 5.0f * deltaTime;
    glm::vec3 newPos = camera.eye;

    if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS) newPos += moveSpeed * camera.front;
    if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS) newPos -= moveSpeed * camera.front;
    if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS) newPos -= moveSpeed * camera.right;
    if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS) newPos += moveSpeed * camera.right;

    // Floor collision (prevent going below y=0.5)
    if (newPos.y < 0.5f) newPos.y = 0.5f;
    camera.eye = newPos;
}

void mouseInput() {
    double xPos, yPos;
    glfwGetCursorPos(gWindow, &xPos, &yPos);
    glfwSetCursorPos(gWindow, 1024 / 2, 768 / 2);

    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    camera.pitch += 0.005f * float(768 / 2 - yPos);
    camera.calculateCameraVectors();
}