#ifndef DEMON_H
#define DEMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdlib> // สำหรับ rand()
#include "../projectile/projectile.h"
#include "../wall/stonewall.h"
#include "../aoe/aoe.h"

inline bool checkLineCircleIntersection(glm::vec2 p1, glm::vec2 p2, glm::vec2 center, float radius)
{
    glm::vec2 d = p2 - p1;
    glm::vec2 f = p1 - center;

    float a = glm::dot(d, d);
    float b = 2.0f * glm::dot(f, d);
    float c = glm::dot(f, f) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        return false; // ไม่มีการตัดกัน
    }
    else
    {
        // มีการตัดกัน แต่ต้องเช็คว่าจุดตัดอยู่ภายใน Segment p1-p2 หรือไม่
        discriminant = sqrt(discriminant);
        float t1 = (-b - discriminant) / (2 * a);
        float t2 = (-b + discriminant) / (2 * a);

        // ถ้า t อยู่ระหว่าง 0 ถึง 1 แสดงว่าจุดตัดอยู่บนเส้นที่เราลากพอดี
        if ((t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1))
        {
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------------------
// 🔴 Structs (Crystal)
// --------------------------------------------------------------------------------
struct Crystal
{
    glm::vec3 Position;
    float RotationY;
};

struct CrystalLayer
{
    std::vector<Crystal> crystals;
    int layer;
};

// --------------------------------------------------------------------------------
// 🔴 Enum
// --------------------------------------------------------------------------------
enum class AnimState
{
    IDLE = 1,

    IDLE_ATTACK01,
    ATTACK01_IDLE,

    IDLE_ATTACK02,
    ATTACK02_IDLE,

    IDLE_ATTACK03,
    ATTACK03_IDLE,

    IDLE_WALK,
    WALK_IDLE,
    WALK,
    HURT,
    HURT_IDLE,
    DEAD
};

// --------------------------------------------------------------------------------
// 🔴 Class Declaration (Demon)
// --------------------------------------------------------------------------------
class Demon
{
private:
    Model m_model;
    Animator m_animator;
    Shader m_shader;

    // 🌟 2. Staff & Crystal Models (เพิ่มเข้ามา)
    Model m_staffModel;
    Shader m_staffShader;
    Model m_crystalModel;
    Shader m_crystalShader;
    unsigned int m_crystalDiffuseID;

    // Animations
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_attackAnim01;
    Animation m_attackAnim02;
    Animation m_attackAnim03; // Attack 03 = Cast Spell เดิม
    Animation m_hurtAnim;
    Animation m_deadAnim;

    AnimState m_charState = AnimState::IDLE;
    float m_blendAmount = 0.0f;
    float m_blendRate = 0.1f;
    bool m_isDead = false;
    bool m_isMoving = false;

    // Logic สำหรับ Rotation และ Position
    glm::vec3 m_position = glm::vec3(0.0f, -0.4f, -3.0f);
    glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
    float m_rotationY = 0.0f;

    float m_targetRotationY = 0.0f;
    const float ROTATION_SPEED = 20.0f;

    int m_handBoneID;
    float m_stateTime = -1.0f;
    float m_hurtTimer = 0.0f;

    float m_autoAttackCooldown = 8.0f;
    float m_autoAttackTimer = 0.0f;

    // 🌟 NEW: ตัวแปรสำหรับการเตือนภัย (Warning Phase)
    bool m_isWarningPhase = false;       // กำลังแสดง AoE หรือไม่
    float m_warningTimer = 0.0f;         // นับเวลาถอยหลังช่วงเตือนภัย
    const float WARNING_DURATION = 4.0f; // ระยะเวลาเตือนภัย 4 วินาที

    // 🌟 NEW: เก็บเป้าหมายที่จะโจมตีหลังจากเตือนเสร็จ
    glm::vec3 m_pendingAttackTarget;
    int m_pendingAttackType = 0; // 0, 1, 2 (สุ่มไว้ล่วงหน้า)

    // Crystal Attack Logic
    std::vector<CrystalLayer> m_activeAttack;
    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    int m_currentLayer = 0;
    const float LAYER_SPAWN_RATE = 0.05f;
    const float EFFECT_DURATION = 1.0f;

    bool m_isAttacking_Anim02;
    float m_attackAnim02_Timer;

    std::vector<Projectile> m_activeFireballs;

    Model m_fireballModel;
    Shader m_fireballShader;

    const int FIREBALL_BURST_COUNT = 15;
    const float FIREBALL_SPREAD_ANGLE = 8.0f;

    Model m_wallModel;
    Shader m_wallShader;
    unsigned int m_wallEmissiveID;
    StoneWall m_stoneWallEffect;

    glm::mat4 m_rightHandBoneMatrix; // World Space Matrix ของมือขวา
    glm::vec3 m_forwardDirection;

    AoEIndicator *m_aoeIndicator;

    // 🔴 Collision radius for hitscan detection
    static constexpr float DEMON_COLLISION_RADIUS = 1.5f;

    // 🔴 ฟังก์ชันช่วยสุ่ม/จัดการ Attack
    void updateCrystalAttack(float deltaTime, float currentFrame);
    void TriggerAttack(Animation *nextAnim);

    // --- State Handler Prototypes (ลบ window ออก) ---
    void handleStateIdle();
    void handleStateIdleWalk();
    void handleStateWalk();
    void handleStateWalkIdle();
    void handleStateIdleAttack01();
    void handleStateAttack01Idle();
    void handleStateIdleAttack02();
    void handleStateAttack02Idle();
    void handleStateIdleAttack03();
    void handleStateAttack03Idle();
    void handleStateHurt();
    void handleStateHurtIdle();
    void handleStateDead();
    void ShootFireball();
    void UpdateBoneMatrices(const std::vector<glm::mat4> &finalBoneMatrices);

public:
    Demon(Shader &mainShader, Model &staffModel, Shader &staffShader,
          Model &crystalModel, Shader &crystalShader, unsigned int crystalTexID,
          Model &fbModel, Shader &fbShader,
          Model &wallModel, Shader &wallShader, unsigned int wallEmissiveID)
        : m_model(FileSystem::getPath("src/demon/object/Whiteclown N Hallin.dae")),
          m_idleAnim(FileSystem::getPath("src/demon/object/standing idle.dae"), &m_model),
          m_walkAnim(FileSystem::getPath("src/demon/object/Standing Walk Forward.dae"), &m_model),
          m_attackAnim01(FileSystem::getPath("src/demon/object/Standing 2H Magic Attack 01.dae"), &m_model),
          m_attackAnim02(FileSystem::getPath("src/demon/object/Standing 1H Magic Attack 03.dae"), &m_model),
          m_attackAnim03(FileSystem::getPath("src/demon/object/Standing 2H Cast Spell 01.dae"), &m_model),
          m_hurtAnim(FileSystem::getPath("src/demon/object/Standing React Small From Front.dae"), &m_model),
          m_deadAnim(FileSystem::getPath("src/demon/object/Standing React Death Backward.dae"), &m_model),
          m_animator(&m_idleAnim),
          m_shader(mainShader),
          m_staffModel(staffModel),
          m_staffShader(staffShader),
          m_crystalModel(crystalModel),
          m_crystalShader(crystalShader),
          m_crystalDiffuseID(crystalTexID),
          m_fireballModel(fbModel),
          m_fireballShader(fbShader),
          m_wallModel(wallModel), m_wallShader(wallShader), m_wallEmissiveID(wallEmissiveID),
          m_stoneWallEffect(wallModel, wallShader, 0.1f)
    {
        try
        {
            m_handBoneID = m_model.GetBoneInfoMap().at("mixamorig_RightHand").id;
        }
        catch (const std::out_of_range &oor)
        {
            std::cerr << "Error: Bone 'mixamorig_RightHand' not found in model." << std::endl;
            m_handBoneID = -1;
        }
        m_aoeIndicator = new AoEIndicator("aoe.vs", "aoe.fs");
        m_isAttacking_Anim02 = false;
        m_attackAnim02_Timer = 0.0f;
        m_rightHandBoneMatrix = glm::mat4(1.0f);
        m_forwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    // 🌟 Public Methods
    void Update(float deltaTime, float currentFrame);
    void Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos);

    // 🌟 Input/Action Methods (ไม่ต้องใช้ปุ่ม)
    void Move(float deltaTime, bool isForward);
    void StopMove();
    void Rotate(float deltaTime, float direction);
    void LookAtPosition(const glm::vec3 &targetPos);
    void UpdateRotationTowardTarget(float deltaTime);
    void Attack(); // 🌟 2. สุ่มแอนิเมชัน 3 แบบ
    void Attack(const glm::vec3 &targetPos, int attackType);
    void AttackAnim02(); // 🌟 เพิ่ม Public Method
    void TakeDamage();   // 🌟 1. ถูกโจมตี (เปลี่ยนไป HURT)
    void TriggerDeath(); // 🌟 1. ตายทันที (ไม่ต้องใช้ปุ่ม)

    // Getters
    int GetHandBoneID() const { return m_handBoneID; }
    const std::vector<glm::mat4> &GetFinalBoneMatrices() const { return m_animator.m_FinalBoneMatrices; }
    glm::mat4 GetModelMatrix() const;
    glm::vec3 GetForwardDirection() const { return m_forward; }
    float GetRotationY() const { return m_rotationY; }
    bool IsDead() const { return m_isDead; }
    bool IsCastingAttack() const { return m_isAttacking; }
    bool IsWarningPhase() const { return m_isWarningPhase; }
    const std::vector<CrystalLayer> &GetActiveAttackCrystals() const { return m_activeAttack; }
    float GetCollisionRadius() const { return DEMON_COLLISION_RADIUS; }

    // 🌟 NEW: Get unscaled world position for collision
    glm::vec3 GetWorldPosition() const { return m_position; }
};

// --------------------------------------------------------------------------------
// 🔴 Full Implementation of remaining methods (ต้องนิยามนอกคลาส แต่ยังอยู่ในไฟล์ .h)
// --------------------------------------------------------------------------------

inline glm::mat4 Demon::GetModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.5f, 2.5f, 2.5f));
    return model;
}

