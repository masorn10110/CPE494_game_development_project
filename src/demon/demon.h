#ifndef DEMON_H
#define DEMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdlib> // สำหรับ rand()
#include "projectile/projectile.h"
#include "wall/stonewall.h"

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
    glm::vec3 m_position = glm::vec3(0.0f, -0.4f, 0.0f);
    glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
    float m_rotationY = 0.0f;

    int m_handBoneID;
    float m_stateTime = -1.0f;
    float m_hurtTimer = 0.0f;

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

    const int FIREBALL_BURST_COUNT = 10;
    const float FIREBALL_SPREAD_ANGLE = 8.0f;

    Model m_wallModel;
    Shader m_wallShader;
    unsigned int m_wallEmissiveID;
    StoneWall m_stoneWallEffect;

    glm::mat4 m_rightHandBoneMatrix; // World Space Matrix ของมือขวา
    glm::vec3 m_forwardDirection;

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
          m_stoneWallEffect(wallModel, wallShader, 0.3f)
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
    void Attack();       // 🌟 2. สุ่มแอนิเมชัน 3 แบบ
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
    const std::vector<CrystalLayer> &GetActiveAttackCrystals() const { return m_activeAttack; }
};

// --------------------------------------------------------------------------------
// 🔴 Full Implementation of remaining methods (ต้องนิยามนอกคลาส แต่ยังอยู่ในไฟล์ .h)
// --------------------------------------------------------------------------------

inline glm::mat4 Demon::GetModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
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
inline void Demon::Attack()
{
    if (m_isDead || m_isAttacking || (m_charState != AnimState::IDLE && m_charState != AnimState::WALK))
        return;

    Animation *nextAnim = nullptr;
    int randomType = rand() % 3; // 0, 1, or 2

    if (randomType == 0)
        nextAnim = &m_attackAnim01;
    else if (randomType == 1)
        nextAnim = &m_attackAnim02;
    else if (randomType == 2)
        nextAnim = &m_attackAnim03;

    TriggerAttack(nextAnim);
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
    glm::vec3 startPos = m_rightHandBoneMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    for (int i = 0; i < FIREBALL_BURST_COUNT; ++i)
    {
        // 1. สร้าง Fireball ใหม่ (สมมติว่า Projectile มี Constructor แบบนี้)
        Projectile newFireball(m_fireballModel, m_fireballShader, 0.5f, false);

        // 2. คำนวณทิศทางที่มีการกระจาย (Spread)
        // คำนวณค่า Offset องศา (-1, 0, 1) * SpreadAngle สำหรับ 3 ลูก
        float offsetIndex = (float)(i - (FIREBALL_BURST_COUNT - 1) / 2.0f);
        float angleOffset = offsetIndex * FIREBALL_SPREAD_ANGLE;

        // หมุนเวกเตอร์ m_forward รอบแกน Y (yaw)
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

        // World Matrix ของมือขวา = Demon's Model Matrix * Hand's Bone Matrix
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

                float baseScale = 1.5f;
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
    if (m_stateTime >= 0.0f) // โค้ดที่ถูกต้อง
    {
        m_stateTime += deltaTime;

        if (m_currentLayer < 20 && m_stateTime >= m_currentLayer * LAYER_SPAWN_RATE)
        {
            m_currentLayer++;
            CrystalLayer newLayer;
            newLayer.layer = m_currentLayer;

            // 🔴 ค่าที่ปรับเพื่อควบคุมการกระจายตัว (เหมือนที่ปรับในขั้นตอนก่อนหน้า)
            float zOffset = 0.5f + (m_currentLayer * 0.7f);
            float radius = (m_currentLayer * 0.5f);
            int numCrystals = glm::min(m_currentLayer * 2, 12);

            glm::mat4 demonBaseModel = glm::mat4(1.0f);
            demonBaseModel = glm::translate(demonBaseModel, glm::vec3(0.0f, -0.4f, 0.0f));
            demonBaseModel = glm::rotate(demonBaseModel, glm::radians(GetRotationY()), glm::vec3(0.0f, 1.0f, 0.0f));
            demonBaseModel = glm::scale(demonBaseModel, glm::vec3(.5f, .5f, .5f));

            for (int i = 0; i < numCrystals; ++i)
            {
                Crystal crystal;
                float angle = (float)i / (float)numCrystals * 60.0f - 30.0f;

                glm::vec3 localPos = glm::vec3(
                    radius * glm::sin(glm::radians(angle)),
                    0.0f, // 🔴 คงระดับความสูงไว้ที่ 0.0f (ระดับเท้า Demon)
                    zOffset + radius * glm::cos(glm::radians(angle)));

                crystal.Position = glm::vec3(demonBaseModel * glm::vec4(localPos, 1.0f));
                crystal.RotationY = angle + (currentFrame * 100.0f);

                newLayer.crystals.push_back(crystal);
            }
            m_activeAttack.push_back(newLayer);
        }

        if (m_stateTime >= EFFECT_DURATION) // ใช้ m_stateTime ในการกำหนดระยะเวลาเอฟเฟกต์
        {
            m_activeAttack.clear();
            m_currentLayer = 0;
            m_stateTime = -1.0f; // Reset to Not Triggered
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