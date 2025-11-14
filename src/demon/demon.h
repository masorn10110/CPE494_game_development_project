#ifndef DEMON_H
#define DEMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL // 🔴 ต้องรวม
#include <glm/gtx/rotate_vector.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

// 🔴 โครงสร้าง Crystal Attack ถูกย้ายเข้ามาใน Header
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

// --------------------------------------------------------------------------------
// 🔴 Enum
// --------------------------------------------------------------------------------
enum class AnimState
{
    IDLE = 1,
    IDLE_CAST,
    CAST_IDLE,
    IDLE_ATTACK01,
    ATTACK01_IDLE,

    IDLE_WALK,
    WALK_IDLE,
    WALK,
    HURT,
    HURT_IDLE,
    DEAD
};

// --------------------------------------------------------------------------------
// 🔴 Class Declaration (รวม Implementation บางส่วน)
// --------------------------------------------------------------------------------
class Demon
{
private:
    Model m_model;
    Animator m_animator;

    // Animations
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_castAnim;
    Animation m_attackAnim01;
    Animation m_hurtAnim;
    Animation m_deadAnim;

    AnimState m_charState = AnimState::IDLE;
    float m_blendAmount = 0.0f;
    float m_blendRate = 0.1f;
    bool m_isDead = false;

    // Logic สำหรับ Rotation
    glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
    float m_rotationY = 0.0f;

    // Transition times (Calculated based on animation duration)
    // const float CAST_TRANSITION_TIME;
    // const float ATTACK01_TRANSITION_TIME;
    // const float IDLE_TIME;
    // const float WALK_TIME;
    // const float HURT_ANIM_TIME = 0.5f;

    int m_handBoneID;
    float m_stateTime = -1.0f;
    float m_hurtTimer = 0.0f;

    // 🌟🌟 Crystal Attack Logic (Member Variables) 🌟🌟
    std::vector<CrystalLayer> m_activeAttack;
    bool m_isAttacking = false; // 🔴 แทนที่ extern bool isAttacking;
    float m_attackTimer = 0.0f;
    int m_currentLayer = 0;

    // 🔴 ค่าคงที่สำหรับ Crystal Logic (ปรับที่นี่ได้เลย)
    const float LAYER_SPAWN_RATE = 0.05f;
    const float EFFECT_DURATION = 1.0f;

    // 🔴 ฟังก์ชันจัดการ Crystal
    void updateCrystalAttack(float deltaTime, float currentFrame);
    // 🌟🌟 สิ้นสุด Crystal Attack Logic 🌟🌟

    // --- State Handler Prototypes ---
    void handleStateIdle(GLFWwindow *window);
    void handleStateIdleWalk();
    void handleStateWalk(GLFWwindow *window);
    void handleStateWalkIdle();
    void handleStateIdleCast();
    void handleStateCastIdle();

    void handleStateIdleAttack01();
    void handleStateAttack01Idle();

    void handleStateHurt();
    void handleStateHurtIdle();
    void handleStateDead();