// 🌟 Action Methods Implementation
inline void Demon::Move(float deltaTime, bool isForward)
{
    m_isMoving = true;
    if (m_charState == AnimState::IDLE)
    {
        m_charState = AnimState::IDLE_WALK;
    }
    if (m_charState == AnimState::WALK || m_charState == AnimState::IDLE_WALK)
    {
        float speed = 2.0f * (isForward ? 1.0f : -1.0f);
        m_position += m_forward * speed * deltaTime;
    }
}

inline void Demon::StopMove()
{
    m_isMoving = false;
}

inline void Demon::LookAtPosition(const glm::vec3 &targetPos)
{
    glm::vec3 directionVector = targetPos - m_position;
    directionVector.y = 0.0f;

    if (glm::length(directionVector) > 0.01f)
    {
        directionVector = glm::normalize(directionVector);
        float targetAngleRad = glm::atan(directionVector.x, directionVector.z);

        // 🔴 NOTE: ต้องตรวจสอบว่า 0 องศาคือ Z- หรือ Z+
        // เนื่องจาก Demon เริ่มต้นหันไปทาง Z ลบ (-Z), และ atan2(x, z) วัดจาก +Z
        // สูตรนี้ควรให้ค่ามุมที่ถูกต้อง (Yaw)
        m_targetRotationY = glm::degrees(targetAngleRad);
    }
}

