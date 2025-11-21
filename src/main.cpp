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

    // --- โหลด Model และ Shader สำหรับ Staff/Crystal ---
    Shader staffShader("model_loading_staff.vs", "model_loading_staff.fs");
    Model staffModel(FileSystem::getPath("src/staff/Staff.obj"));
    Shader ourShader("anim_model_demon.vs", "anim_model_demon.fs");

    unsigned int crystalDiffuseID = TextureFromFile(
        FileSystem::getPath("src/crystal/textures/crystal_m_Base_color.png").c_str());

    Shader crystalShader("model_loading_crystal.vs", "model_loading_crystal.fs");
    Model crystalModel(FileSystem::getPath("src/crystal/stylized_crystal_SM.obj"));

    // --------------------------------------------------------------------
    // 🚀 ส่วนที่ 1: โหลดโมเดล Projectile (Fireball/Meteor)
    // --------------------------------------------------------------------
    Shader fireballShader("model_loading_fireball.vs", "model_loading_fireball.fs");
    Model fireballModel(FileSystem::getPath("src/fireball/scene.gltf"));

    Model wallModel(FileSystem::getPath("src/wall/stonewallL.exported.obj"));
    Shader wallShader("model_loading_stonewall.vs", "model_loading_stonewall.fs");
    unsigned int wallEmissiveID = TextureFromFile(
        FileSystem::getPath("src/wall/textures/stonewall_Emissive.png").c_str());

    // --------------------------------------------------------------------
    // 🌟 ส่วนที่ 2: สร้าง Demon Object (ส่ง Projectile Model เข้าไปด้วย)
    // --------------------------------------------------------------------
    Demon demon(ourShader, staffModel, staffShader, crystalModel, crystalShader, crystalDiffuseID,
                fireballModel, fireballShader, // 👈 เพิ่ม Fireball Model/Shader
                wallModel, wallShader, wallEmissiveID);
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, demon);
        demon.Update(deltaTime, currentFrame);

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/Projection Setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --------------------------------------------------------------------
        // ❌ ลบ: ลบ Draw Meteor เก่าออก
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

        // Draw Demon, Staff, Crystal และ Fireball (ทั้งหมดถูกจัดการผ่าน Demon::Draw)
        demon.Draw(projection, view, camera.Position);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --------------------------------------------------------------------------------
// 🔴 ฟังก์ชัน processInput (ส่วนที่แก้ไข)
// --------------------------------------------------------------------------------

void processInput(GLFWwindow *window, Demon &demon)
{
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

    // Demon Movement & Actions

    // การเคลื่อนที่
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        demon.Move(deltaTime, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        demon.Move(deltaTime, false);
    }
    else
    {
        demon.StopMove();
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

    // การโจมตี (Attack) - สุ่ม 3 ท่า
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        demon.Attack();
    }

    // 🌟🌟 ส่วนที่ 3: เพิ่มปุ่ม AttackAnim02 โดยตรง (ใช้ปุ่ม J เป็นตัวอย่าง)
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        demon.AttackAnim02();
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

// --------------------------------------------------------------------------------
// 🔴 ฟังก์ชัน OpenGL/GLFW Callbacks และ TextureFromFile (ไม่มีการแก้ไข)
// --------------------------------------------------------------------------------

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