    // --- Interrupt Logic ---
    bool checkHurtInterrupt(GLFWwindow *window);

public:
    // 1. Constructor (Implementation อยู่ใน .h)
    Demon() : m_model(FileSystem::getPath("src/demon/object/Whiteclown N Hallin.dae")),
              m_idleAnim(FileSystem::getPath("src/demon/object/standing idle.dae"), &m_model),
              m_walkAnim(FileSystem::getPath("src/demon/object/Standing Walk Forward.dae"), &m_model),
              m_castAnim(FileSystem::getPath("src/demon/object/Standing 2H Cast Spell 01.dae"), &m_model),
              m_attackAnim01(FileSystem::getPath("src/demon/object/Standing 2H Magic Attack 01.dae"), &m_model),
              m_hurtAnim(FileSystem::getPath("src/demon/object/Standing React Small From Front.dae"), &m_model),
              m_deadAnim(FileSystem::getPath("src/demon/object/Standing React Death Backward.dae"), &m_model),
              m_animator(&m_idleAnim)
    //   CAST_TRANSITION_TIME(m_castAnim.GetDuration() * 0.2f),
    //   ATTACK01_TRANSITION_TIME(m_attackAnim01.GetDuration() * 0.3f),
    //   WALK_TIME(m_walkAnim.GetDuration() * 0.13f),
    //   IDLE_TIME(m_idleAnim.GetDuration() * 0.22f)
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
    }

    // 2. Public Methods
    void Update(GLFWwindow *window, float deltaTime, float currentFrame); // 🔴 รับ currentFrame มาด้วย
    void Draw(Shader &shader) { m_model.Draw(shader); }
    int GetHandBoneID() const { return m_handBoneID; }
    const std::vector<glm::mat4> &GetFinalBoneMatrices() const { return m_animator.m_FinalBoneMatrices; }

    glm::vec3 GetForwardDirection() const { return m_forward; }
    float GetRotationY() const { return m_rotationY; }

    void TriggerDeath()
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
    bool IsDead() const { return m_isDead; }

    // 🌟🌟 Demon::Attack() (Implementation อยู่ใน .h)
    void Attack()
    {
        if (m_isDead)
            return;

        // 🔴 เริ่มต้น Attack Logic
        m_isAttacking = true;
        m_attackTimer = 0.0f;
        m_currentLayer = 0;
        m_activeAttack.clear();
        m_stateTime = -1.0f;

        if (m_charState == AnimState::IDLE)
        {
            m_blendAmount = 0.0f;
            m_animator.PlayAnimation(&m_idleAnim, &m_attackAnim01, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE_ATTACK01;
        }
        else if (m_charState == AnimState::WALK)
        {
            m_blendAmount = 0.0f;
            m_animator.PlayAnimation(&m_walkAnim, &m_attackAnim01, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE_ATTACK01;
        }
    }

    // 🌟🌟 Getter สำหรับ Crystal Logic (ใช้ใน main.cpp สำหรับ Render)
    bool IsCastingAttack() const { return m_isAttacking; }
    const std::vector<CrystalLayer> &GetActiveAttackCrystals() const { return m_activeAttack; }
    // 🌟🌟
};

// --------------------------------------------------------------------------------
// 🔴 Full Implementation of remaining methods (ต้องนิยามนอกคลาส แต่ยังอยู่ในไฟล์ .h)
// --------------------------------------------------------------------------------

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

inline bool Demon::checkHurtInterrupt(GLFWwindow *window)
{
    // ... (โค้ดเดิม)
    if (m_isDead)
        return false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        TriggerDeath();
        return true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        if (m_charState == AnimState::IDLE || m_charState == AnimState::WALK)
        {
            m_blendAmount = 0.0f;
            Animation *currentAnim = m_animator.m_CurrentAnimation;

            m_animator.PlayAnimation(NULL, &m_hurtAnim, 0.0f, 0.0f, 0.0f);
            m_charState = AnimState::HURT;
            m_hurtTimer = 0.0f;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

inline void Demon::Update(GLFWwindow *window, float deltaTime, float currentFrame)
{
    if (m_isDead)
    {
        handleStateDead(); // อาจจะทำให้ m_CurrentTime = Duration
        if (m_animator.m_CurrentTime < m_deadAnim.GetDuration() - (m_deadAnim.GetDuration() * 0.1))
        {
            m_animator.UpdateAnimation(deltaTime);
        }

        return;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_rotationY += 100.0f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_rotationY -= 100.0f * deltaTime;
    }

    m_forward.x = glm::sin(glm::radians(m_rotationY));
    m_forward.z = -glm::cos(glm::radians(m_rotationY));
    m_forward = glm::normalize(m_forward);

    // 1. ตรวจสอบการขัดจังหวะ (Interrupt)
    checkHurtInterrupt(window);

    // 🌟🌟 เรียก Attack() หากกด Spacebar และไม่กำลังโจมตีอยู่
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && !m_isAttacking) // 🔴 ใช้ m_isAttacking
    {
        if (m_charState == AnimState::IDLE || m_charState == AnimState::WALK)
        {
            Attack();
        }
    }

    // 2. จัดการ Crystal Attack Logic
    updateCrystalAttack(deltaTime, currentFrame); // 🔴 เรียกใช้ฟังก์ชันที่ถูกย้ายเข้ามา

    // 3. State Machine update (โค้ดเดิม)
    switch (m_charState)
    {
    case AnimState::IDLE:
        handleStateIdle(window);
        break;
    case AnimState::IDLE_WALK:
        handleStateIdleWalk();
        break;
    case AnimState::WALK:
        handleStateWalk(window);
        break;
    case AnimState::WALK_IDLE:
        handleStateWalkIdle();
        break;
    case AnimState::IDLE_CAST:
        handleStateIdleCast();
        break;
    case AnimState::CAST_IDLE:
        handleStateCastIdle();
        break;
    case AnimState::IDLE_ATTACK01:
        handleStateIdleAttack01();
        break;
    case AnimState::ATTACK01_IDLE:
        handleStateAttack01Idle();
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

    // 4. Update animator
    m_animator.UpdateAnimation(deltaTime);
}

inline void Demon::handleStateIdle(GLFWwindow *window)
{

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_idleAnim, &m_walkAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE_WALK;
    }
    else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        m_blendAmount = 0.0f;
        m_animator.PlayAnimation(&m_idleAnim, &m_castAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
        m_charState = AnimState::IDLE_CAST;
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

inline void Demon::handleStateWalk(GLFWwindow *window)
{
    // m_animator.PlayAnimation(&m_walkAnim, NULL, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (glfwGetKey(window, GLFW_KEY_UP) != GLFW_PRESS)
    {
        m_charState = AnimState::WALK_IDLE;
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

inline void Demon::handleStateIdleCast()
{
    m_blendAmount += m_blendRate;
    m_blendAmount = glm::min(m_blendAmount, 1.0f);
    m_animator.PlayAnimation(&m_idleAnim, &m_castAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
    if (m_blendAmount >= 1.0f)
    {
        m_blendAmount = 0.0f;
        float startTime = m_animator.m_CurrentTime2;
        m_animator.PlayAnimation(&m_castAnim, NULL, startTime, 0.0f, m_blendAmount);
        m_charState = AnimState::CAST_IDLE;
    }
}

inline void Demon::handleStateCastIdle()
{
    if (m_animator.m_CurrentTime >= m_castAnim.GetDuration() * 0.99f)
    {
        m_blendAmount += m_blendRate;
        m_blendAmount = glm::min(m_blendAmount, 1.0f);
        m_animator.PlayAnimation(&m_castAnim, &m_idleAnim, m_animator.m_CurrentTime, m_animator.m_CurrentTime2, m_blendAmount);
        m_animator.m_CurrentTime = m_castAnim.GetDuration() * 0.99f;
        if (m_blendAmount >= 1.0f)
        {
            m_blendAmount = 0.0f;
            float startTime = m_animator.m_CurrentTime2;
            m_animator.PlayAnimation(&m_idleAnim, NULL, startTime, 0.0f, m_blendAmount);
            m_charState = AnimState::IDLE;
        }
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
    // m_animator.PlayAnimation(m_animator.m_CurrentAnimation, &m_deadAnim, m_animator.m_CurrentTime, 0.0f, m_blendAmount);
    // m_blendAmount += m_blendRate;
    // m_blendAmount = glm::min(m_blendAmount, 1.0f);
    if (m_animator.m_CurrentTime >= m_deadAnim.GetDuration() * 0.99)
    {
        m_animator.m_CurrentTime = m_deadAnim.GetDuration();
    }
}

#endif // DEMON_H