inline void Demon::UpdateRotationTowardTarget(float deltaTime)
{
    float currentAngle = m_rotationY;
    float targetAngle = m_targetRotationY;

    // 1. ปรับ Target Angle ให้เป็นมุมที่สั้นที่สุด
    float diff = targetAngle - currentAngle;
    while (diff > 180.0f)
        diff -= 360.0f;
    while (diff < -180.0f)
        diff += 360.0f;

    // 2. คำนวณการหมุนที่ต้องทำ
    float maxRotation = ROTATION_SPEED * deltaTime;

    // 3. หมุนอย่างนุ่มนวล
    if (glm::abs(diff) > maxRotation)
    {
        // ใช้ glm::sign สำหรับทิศทาง +1 หรือ -1
        m_rotationY += glm::sign(diff) * maxRotation;
    }
    else
    {
        m_rotationY = targetAngle;
    }
}

inline void Demon::Rotate(float deltaTime, float direction)
{
    m_rotationY += 100.0f * deltaTime * direction;
}

// 🌟 TriggerDeath (Public Method)
inline void Demon::TriggerDeath()
{
    if (!m_isDead)
    {
        m_isDead = true;
        m_blendAmount = 1.0f;
        m_animator.PlayAnimation(&m_deadAnim, NULL, 0.0f, 0.0f, 0.0f);
        m_animator.UpdateAnimation(0.0f);
        m_charState = AnimState::DEAD;
    }
}

// 🌟 TakeDamage (Public Method)
inline void Demon::TakeDamage()
{
    if (m_isDead || m_charState == AnimState::HURT || m_charState == AnimState::HURT_IDLE)
        return;

    if (m_charState == AnimState::IDLE || m_charState == AnimState::WALK)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(NULL, &m_hurtAnim, 0.0f, 0.0f, 0.0f);
        m_charState = AnimState::HURT;
        m_hurtTimer = 0.0f;
    }
}

// 🌟 Attack (สุ่มแอนิเมชัน 3 แบบ)
inline void Demon::Attack(const glm::vec3 &targetPos, int attackType = -1)
{
    if (m_isDead || m_isAttacking || (m_charState != AnimState::IDLE && m_charState != AnimState::WALK))
        return;

    LookAtPosition(targetPos);

    Animation *nextAnim = nullptr;
    int type = (attackType != -1) ? attackType : (rand() % 3); // ใช้ค่าที่ส่งมา หรือสุ่มใหม่ถ้าไม่มี

    if (type == 0)
        nextAnim = &m_attackAnim01;
    else if (type == 1)
        nextAnim = &m_attackAnim02;
    else
        nextAnim = &m_attackAnim03;

    TriggerAttack(nextAnim);
}

// overload เดิม (สำหรับการกดปุ่ม K ทดสอบ)
inline void Demon::Attack()
{
    // ใช้เป้าหมายปัจจุบัน หรือตำแหน่งข้างหน้า
    glm::vec3 dummyTarget = m_position + m_forward * 5.0f;
    Attack(dummyTarget, -1); // สุ่ม
}

// 🌟 AttackAnim02 (Public Method)
inline void Demon::AttackAnim02()
{
    if (m_isDead || m_isAttacking || (m_charState != AnimState::IDLE && m_charState != AnimState::WALK))
        return;
    TriggerAttack(&m_attackAnim02);
}

