#include <iostream>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Function prototypes
void keyboardInput(GLFWwindow* window);
void mouseInput(GLFWwindow* window);

// Frame timers
float previousTime = 0.0f;
float deltaTime = 0.0f;

Camera camera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f));

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
    // Window initialization
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Crate Pillars", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Shaders
    unsigned int shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    glUseProgram(shaderID);

    // Models and textures
    Model cube("../assets/cube.obj");
    cube.addTexture("../assets/crate.jpg", "diffuse");
    cube.ka = 1.0f;
    cube.kd = 0.5f;
    cube.ks = 0.5f;
    cube.Ns = 20.0f;



    // Light setup
    Light lightSources;
    lightSources.addDirectionalLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));


    // Create 20 pillars of stacked crates
    std::vector<Object> objects;
    const int numPillars = 20;
    const int cratesPerPillar = 5;
    const float pillarSpacing = 5.0f;
    const float startX = -((numPillars - 1) * pillarSpacing) / 2.0f;

    for (int pillar = 0; pillar < numPillars; pillar++) {
        for (int level = 0; level < cratesPerPillar; level++) {
            Object crate;
            crate.name = "cube";
            crate.position = glm::vec3(startX + pillar * pillarSpacing, level * 1.0f, 0.0f);
            crate.scale = glm::vec3(1.0f, 1.0f, 1.0f);
            crate.angle = Maths::radians(0.0f);
            crate.rotation = glm::vec3(0.0f, 1.0f, 0.0f);
            objects.push_back(crate);
        }
    }

    // Add a floor
    Object floor;
    floor.name = "cube";
    floor.position = glm::vec3(0.0f, -0.5f, 0.0f);
    floor.scale = glm::vec3(50.0f, 0.1f, 50.0f);
    floor.angle = 0.0f;
    objects.push_back(floor);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        keyboardInput(window);
        mouseInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.target = camera.eye + camera.front;
        camera.quaternionCamera();
        glUseProgram(shaderID);

        lightSources.toShader(shaderID, camera.view);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "V"), 1, GL_FALSE, &camera.view[0][0]);

        for (unsigned int i = 0; i < static_cast<unsigned int>(objects.size()); i++)
        {
            glm::mat4 translate = Maths::translate(objects[i].position);
            glm::mat4 scale = Maths::scale(objects[i].scale);
            glm::mat4 rotate = Maths::rotate(objects[i].angle, objects[i].rotation);
            glm::mat4 model = translate * rotate * scale;

            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            if (objects[i].name == "cube")
                cube.draw(shaderID);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cube.deleteBuffers();
    glDeleteProgram(shaderID);
    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.eye += cameraSpeed * camera.front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.eye -= cameraSpeed * camera.front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.eye -= cameraSpeed * camera.right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.eye += cameraSpeed * camera.right;
}

void mouseInput(GLFWwindow* window)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    camera.yaw += 0.005f * float(xPos - 1024 / 2);
    camera.pitch += 0.005f * float(768 / 2 - yPos);
    camera.calculateCameraVectors();
}