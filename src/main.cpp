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
void processInput(GLFWwindow *window);
unsigned int TextureFromFile(const char *path);

// --------------------------------------------------------------------------------
// 🔴 Global Variables (Crystal Logic ถูกลบออกไปหมดแล้ว)
// --------------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 🔴 ลบ Crystal attack global variables ออก

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

    Demon demon;

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // 🌟🌟 Logic การสั่ง Attack Demon (ง่ายขึ้นมาก) 🌟🌟
        // Logic การสั่งโจมตีถูกย้ายเข้าไปใน Demon::Update แล้ว แต่ยังคงความสามารถในการสั่งโจมตีจากภายนอก
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !demon.IsCastingAttack())
        {
            demon.Attack();
        }

        // 🔴 ส่ง currentFrame ให้ Demon.Update เพื่อใช้ในการหมุน Crystal
        demon.Update(window, deltaTime, currentFrame);

        // --------------------------------------------------
        // 🔴 Logic การจัดการ Crystal Attack (Generation) ถูกย้ายไปอยู่ใน demon.Update แล้ว
        // --------------------------------------------------

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/Projection Setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        ourShader.use();

        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // Demon Transform Setup
        auto transforms = demon.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.0f));
        model = glm::rotate(model, glm::radians(demon.GetRotationY()), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
        ourShader.setMat4("model", model);

        demon.Draw(ourShader);

        // ----------------------------------------------
        // --- Crystal Render Logic (เรียกใช้ Getter) ---
        // ----------------------------------------------
        crystalShader.use();
        crystalShader.setMat4("projection", projection);
        crystalShader.setMat4("view", view);
        crystalShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
        crystalShader.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, crystalDiffuseID);
        crystalShader.setInt("texture_diffuse1", 0);

        if (demon.IsCastingAttack()) // 🔴 ตรวจสอบสถานะจาก Demon
        {
            const auto &activeAttack = demon.GetActiveAttackCrystals(); // 🔴 ดึง Crystal Data
            for (const auto &layer : activeAttack)
            {
                for (const auto &crystal : layer.crystals)
                {
                    glm::mat4 crystalModelMatrix = glm::mat4(1.0f);
                    crystalModelMatrix = glm::translate(crystalModelMatrix, crystal.Position);
                    crystalModelMatrix = glm::rotate(crystalModelMatrix, glm::radians(crystal.RotationY), glm::vec3(0.0f, 1.0f, 0.0f));

                    float baseScale = 1.5f;
                    float scale = baseScale + (layer.layer * 0.05f);

                    crystalModelMatrix = glm::scale(crystalModelMatrix, glm::vec3(scale));

                    crystalShader.setMat4("model", crystalModelMatrix);
                    crystalModel.Draw(crystalShader);
                }
            }
        }
        int handBoneID = demon.GetHandBoneID();

        glm::mat4 handBoneTransform = transforms[handBoneID];
        glm::mat4 worldHandMatrix = model * handBoneTransform;

        glm::mat4 staffOffset = glm::mat4(1.0f);
        
        staffOffset = glm::translate(staffOffset, glm::vec3(-95.0f, 178.0f, -190.0f)); 
        staffOffset = glm::rotate(staffOffset, glm::radians(90.0f), glm::vec3(1, 0, 0)); 
        staffOffset = glm::scale(staffOffset, glm::vec3(50.0f)); 
        
        glm::mat4 staffModelMatrix = worldHandMatrix * staffOffset;

        staffShader.use();

        staffShader.setMat4("projection", projection);
        staffShader.setMat4("view", view);
        staffShader.setMat4("model", staffModelMatrix);

        staffShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
        staffShader.setVec3("viewPos", camera.Position);

        // 5. วาด Staff
        staffModel.Draw(staffShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ... (ส่วน Implementations ของ Global Functions)
// --------------------------------------------------------------------------------
// 🔴 ฟังก์ชัน OpenGL/GLFW Callbacks และ TextureFromFile (Implementations)
// --------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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