// 🌟 TriggerAttack (เปลี่ยนไปใช้ nextAnim)
inline void Demon::TriggerAttack(Animation *nextAnim)
{
    m_isAttacking = true;
    m_attackTimer = 0.0f;
    m_currentLayer = 0;
    m_activeAttack.clear();
    m_stateTime = -1.0f;

    // กำหนด State Machine ที่ถูกต้องตามแอนิเมชัน
    AnimState nextBlendState;
    if (nextAnim == &m_attackAnim01)
    {
        nextBlendState = AnimState::IDLE_ATTACK01;
    }
    else if (nextAnim == &m_attackAnim02)
    {
        nextBlendState = AnimState::IDLE_ATTACK02;
    }
    else
    {
        nextBlendState = AnimState::IDLE_ATTACK03;
    }
    Animation *currentAnim = (m_charState == AnimState::IDLE) ? &m_idleAnim : &m_walkAnim;

    m_blendAmount = 0.0f;
    m_animator.PlayAnimation(currentAnim, nextAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
    m_charState = nextBlendState;
}

inline void Demon::ShootFireball()
{
    // 🌟 1. กำหนด Local Offset (ตำแหน่งสัมพัทธ์กับ Demon World Position)
    // Y: ความสูงที่ Fireball ควรจะออก (จากระดับพื้น -0.4f), Z: ห่างจากตัว Demon
    float spawnHeight = 1.5f;
    float spawnForwardDistance = 1.0f; // พุ่งออกจาก Demon 1.0 หน่วย

    // 🌟 2. คำนวณ World Position: Base Position + Offset Forward + Offset Height
    glm::vec3 startPos = m_position;

    // ชดเชยการเลื่อนตำแหน่งในแนวไปข้างหน้า (ตามทิศทาง m_forward ที่คำนวณจาก m_rotationY)
    startPos += m_forward * spawnForwardDistance;

    // ชดเชยความสูง (Y-axis)
    startPos.y += spawnHeight;

    for (int i = 0; i < FIREBALL_BURST_COUNT; ++i)
    {
        Projectile newFireball(m_fireballModel, m_fireballShader, 0.5f, false);

        // ... (Logic การกระจาย) ...
        float offsetIndex = (float)(i - (FIREBALL_BURST_COUNT - 1) / 2.0f);
        float angleOffset = offsetIndex * FIREBALL_SPREAD_ANGLE;

        glm::vec3 rotatedForward = glm::rotateY(m_forward, glm::radians(angleOffset));

        newFireball.Launch(startPos, rotatedForward);
        m_activeFireballs.push_back(newFireball);
    }
}

inline void Demon::UpdateBoneMatrices(const std::vector<glm::mat4> &finalBoneMatrices)
{
    // ตรวจสอบว่า Bone ID ของมือขวาถูกต้องและอยู่ในขอบเขต
    if (m_handBoneID >= 0 && m_handBoneID < finalBoneMatrices.size())
    {
        glm::mat4 modelMatrix = GetModelMatrix(); // Model Matrix ของ Demon
        glm::mat4 boneMatrix = finalBoneMatrices[m_handBoneID];

        m_rightHandBoneMatrix = modelMatrix * boneMatrix;
    }
}

// 🌟 Demon::Draw (รวม Staff และ Crystal)
inline void Demon::Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos)
{
    // 1. DRAW DEMON
    m_shader.use();
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);

    auto transforms = m_animator.m_FinalBoneMatrices;
    for (int i = 0; i < transforms.size(); ++i)
        m_shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    glm::mat4 modelMatrix = GetModelMatrix();
    m_shader.setMat4("model", modelMatrix);
    m_model.Draw(m_shader);

    UpdateBoneMatrices(transforms);

    // 2. DRAW STAFF (🌟 รวม Staff Logic เข้ามาใน Draw)
    int handBoneID = GetHandBoneID();
    if (handBoneID >= 0 && handBoneID < transforms.size())
    {
        glm::mat4 handBoneTransform = transforms[handBoneID];
        glm::mat4 worldHandMatrix = modelMatrix * handBoneTransform;

        glm::mat4 staffOffset = glm::mat4(1.0f);
        staffOffset = glm::translate(staffOffset, glm::vec3(-95.0f, 178.0f, -190.0f));
        staffOffset = glm::rotate(staffOffset, glm::radians(90.0f), glm::vec3(1, 0, 0));
        staffOffset = glm::scale(staffOffset, glm::vec3(50.0f));

        glm::mat4 staffModelMatrix = worldHandMatrix * staffOffset;

        m_staffShader.use();
        m_staffShader.setMat4("projection", projection);
        m_staffShader.setMat4("view", view);
        m_staffShader.setMat4("model", staffModelMatrix);
        m_staffShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
        m_staffShader.setVec3("viewPos", viewPos);

        m_staffModel.Draw(m_staffShader);
    }

    // 3. DRAW CRYSTAL (🌟 รวม Crystal Logic เข้ามาใน Draw)
    if (IsCastingAttack())
    {
        m_crystalShader.use();
        m_crystalShader.setMat4("projection", projection);
        m_crystalShader.setMat4("view", view);
        m_crystalShader.setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
        m_crystalShader.setVec3("viewPos", viewPos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_crystalDiffuseID);
        m_crystalShader.setInt("texture_diffuse1", 0);

        const auto &activeAttack = GetActiveAttackCrystals();
        for (const auto &layer : activeAttack)
        {
            for (const auto &crystal : layer.crystals)
            {
                glm::mat4 crystalModelMatrix = glm::mat4(1.0f);
                crystalModelMatrix = glm::translate(crystalModelMatrix, crystal.Position);
                crystalModelMatrix = glm::rotate(crystalModelMatrix, glm::radians(crystal.RotationY), glm::vec3(0.0f, 1.0f, 0.0f));

                float baseScale = 4.0f;
                float scale = baseScale + (layer.layer * 0.05f);

                crystalModelMatrix = glm::scale(crystalModelMatrix, glm::vec3(scale));

                m_crystalShader.setMat4("model", crystalModelMatrix);
                m_crystalModel.Draw(m_crystalShader);
            }
        }
    }

    for (Projectile &fireball : m_activeFireballs)
    {
        fireball.Draw(projection, view, viewPos);
    }

    if (m_isWarningPhase && m_aoeIndicator)
    {
        // ใช้ m_position และ m_forward (ซึ่งหันไปหา Target แล้ว)

        if (m_pendingAttackType == 0) // Crystal (Cone) - Attack 01
        {
            // ใช้ค่าเดียวกับตอนร่ายจริง
            m_aoeIndicator->DrawCone(view, projection, m_position, m_forward, 20.0f, 45.0f);
        }
        else if (m_pendingAttackType == 1) // Fireball (Box) - Attack 02
        {
            // ใช้ค่าเดียวกับตอนร่ายจริง (GetRandomTarget ใช้ AREA_WIDTH=8.0, DEPTH=10.0)
            // DrawRectangle วาดจากจุดกึ่งกลางไปข้างหน้า
            // ปรับขนาดให้ตรงกับพื้นที่สุ่มของ Fireball
            m_aoeIndicator->DrawRectangle(view, projection, m_position, m_forward, 12.0f, 8.0f);
        }
        else if (m_pendingAttackType == 2) // StoneWall (Box) - Attack 03
        {
            // 🌟 FIX STONEWALL POSITION: คำนวณให้ตรงกับ StoneWall::Cast()
            // ใน Cast: m_centerPosition = casterPos + forwardDir * WALL_DEFAULT_DISTANCE (6.0f)
            //          + rightDir * sideOffset (1.5f)

            // 1. คำนวณ Right Vector
            glm::vec3 rightDir = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));

            // 2. คำนวณ Center เหมือนใน Cast()
            float dist = 13.0f; // WALL_DEFAULT_DISTANCE
            float sideOffset = 1.5f;
            // glm::vec3 wallCenter = m_position + (m_forward * dist) + (rightDir * sideOffset);
            glm::vec3 wallCenter = m_position + (m_forward * dist);
            wallCenter.y = 0.02f; // ยกขึ้นเหนือพื้นนิดหน่อย

            // 3. คำนวณขนาด
            float wallLength = 1.0f * 25.0f; // Length Multiplier (1) * Scale (25.0f)
            float wallThickness = 1.0f;

            // 4. วาด
            m_aoeIndicator->DrawBoxAtCenter(view, projection,
                                            wallCenter,
                                            rightDir, // หันข้าง (แนวกำแพง)
                                            wallLength,
                                            wallThickness);
        }
    }

    // if (m_charState == AnimState::IDLE_ATTACK01 || m_charState == AnimState::ATTACK01_IDLE)
    // {
    //     // รัศมี 10.0, มุมกระจาย (คำนวณใน initCone ไว้แล้ว)
    //     // แสดงผลจนกว่าจะจบ State
    //     if (m_stateTime < EFFECT_DURATION)
    //     {
    //         m_aoeIndicator->DrawCone(view, projection, m_position, m_forward, 20.0f, 45.0f);
    //     }
    // }

    // // 2. Fireball (Rectangle) - Attack 02
    // if (m_isAttacking_Anim02)
    // {
    //     // พื้นที่สี่เหลี่ยม กว้าง 4 ยาว 6 (ตาม Logic Fireball Random)
    //     // ตำแหน่งเริ่มที่ Demon พุ่งไปข้างหน้า
    //     m_aoeIndicator->DrawRectangle(view, projection, m_position, m_forward, 12.0f, 30.0f);
    // }

    // // 3. Stone Wall (Rectangle) - Attack 03
    // if (m_charState == AnimState::IDLE_ATTACK03 || m_charState == AnimState::ATTACK03_IDLE)
    // {
    //     if (m_stoneWallEffect.IsActive())
    //     {
    //         // ... (Code วาดโมเดลกำแพงเดิม) ...
    //         // ...
    //         m_stoneWallEffect.Draw(projection, view, viewPos);
    //         glDisable(GL_BLEND);

    //         // 🔴🔴 DRAW AOE INDICATOR (พื้นสีแดง) 🔴🔴
    //         if (m_aoeIndicator) // ตรวจสอบว่ามีตัววาดหรือไม่
    //         {
    //             // ดึงข้อมูลจาก Wall
    //             glm::vec3 wallCenter = m_stoneWallEffect.GetCenterPosition();
    //             glm::vec3 wallForward = m_stoneWallEffect.GetForwardDirection();

    //             // คำนวณขนาดพื้นที่สีแดง
    //             // ความยาวกำแพง (ด้านข้าง) = Multiplier * SegmentWidth (1.0f)
    //             float wallLength = (float)m_stoneWallEffect.GetLengthMultiplier() * 25.0f;

    //             // ความหนากำแพง (ด้านลึก) = ให้กว้างกว่าตัวกำแพงจริงหน่อยเพื่อให้เห็นชัด (เช่น 1.5 หน่วย)
    //             float wallThickness = 2.0f;

    //             // ⚠️ สลับด้าน: เนื่องจากกำแพงวางตัวขวาง (แนว X) แต่โมเดล Quad ปกติวางแนว Z
    //             // เราจึงต้องส่งค่าสลับกัน หรือ หมุน 90 องศา
    //             // วิธีง่ายสุด: ใช้ DrawBoxAtCenter แต่สลับค่า Width/Length และหมุนเพิ่ม

    //             // หมุน forward ของ Indicator ไป 90 องศาเพื่อให้ขนานกับแนวกำแพง
    //             glm::vec3 wallRight = glm::normalize(glm::cross(wallForward, glm::vec3(0, 1, 0)));

    //             // วาดสี่เหลี่ยม
    //             // ใช้ wallRight เป็นทิศทางหลัก เพื่อให้สี่เหลี่ยมหันข้างตามกำแพง
    //             // Width (แกน X ของกล่อง) = wallLength
    //             // Length (แกน Z ของกล่อง) = wallThickness
    //             m_aoeIndicator->DrawBoxAtCenter(view, projection,
    //                                             wallCenter,
    //                                             wallRight,      // หันไปทางขวา (แนวกำแพง)
    //                                             wallLength,     // ความยาวกำแพง
    //                                             wallThickness); // ความหนาพื้นที่
    //         }
    //     }
    // }

    if (m_stoneWallEffect.IsActive())
    {
        // 1. ตั้งค่า Emissive Texture ก่อนวาด (ต้องเป็น Shader Unit 0)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_wallEmissiveID);
        m_wallShader.use(); // ใช้ Shader เฉพาะของ Wall
        m_wallShader.setInt("texture_emissive", 0);

        // 2. เปิด Blending สำหรับ Alpha Fade
        glEnable(GL_BLEND);
        // ใช้ Blending แบบปกติสำหรับการจางหาย
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_stoneWallEffect.Draw(projection, view, viewPos);

        glDisable(GL_BLEND);
    }
}
inline void Demon::updateCrystalAttack(float deltaTime, float currentFrame)
{
    // กำหนดข้อมูลบ่อน้ำ (Fountains) - ปรับค่าตาม Bounding Box ที่ให้มา
    const glm::vec2 FOUNTAIN_RIGHT_POS(5.9f, 2.75f);
    const glm::vec2 FOUNTAIN_LEFT_POS(-6.3f, 2.75f);
    const float FOUNTAIN_RADIUS = 1.8f; // เผื่อระยะรัศมีนิดหน่อย

    if (m_stateTime >= 0.0f)
    {
        m_stateTime += deltaTime;

        if (m_currentLayer < 20 && m_stateTime >= m_currentLayer * LAYER_SPAWN_RATE)
        {
            m_currentLayer++;
            CrystalLayer newLayer;
            newLayer.layer = m_currentLayer;

            float zOffset = 0.5f + (m_currentLayer * 0.1f);
            float radius = (m_currentLayer * 0.5f);
            int numCrystals = glm::min(m_currentLayer * 2, 12);

            glm::mat4 demonBaseModel = GetModelMatrix();
            glm::vec3 demonPos = m_position; // ตำแหน่ง Demon (จุดเริ่มของเส้น)

            for (int i = 0; i < numCrystals; ++i)
            {
                Crystal crystal;
                float angle = (float)i / (float)numCrystals * 60.0f - 30.0f;

                glm::vec3 localPos = glm::vec3(
                    radius * glm::sin(glm::radians(angle)),
                    0.0f,
                    zOffset + radius * glm::cos(glm::radians(angle)));

                // คำนวณตำแหน่งโลกของ Crystal
                crystal.Position = glm::vec3(demonBaseModel * glm::vec4(localPos, 1.0f));
                crystal.RotationY = angle + (currentFrame * 100.0f);

                // 🔴🔴 CHECK COLLISION: Demon -> Crystal ตัดผ่านบ่อน้ำหรือไม่ 🔴🔴

                // แปลงเป็น 2D (XZ plane)
                glm::vec2 startPoint(demonPos.x, demonPos.z);
                glm::vec2 endPoint(crystal.Position.x, crystal.Position.z);

                bool hitRight = checkLineCircleIntersection(startPoint, endPoint, FOUNTAIN_RIGHT_POS, FOUNTAIN_RADIUS);
                bool hitLeft = checkLineCircleIntersection(startPoint, endPoint, FOUNTAIN_LEFT_POS, FOUNTAIN_RADIUS);

                // ถ้าไม่ชนบ่อน้ำเลย ให้เพิ่ม Crystal เข้าไป
                if (!hitRight && !hitLeft)
                {
                    newLayer.crystals.push_back(crystal);
                }
            }

            // เพิ่ม Layer แม้ว่าจะไม่มี Crystal ใน Layer นั้น (เพื่อรักษาลำดับเวลา)
            // หรือจะเช็คว่า !newLayer.crystals.empty() ก็ได้ถ้าไม่อยากเก็บ Layer ว่าง
            m_activeAttack.push_back(newLayer);
        }

        if (m_stateTime >= EFFECT_DURATION)
        {
            m_activeAttack.clear();
            m_currentLayer = 0;
            m_stateTime = -1.0f;
        }
    }
}

