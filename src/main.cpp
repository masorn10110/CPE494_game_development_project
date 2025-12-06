#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>

#include <iostream>
#include <vector>
#include "demon/demon.h"
#include "stb_image.h"
#include "player/player.h"
#include "projectile/projectile.h"
#include "collision/collision.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, Player &player);
unsigned int TextureFromFile(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera - ปรับตำแหน่งให้เหมาะสม
Camera camera(glm::vec3(-0.228665f, 19.0239f, 6.79832f), glm::vec3(0.0f, 1.5f, 0.0f), -2250.39f, -75.7f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
unsigned int playerNoTarget = rand() % 2;
bool onetime = true;

bool cameraRotationEnabled = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Character control
glm::vec3 characterPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 previousPosition = glm::vec3(0.0f, 0.0f, 0.0f); // เก็บตำแหน่งก่อนหน้า
float characterRotation = 0.0f;                           // มุมหมุนของตัวละคร (degrees)
float characterSpeed = 5.0f;
bool isMoving = false;

// Lighting
glm::vec3 lightPos(100.0f, 200.0f, 50.0f); // ตำแหน่งแสง
glm::vec3 lightColor(1.0f, 0.9f, 0.7f);    // สีแสง (ขาว)

// // Bounding Box structure
// struct BoundingBox
// {
//     glm::vec3 min;
//     glm::vec3 max;

//     BoundingBox() : min(glm::vec3(0.0f)), max(glm::vec3(0.0f)) {}
//     BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}

//     // Transform AABB
//     BoundingBox transform(const glm::mat4 &modelMatrix)
//     {
//         glm::vec3 corners[8] = {
//             glm::vec3(min.x, min.y, min.z),
//             glm::vec3(max.x, min.y, min.z),
//             glm::vec3(min.x, max.y, min.z),
//             glm::vec3(max.x, max.y, min.z),
//             glm::vec3(min.x, min.y, max.z),
//             glm::vec3(max.x, min.y, max.z),
//             glm::vec3(min.x, max.y, max.z),
//             glm::vec3(max.x, max.y, max.z)};

//         glm::vec3 newMin = glm::vec3(modelMatrix * glm::vec4(corners[0], 1.0f));
//         glm::vec3 newMax = newMin;

//         for (int i = 1; i < 8; i++)
//         {
//             glm::vec3 transformed = glm::vec3(modelMatrix * glm::vec4(corners[i], 1.0f));
//             newMin = glm::min(newMin, transformed);
//             newMax = glm::max(newMax, transformed);
//         }

//         return BoundingBox(newMin, newMax);
//     }
// };

// // ฟังก์ชันเช็ค collision ระหว่าง 2 AABB
// bool checkAABBCollision(const BoundingBox &a, const BoundingBox &b)
// {
//     return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
//            (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
//            (a.min.z <= b.max.z && a.max.z >= b.min.z);
// }

// สร้าง Bounding Boxes สำหรับกำแพงปราสาท (หลังจาก scale 0.5)
std::vector<BoundingBox> createCastleWallBoundingBoxes()
{
    std::vector<BoundingBox> walls;

    // จากข้อมูลใน box.txt:
    // ตัวละครเคลื่อนที่ได้: X (-10 ถึง +10), Z (-5 ถึง +10)
    // ต้องสร้างกำแพงปิดล้อมพื้นที่นี้

    float wallThickness = 2.0f;
    float minX = -11.0f; // ขอบซ้าย
    float maxX = 11.0f;  // ขอบขวา
    float minZ = -5.5f;  // ขอบหลัง
    float maxZ = 10.5f;  // ขอบหน้า
    float wallHeight = 20.0f;

    // กำแพงทางเหนือ (North wall - แกน +Z)
    walls.push_back(BoundingBox(
        glm::vec3(minX, -1.0f, maxZ - wallThickness),
        glm::vec3(maxX, wallHeight, maxZ)));

    // กำแพงทางใต้ (South wall - แกน -Z)
    walls.push_back(BoundingBox(
        glm::vec3(minX, -1.0f, minZ),
        glm::vec3(maxX, wallHeight, minZ + wallThickness)));

    // กำแพงทางตะวันออก (East wall - แกน +X)
    walls.push_back(BoundingBox(
        glm::vec3(maxX - wallThickness, -1.0f, minZ),
        glm::vec3(maxX, wallHeight, maxZ)));

    // กำแพงทางตะวันตก (West wall - แกน -X)
    walls.push_back(BoundingBox(
        glm::vec3(minX, -1.0f, minZ),
        glm::vec3(minX + wallThickness, wallHeight, maxZ)));

    return walls;
}

BoundingBox characterBBox;
std::vector<BoundingBox> castleWalls;

// Animation states
/*enum AnimState {
    IDLE,
    WALK,
};*/
// AnimState currentState = IDLE;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader castleShader("1.model_loading.vs", "1.model_loading.fs");
    Shader charShader("anim_model.vs", "anim_model.fs");

    // load models
    // -----------
    // idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6
    Model castleModel(FileSystem::getPath("resource/objects/castle/Castle.obj"));
    if (castleModel.meshes.size() == 0)
    {
        std::cout << "Failed to load castle model" << std::endl;
        return -1;
    }

    Model charModel(FileSystem::getPath("resource/maximo/Remy/Remy.dae"));
    if (charModel.meshes.size() == 0)
    {
        std::cout << "Failed to load charmodel" << std::endl;
        return -1;
    }

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

    Shader modelShader("model_loading_1.vs", "model_loading_1.fs");
    Shader animShader("anim_model_1.vs", "anim_model_1.fs");
    Shader hitscanShader("hitscan.vs", "hitscan.fs");
    Player player1(
        FileSystem::getPath("src/player/object/Rifle Aiming Idle.dae"),
        FileSystem::getPath("src/player/object/Rifle Aiming Idle.dae"),
        FileSystem::getPath("src/player/object/Rifle Run.dae"),
        FileSystem::getPath("src/player/object/Jump Up.dae"),
        FileSystem::getPath("src/player/object/Jump Loop.dae"),
        FileSystem::getPath("src/player/object/Jump Down.dae"),
        FileSystem::getPath("src/player/object/Firing Rifle.dae"),
        FileSystem::getPath("src/playergun/object/heavy_rifle.obj"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Idle.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Run Forward.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Jump.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Falling Idle.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Jump Landing.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing 1H Magic Attack 01.dae"),
        FileSystem::getPath("src/player/object/Dying.dae"),
        FileSystem::getPath("src/player/object/Dying_last_frame.dae"),
        glm::vec3(5.0f, 0.0f, 5.0f));
    Player player2(
        FileSystem::getPath("src/player/object/Rifle Aiming Idle.dae"),
        FileSystem::getPath("src/player/object/Rifle Aiming Idle.dae"),
        FileSystem::getPath("src/player/object/Rifle Run.dae"),
        FileSystem::getPath("src/player/object/Jump Up.dae"),
        FileSystem::getPath("src/player/object/Jump Loop.dae"),
        FileSystem::getPath("src/player/object/Jump Down.dae"),
        FileSystem::getPath("src/player/object/Firing Rifle.dae"),
        FileSystem::getPath("src/playergun/object/heavy_rifle.obj"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Idle.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Run Forward.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing Jump.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Falling Idle.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Jump Landing.dae"),
        FileSystem::getPath("src/player/object/grenade_anim/Standing 1H Magic Attack 01.dae"),
        FileSystem::getPath("src/player/object/Dying.dae"),
        FileSystem::getPath("src/player/object/Dying_last_frame.dae"),
        glm::vec3(-5.0f, 0.0f, 5.0f));
    // สร้าง Bounding Boxes สำหรับกำแพงปราสาท
    castleWalls = createCastleWallBoundingBoxes();

    std::cout << "\n=== Castle Walls Bounding Boxes ===" << std::endl;
    const char *wallNames[] = {"North", "South", "East", "West"};
    for (size_t i = 0; i < castleWalls.size(); i++)
    {
        std::cout << wallNames[i] << " Wall: Min("
                  << castleWalls[i].min.x << ", " << castleWalls[i].min.y << ", " << castleWalls[i].min.z
                  << ") Max("
                  << castleWalls[i].max.x << ", " << castleWalls[i].max.y << ", " << castleWalls[i].max.z << ")" << std::endl;
    }

    // Animation walkAnimation(FileSystem::getPath("resource/maximo/Standard Walk/Standard Walk.dae"), &charModel);
    // Animation idleAnimation(FileSystem::getPath("resource/maximo/Idle/Idle.dae"), &charModel);

    // Animator animator(&idleAnimation);

    std::cout << "\n=== Controls ===" << std::endl;
    std::cout << "Arrow Keys: Move character" << std::endl;
    std::cout << "WASD: Move camera" << std::endl;
    std::cout << "Right Mouse: Rotate camera" << std::endl;
    std::cout << "P: Print info" << std::endl;
    std::cout << "C: Toggle collision debug" << std::endl;
    std::cout << "==================\n"
              << std::endl;

    bool movingLight = true;
    bool debugCollision = false;

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // เก็บตำแหน่งก่อนหน้าเพื่อใช้ในการ rollback
        // previousPosition = player1.position;

        // input
        // -----
        processInput(window, player1);
        demon.Update(deltaTime, currentFrame);
        player1.Update(deltaTime,
                       glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS);
        player2.Update(deltaTime,
                       glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS,
                       glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS);

        // ============================================================
        // 🎯 CHECK HITSCAN COLLISIONS - Player1 shoots Player2
        // ============================================================
        glm::vec3 hitPoint;

        if (player1.lastHitscan.CheckHit(player2.position, player2.playerradius, hitPoint))
        {
            player2.health -= 10;
            std::cout << "Player2 hit! Health: " << player2.health << std::endl;
        }

        // ============================================================
        // 🎯 CHECK HITSCAN COLLISIONS - Player2 shoots Player1
        // ============================================================
        if (player2.lastHitscan.CheckHit(player1.position, player1.playerradius, hitPoint))
        {
            player1.health -= 10;
            std::cout << "Player1 hit! Health: " << player1.health << std::endl;
        }

        // ============================================================
        // 🎯 CHECK HITSCAN COLLISIONS - Player1 shoots Demon
        // ============================================================
        if (player1.lastHitscan.CheckHit(demon.GetWorldPosition(), demon.GetCollisionRadius(), hitPoint))
        {
            demon.TakeDamage();
            std::cout << "Demon hit by Player1!" << std::endl;
        }

        // ============================================================
        // 🎯 CHECK HITSCAN COLLISIONS - Player2 shoots Demon
        // ============================================================
        if (player2.lastHitscan.CheckHit(demon.GetWorldPosition(), demon.GetCollisionRadius(), hitPoint))
        {
            demon.TakeDamage();
            std::cout << "Demon hit by Player1!" << std::endl;
        }
        // animator.UpdateAnimation(deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        if (movingLight)
        {
            lightPos.x = sin(currentFrame) * 50.0f;
            lightPos.z = cos(currentFrame) * 50.0f;
        }

        // เปิด wireframe mode เพื่อดูว่ามี geometry หรือไม่
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //===============
        // castle model
        //===============
        // view/projection transformations
        castleShader.use();
        castleShader.setMat4("projection", projection);
        castleShader.setMat4("view", view);

        // ส่งค่า lighting uniforms
        castleShader.setVec3("lightPos", lightPos);
        castleShader.setVec3("viewPos", camera.Position);
        castleShader.setVec3("lightColor", lightColor);

        glm::mat4 castleModel_mat = glm::mat4(1.0f);
        castleModel_mat = glm::translate(castleModel_mat, glm::vec3(0.0f, -1.0f, 0.0f));
        castleModel_mat = glm::scale(castleModel_mat, glm::vec3(0.5f, 0.5f, 0.5f));
        castleShader.setMat4("model", castleModel_mat);
        castleModel.Draw(castleShader);

        //===============
        // character model
        //===============
        charShader.use();
        charShader.setMat4("projection", projection);
        charShader.setMat4("view", view);

        // ส่งค่า lighting uniforms
        charShader.setVec3("lightPos", lightPos);
        charShader.setVec3("viewPos", camera.Position);
        charShader.setVec3("lightColor", lightColor);

        /*auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            charShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);*/

        // คำนวณ model matrix ของตัวละคร
        /*glm::mat4 characterModel_mat = glm::mat4(1.0f);
        characterModel_mat = glm::translate(characterModel_mat, characterPosition);
        characterModel_mat = glm::rotate(characterModel_mat, glm::radians(characterRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        characterModel_mat = glm::scale(characterModel_mat, glm::vec3(0.8f, 0.8f, 0.8f));
        charShader.setMat4("model", characterModel_mat);
        charModel.Draw(charShader);*/

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

        player1.Draw(animShader, modelShader, hitscanShader, view, projection);
        player2.Draw(animShader, modelShader, hitscanShader, view, projection);

        // สร้าง Bounding Box สำหรับตัวละคร (ขนาดประมาณ)
        // float charRadius = 1.0f; // ปรับให้เหมาะสมกับขนาดตัวละคร
        // characterBBox = BoundingBox(
        //     player1.position - glm::vec3(player1.playerradius, 0.0f, player1.playerradius),
        //     player1.position + glm::vec3(player1.playerradius, 3.0f, player1.playerradius));

        CheckWallCollision(
            player1.position,
            player1.previousPosition,
            player1.playerradius,
            castleWalls);
        CheckWallCollision(
            player2.position,
            player2.previousPosition,
            player2.playerradius,
            castleWalls);

        // characterBBox = BoundingBox(
        //     player1.position - glm::vec3(player1.playerradius, 0.0f, player1.playerradius),
        //     player1.position + glm::vec3(player1.playerradius, 3.0f, player1.playerradius));

        // เช็ค collision กับกำแพงทั้งหมด
        // bool collisionDetected = false;
        // for (size_t i = 0; i < castleWalls.size(); i++)
        // {
        //     if (CheckAABBCollision(characterBBox, castleWalls[i]))
        //     {
        //         collisionDetected = true;
        //         if (debugCollision)
        //         {
        //             std::cout << "Collision with wall " << i << std::endl;
        //         }
        //         break;
        //     }
        // }

        // if (collisionDetected)
        // {
        //     // มี collision - ย้อนกลับไปตำแหน่งเดิม
        //     player1.position = previousPosition;
        //

        if (!demon.IsCastingAttack() && !demon.IsWarningPhase())
        {
            if (onetime)
            {
                playerNoTarget = rand() % 2;
                onetime = false;
                printf("Demon is targeting Player %d\n", playerNoTarget ? 1 : 2);
            }
            glm::vec3 targetPosition = playerNoTarget ? player1.position : player2.position;
            demon.LookAtPosition(targetPosition);
        }
        else
        {
            onetime = true;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, Player &player)
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

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (!cameraRotationEnabled)
        {
            cameraRotationEnabled = true;
            firstMouse = true; // รีเซ็ตเมื่อเริ่มหมุน
        }
    }
    else
        cameraRotationEnabled = false;

    // Character movement (Arrow Keys)
    // isMoving = false;
    // bool isRunning = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    // float speed = characterSpeed;

    // glm::vec3 moveDirection = glm::vec3(0.0f);

    // if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    // {
    //     // เคลื่อนที่ไปข้างหน้า (แกน -Z)
    //     moveDirection.z -= 1.0f;
    //     characterRotation = 0.0f;
    //     isMoving = true;
    // }
    // if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    // {
    //     // เคลื่อนที่ถอยหลัง (แกน +Z)
    //     moveDirection.z += 1.0f;
    //     characterRotation = 180.0f;
    //     isMoving = true;
    // }
    // if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    // {
    //     // เคลื่อนที่ไปซ้าย (แกน -X)
    //     moveDirection.x -= 1.0f;
    //     characterRotation = 90.0f;
    //     isMoving = true;
    // }
    // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    // {
    //     // เคลื่อนที่ไปขวา (แกน +X)
    //     moveDirection.x += 1.0f;
    //     characterRotation = -90.0f;
    //     isMoving = true;
    // }

    // // Normalize และเคลื่อนที่
    // if (glm::length(moveDirection) > 0.0f)
    // {
    //     moveDirection = glm::normalize(moveDirection);
    //     characterPosition += moveDirection * speed * deltaTime;
    // }

    // Update animation state
    // static AnimState previousState = IDLE;
    /*if (isMoving)
    {
        if (currentState != WALK)
            currentState = WALK; // Switch to walk animation
    }
    else
    {
        if (currentState != IDLE)
            currentState = IDLE; // Switch to idle animation
    }*/

    static bool pKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pKeyPressed)
    {
        pKeyPressed = true;
        glm::vec3 playerPos = player.position;
        float playerYaw = player.yaw;

        std::cout << "\n=== Info ===" << std::endl;
        std::cout << "Camera: (" << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << ")" << std::endl;
        std::cout << "Player1 Position: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")" << std::endl;
        std::cout << "Character BBox: Min(" << characterBBox.min.x << ", " << characterBBox.min.y << ", " << characterBBox.min.z
                  << ") Max(" << characterBBox.max.x << ", " << characterBBox.max.y << ", " << characterBBox.max.z << ")" << std::endl;
        std::cout << "==================\n"
                  << std::endl;
        std::cout << "\n--- Camera Constructor Code ---" << std::endl;
        std::cout << "Camera camera(glm::vec3("
                  << camera.Position.x << "f, "
                  << camera.Position.y << "f, "
                  << camera.Position.z << "f), glm::vec3(0.0f, 1.5f, 0.0f), "
                  << camera.Yaw << "f, "
                  << camera.Pitch << "f);" << std::endl;
        std::cout << "-------------------------------" << std::endl;

        std::cout << "==================\n"
                  << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        pKeyPressed = false;
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (!cameraRotationEnabled)
        return;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
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