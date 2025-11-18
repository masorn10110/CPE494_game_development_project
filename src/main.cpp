#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

// 🔴 รวมไฟล์ส่วนหัวของ Demon
#include "demon/demon.h"
#include <vector>
#include <iostream>
#include "stb_image.h"

// --------------------------------------------------------------------------------
// 🔴 ประกาศฟังก์ชัน Global
// --------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, Demon &demon);
unsigned int TextureFromFile(const char *path);

// --------------------------------------------------------------------------------
// 🔴 Global Variables
// --------------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --------------------------------------------------------------------------------
// 🔴 Main Function
// --------------------------------------------------------------------------------
int main()
{
    // ... (ส่วนการตั้งค่า GLFW) ...
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    Shader staffShader("model_loading_1.vs", "model_loading_1.fs");
    Model staffModel(FileSystem::getPath("src/staff/Staff.obj"));
    Shader ourShader("anim_model_1.vs", "anim_model_1.fs");

    unsigned int crystalDiffuseID = TextureFromFile(
        FileSystem::getPath("src/crystal/textures/crystal_m_Base_color.png").c_str());

    Shader crystalShader("model_loading_1.vs", "model_loading_1.fs");
    Model crystalModel(FileSystem::getPath("src/crystal/stylized_crystal_SM.obj"));

    // 🌟 1. สร้าง Demon Object
    Demon demon(ourShader, staffModel, staffShader, crystalModel, crystalShader, crystalDiffuseID);

    // --------------------------------------------------------------------
    // 🚀 เพิ่ม: โหลดโมเดล Meteor
    // --------------------------------------------------------------------
    Shader meteorShader("model_loading_1.vs", "model_loading_1.fs");
    Model meteorModel(FileSystem::getPath("src/fireball/scene.gltf"));

    // ตั้งค่า Transform เริ่มต้นสำหรับ Meteor (เพื่อให้มันลอยอยู่ข้างบนและมองเห็นได้ชัด)
    // 🌟🌟 ปรับตำแหน่ง (Y) ให้สูงขึ้น และ Z ให้ไกลขึ้น เพื่อรองรับขนาดที่ใหญ่ขึ้น
    glm::vec3 meteorPosition = glm::vec3(0.0f, 10.0f, -30.0f);

    // 🌟🌟 แก้ไข: ปรับ Scale เป็น 50 เท่า
    float meteorScale = 1.0f;

    float meteorRotation = 0.0f;
    // --------------------------------------------------------------------

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, demon);
        demon.Update(deltaTime, currentFrame);

        // 🚀 เพิ่ม: Animation หมุน Meteor เพื่อให้ดูน่าสนใจ
        meteorRotation += 50.0f * deltaTime;

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/Projection Setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --------------------------------------------------------------------
        // 🚀 เพิ่ม: Draw Meteor
        // --------------------------------------------------------------------
        meteorShader.use();
        meteorShader.setMat4("projection", projection);
        meteorShader.setMat4("view", view);

        glm::mat4 meteorModelMatrix = glm::mat4(1.0f);
        meteorModelMatrix = glm::translate(meteorModelMatrix, meteorPosition);
        meteorModelMatrix = glm::rotate(meteorModelMatrix, glm::radians(meteorRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        meteorModelMatrix = glm::scale(meteorModelMatrix, glm::vec3(meteorScale));

        meteorShader.setMat4("model", meteorModelMatrix);
        meteorShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f)); // สมมติ LightPos
        meteorShader.setVec3("viewPos", camera.Position);

        meteorModel.Draw(meteorShader);
        // --------------------------------------------------------------------

        // Draw DEMON (Animated Model)
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("model", demon.GetModelMatrix()); // ใช้ Getter ของ Demon

        // Demon Bone Transform Setup
        auto transforms = demon.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // Draw Demon, Staff และ Crystal (ผ่าน Demon::Draw)
        demon.Draw(projection, view, camera.Position);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --------------------------------------------------------------------------------
// 🔴 ฟังก์ชัน OpenGL/GLFW Callbacks และ TextureFromFile (Implementations)
// --------------------------------------------------------------------------------

void processInput(GLFWwindow *window, Demon &demon)
{
    // ... (ฟังก์ชัน processInput เดิม) ...
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Demon Movement & Actions (ใช้ Public Methods ของ Demon)

    // การเคลื่อนที่
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        demon.Move(deltaTime, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        demon.Move(deltaTime, false);
    }

    // การหมุน
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        demon.Rotate(deltaTime, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        demon.Rotate(deltaTime, -1.0f);
    }

    // การโจมตี (Attack)
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        demon.Attack();
    }

    // การบาดเจ็บ/ตาย (สำหรับทดสอบ)
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        demon.TakeDamage();
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        demon.TriggerDeath();
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int TextureFromFile(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}