inline void Demon::Update(float deltaTime, float currentFrame)
{
    if (m_isDead)
    {
        handleStateDead();
        if (m_animator.m_CurrentTime < m_deadAnim.GetDuration() - (m_deadAnim.GetDuration() * 0.1))
        {
            m_animator.UpdateAnimation(deltaTime);
        }
        return;
    }

    // std::cout << m_position.x << " " << m_position.y << " " << m_position.z << std::endl;

    if (!m_isWarningPhase && !m_isAttacking)
    {
        m_autoAttackTimer += deltaTime;

        if (m_autoAttackTimer >= m_autoAttackCooldown)
        {
            // 1. เริ่ม Warning Phase
            m_isWarningPhase = true;
            m_warningTimer = 0.0f;

            // 2. สุ่มประเภทการโจมตีและกำหนดเป้าหมายล่วงหน้า
            m_pendingAttackType = rand() % 3;

            // สมมติเป้าหมาย (ปรับให้ Dynamic ได้ตามต้องการ เช่น ตำแหน่งผู้เล่นล่าสุด)
            // m_pendingAttackTarget = glm::vec3(20.0f, m_position.y, 20.0f);

            // 3. หันหน้าไปหาเป้าหมายทันที (หรือจะค่อยๆ หันก็ได้)
            // LookAtPosition(m_pendingAttackTarget);

            m_autoAttackTimer = 0.0f; // รีเซ็ต Cooldown
        }
    }

    // 🌟🌟 Logic ช่วง Warning Phase 🌟🌟
    if (m_isWarningPhase)
    {
        m_warningTimer += deltaTime;

        // ถ้าครบเวลาเตือนแล้ว -> เริ่มโจมตีจริง
        if (m_warningTimer >= WARNING_DURATION)
        {
            m_isWarningPhase = false;

            // เรียก Attack ตามประเภทที่สุ่มไว้
            Attack(m_pendingAttackTarget, m_pendingAttackType);
        }
    }

    // if (m_charState == AnimState::IDLE)
    // {
    //     m_autoAttackTimer += deltaTime;

    //     if (m_autoAttackTimer >= m_autoAttackCooldown)
    //     {
    //         Attack();

    //         m_autoAttackTimer = 0.0f;
    //     }
    // }

    if (!m_isAttacking && !m_isWarningPhase)
    {
        UpdateRotationTowardTarget(deltaTime);
    }

    m_forward.x = glm::sin(glm::radians(m_rotationY)); // กลับเครื่องหมายของ sin
    m_forward.z = glm::cos(glm::radians(m_rotationY)); // กลับเครื่องหมายของ cos (จาก -cos เป็น +cos)
    m_forward = glm::normalize(m_forward);

    for (size_t i = 0; i < m_activeFireballs.size();)
    {
        m_activeFireballs[i].Update(deltaTime);
        // ต้องสมมติว่า Projectile มีฟังก์ชัน IsDestroyed()
        if (m_activeFireballs[i].IsDestroyed())
        {
            m_activeFireballs.erase(m_activeFireballs.begin() + i);
        }
        else
        {
            ++i;
        }
    }

    // 2. จัดการ Crystal Attack Logic
    updateCrystalAttack(deltaTime, currentFrame);

    m_stoneWallEffect.Update(deltaTime);

    if (m_isAttacking_Anim02 && m_attackAnim02_Timer > 0.0f)
    {
        m_attackAnim02_Timer -= deltaTime;

        // 🌟 Trigger ShootFireball เมื่อ Attack Timer หมด
        if (m_attackAnim02_Timer <= 0.0f)
        {
            ShootFireball();
            // ตั้งค่าเป็น -1.0f เพื่อให้ผ่านเงื่อนไขใน Update แต่ไม่ผ่านเงื่อนไขใน State Handler
            m_attackAnim02_Timer = -1.0f;
        }
    }

    // 3. State Machine update (ใช้ State Handler ที่ไม่มี Input)
    switch (m_charState)
    {
    case AnimState::IDLE:
        handleStateIdle();
        break;
    case AnimState::IDLE_WALK:
        handleStateIdleWalk();
        break;
    case AnimState::WALK:
        handleStateWalk();
        break;
    case AnimState::WALK_IDLE:
        handleStateWalkIdle();
        break;

    case AnimState::IDLE_ATTACK01:
        handleStateIdleAttack01();
        break;
    case AnimState::ATTACK01_IDLE:
        handleStateAttack01Idle();
        break;

    case AnimState::IDLE_ATTACK02:
        handleStateIdleAttack02();
        break;
    case AnimState::ATTACK02_IDLE:
        handleStateAttack02Idle();
        break;
    case AnimState::IDLE_ATTACK03:
        handleStateIdleAttack03();
        break;
    case AnimState::ATTACK03_IDLE:
        handleStateAttack03Idle();
        break;

    case AnimState::HURT:
        handleStateHurt();
        break;
    case AnimState::HURT_IDLE:
        handleStateHurtIdle();
        break;
    case AnimState::DEAD:
        handleStateDead();
        break;
    }

    m_animator.UpdateAnimation(deltaTime);
}

inline void Demon::handleStateIdle()
{
}

inline void Demon::handleStateWalk()
{
    if (!m_isMoving)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_walkAnim, &m_idleAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::WALK_IDLE;
    }
}

inline void Demon::handleStateIdleWalk()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_walkAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_walkAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::WALK;
    }
}

inline void Demon::handleStateWalkIdle()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_walkAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE;
    }
}

inline void Demon::handleStateIdleAttack01()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim01, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_attackAnim01, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::ATTACK01_IDLE;
    }
}

inline void Demon::handleStateAttack01Idle()
{
    if (m_animator.m_CurrentTime >= m_attackAnim01.GetDuration() * 0.5f && m_animator.m_CurrentTime <= m_attackAnim01.GetDuration() * 0.6f && m_stateTime < 0.0f)
    {
        m_stateTime = 0.0f;
    }

    if (m_animator.m_CurrentTime >= m_attackAnim01.GetDuration() * 0.99f)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_attackAnim01, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        m_animator.m_CurrentTime = m_attackAnim01.GetDuration() * 0.99f;
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE;
            m_isAttacking = false;
        }
    }
}

inline void Demon::handleStateIdleAttack02()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim02, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_attackAnim02, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::ATTACK02_IDLE;
    }
}

inline void Demon::handleStateAttack02Idle()
{
    float triggerTime = m_attackAnim02.GetDuration() * 0.35f;
    float duration = m_attackAnim02.GetDuration();

    // 🌟 แก้ไข: ตรวจสอบเงื่อนไขการยิง Fireball (ตั้งค่า m_isAttacking_Anim02 = true)
    if (m_animator.m_CurrentTime >= triggerTime && m_attackAnim02_Timer == 0.0f)
    {
        m_attackAnim02_Timer = 0.01f; // ตั้งค่า Timer เล็กน้อยเพื่อ Trigger ใน Update()
        m_isAttacking_Anim02 = true;  // Flag เพื่อให้ Update() รู้ว่าต้องยิง
    }

    if (m_animator.m_CurrentTime >= m_attackAnim02.GetDuration() * 0.99f)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_attackAnim02, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        m_animator.m_CurrentTime = duration * 0.99f;
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);

            m_charState = AnimState::IDLE;
            m_isAttacking = false;        // ✅ [สำคัญ] รีเซ็ตตัวแปรหลัก
            m_isAttacking_Anim02 = false; // ✅ รีเซ็ต Flag การยิง
            m_attackAnim02_Timer = 0.0f;  // ✅ รีเซ็ต Timer เพื่อให้ Trigger ได้อีกครั้ง
        }
    }
}

inline void Demon::handleStateIdleAttack03()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim03, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_attackAnim03, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::ATTACK03_IDLE;
    }
}

inline void Demon::handleStateAttack03Idle()
{
    if (m_animator.m_CurrentTime >= m_attackAnim03.GetDuration() * 0.5f && m_animator.m_CurrentTime <= m_attackAnim03.GetDuration() * 0.6f && !m_stoneWallEffect.IsActive())
    {
        // 🧱 Logic การร่าย Stone Wall
        int wallCount = 1;         // จำนวน Segment กำแพง
        float wallDuration = 4.0f; // อยู่ได้ 4 วินาที

        // ใช้ m_position และ m_forward เป็นจุดเริ่มต้นและทิศทาง
        m_stoneWallEffect.Cast(m_position, m_forward, wallCount, wallDuration);
    }

    if (m_animator.m_CurrentTime >= m_attackAnim03.GetDuration() * 0.99f)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_attackAnim03, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        m_animator.m_CurrentTime = m_attackAnim03.GetDuration() * 0.99f;
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE;
            m_isAttacking = false;
        }
    }
}

inline void Demon::handleStateHurt()
{
    m_animator.PlayAnimation(&m_hurtAnim, NULL, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
    m_hurtTimer += m_animator.m_DeltaTime;
    if (m_hurtTimer >= m_hurtAnim.GetDuration())
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_hurtAnim, &m_idleAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::HURT_IDLE;
        m_hurtTimer = 0.0f;
    }
}

inline void Demon::handleStateHurtIdle()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_hurtAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE;
    }
}

inline void Demon::handleStateDead()
{
    if (m_animator.m_CurrentTime >= m_deadAnim.GetDuration() * 0.99)
    {
        m_animator.m_CurrentTime = m_deadAnim.GetDuration();
    }
}
#endif // DEMON_H