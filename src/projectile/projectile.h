#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp> // สำหรับ glm::angle
#include <glm/gtx/norm.hpp>         // สำหรับ glm::length2
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <learnopengl/shader_m.h>
#include <random>

enum ProjectileState
{
    INACTIVE,
    ASCENDING,
    DESCENDING, // สถานะที่เคลื่อนที่ไปยังเป้าหมาย
    IMPACT
};

class Projectile
{
private:
    Model *model;
    Shader *shader;

    // Animator m_animator;

    ProjectileState state;

    glm::vec3 targetPosition;

    glm::vec3 m_direction;
    glm::vec3 m_position;

    float scale;
    float rotationSpeed;
    float verticalVelocity;
    float lifetime;
    float impactDuration;
    float impactTimer;
    float initialAscentHeight;
    float m_currentRotationY; // สำหรับการหมุนชั่วคราวใน ASCENDING
    bool m_headFollowsDirection;

    // 🔴 ตัวแปร RNG ถูกต้องแล้ว
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;

public:
    // Constructor (Implementation ใน .h)
    Projectile(Model &projModel, Shader &projShader, float initialScale = 0.5f, bool headFollows = true)
        : model(&projModel),
          shader(&projShader),
          state(INACTIVE),
          scale(initialScale),
          rotationSpeed(180.0f),
          verticalVelocity(0.0f),
          lifetime(0.0f),
          impactDuration(1.0f),
          impactTimer(0.0f),
          initialAscentHeight(5.0f),
          m_position(0.0f),
          m_direction(0.0f, 0.0f, -1.0f),
          m_currentRotationY(0.0f),
          m_headFollowsDirection(headFollows),
          rng(std::random_device{}()),
          dist(-1.0f, 1.0f)
    {
    }
    // --------------------------------------------------
    // Public Methods
    // --------------------------------------------------
    void Reset();
    void Launch(const glm::vec3 &startPos, const glm::vec3 &forwardDir);
    void Update(float deltaTime);
    void Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos);

    // Getters
    ProjectileState GetState() const { return state; }
    bool IsDestroyed() const { return state == INACTIVE; }
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetDirection() const { return m_direction; }

private:
    // 🔴 ใช้ Projectile:: นำหน้าในการนิยามด้านล่าง
    glm::vec3 GetRandomTarget(const glm::vec3 &forwardDir);
};

// ====================================================================
// 🔴 Implementation
// ====================================================================

// 🔴 ใช้ Projectile:: นำหน้า
inline void Projectile::Reset()
{
    state = INACTIVE;
    lifetime = 0.0f;
    impactTimer = 0.0f;
    m_position = glm::vec3(0.0f);
}

// 🔴 ใช้ Projectile:: นำหน้า
inline glm::vec3 Projectile::GetRandomTarget(const glm::vec3 &forwardDir)
{
    const float AREA_WIDTH = 2.0f;
    const float AREA_DEPTH = 5.0f;
    const float DISTANCE_FRONT = 5.0f;

    // dist(rng) ถูกเรียกใช้ถูกต้องแล้ว
    glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, glm::vec3(0.0f, 1.0f, 0.0f)));

    float randX = dist(rng) * (AREA_WIDTH / 2.0f);
    float randZ = (dist(rng) + 1.0f) * (AREA_DEPTH / 2.0f) + DISTANCE_FRONT;

    glm::vec3 target = (forwardDir * randZ) + (rightDir * randX);
    target.y = 0.0f;
    return target;
}

// 🔴 ใช้ Projectile:: นำหน้า
inline void Projectile::Launch(const glm::vec3 &startPos, const glm::vec3 &forwardDir)
{
    if (state != INACTIVE)
        return;
    state = ASCENDING;
    m_position = startPos;
    m_direction = forwardDir; // บันทึกทิศทางเริ่มต้น

    // ตั้งค่าการหมุนเริ่มต้นตามทิศทางแนวนอน
    m_currentRotationY = glm::degrees(glm::atan(forwardDir.x, forwardDir.z));

    targetPosition = GetRandomTarget(forwardDir);
    verticalVelocity = 10.0f;
    lifetime = 0.0f;
}

// 🔴 ใช้ Projectile:: นำหน้า
inline void Projectile::Update(float deltaTime)
{
    if (state == INACTIVE)
        return;

    // 🌟 FIX 1: อัปเดตแอนิเมชัน
    // m_animator.UpdateAnimation(deltaTime);

    lifetime += deltaTime;

    switch (state)
    {
        // ... (ASCENDING case เหมือนเดิม)
    case ASCENDING:
    {
        m_position.y += verticalVelocity * deltaTime;
        verticalVelocity -= 20.0f * deltaTime;

        m_currentRotationY += rotationSpeed * deltaTime;

        if (verticalVelocity <= 0.0f || m_position.y >= initialAscentHeight)
        {
            state = DESCENDING;
            // m_direction จะถูกคำนวณใน DESCENDING
        }
        break;
    }
    case DESCENDING:
    {
        float speed = 15.0f;

        glm::vec3 directionVector = targetPosition - m_position;

        m_direction = glm::normalize(directionVector);

        m_position += m_direction * speed * deltaTime;

        if (m_position.y <= targetPosition.y || glm::length2(directionVector) < 0.25f)
        { // 0.25f = 0.5^2
            m_position.y = targetPosition.y;
            state = IMPACT;
            impactTimer = impactDuration;
            // 🔴 NEW: ตั้งทิศทางเป็นศูนย์เมื่อปะทะ เพื่อหยุดการเคลื่อนที่และการหมุนทิศทางทันที
            m_direction = glm::vec3(0.0f);
        }
        break;
    }
    case IMPACT:
    {
        // ในสถานะ IMPACT ให้ Fireball หยุดอยู่กับที่
        // Logic ถูกต้องแล้ว: ไม่มีการเปลี่ยนแปลง m_position
        impactTimer -= deltaTime;
        if (impactTimer <= 0.0f)
        {
            Reset();
        }
        break;
    }
    default:
        break;
    }
}

// 🔴 ใช้ Projectile:: นำหน้า
inline void Projectile::Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos)
{
    if (state == INACTIVE)
        return;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, m_position);

    // 🌟 1. คำนวณ Scale (รวม Procedural Pulse Animation)
    float currentScale = scale;
    float currentScaleFactor = 1.0f;

    if (state != IMPACT)
    {
        float pulseSpeed = 10.0f;
        float pulseMagnitude = 0.1f;
        // 🌟 FIX: ใช้ absolute sine เพื่อให้ขนาดสั่นไปมาระหว่าง 1.0 ถึง 1.1
        currentScaleFactor = 1.0f + pulseMagnitude * glm::abs(glm::sin(lifetime * pulseSpeed));
    }
    
    // currentScale *= currentScaleFactor; // 🌟 FIX: ใช้ factor ปรับ Scale

    // Logic Scale Impact เดิม (Scale Explosion)
    if (state == IMPACT)
    {
        float impactFactor = 1.0f - (impactTimer / impactDuration);
        currentScale *= (1.0f + 0.5f * impactFactor); // 🌟 FIX: ใช้ Scale Explosion
    }

    // 🌟 2. [ORIENTATION] คำนวณการหมุนตามทิศทาง (เหมือนเดิม)
    if (state == DESCENDING)
    {
        // ... (Logic การหมุนตามทิศทางเหมือนเดิม) ...
        glm::vec3 originalForward = glm::vec3(0.0f, 0.0f, -1.0f);
        if (!m_headFollowsDirection) originalForward = -originalForward;
        glm::vec3 currentDirection = m_direction;
        glm::vec3 rotationAxis = glm::cross(originalForward, currentDirection);
        float rotationAngle = glm::angle(originalForward, currentDirection);
        if (rotationAngle > 0.0001f)
            modelMatrix = glm::rotate(modelMatrix, rotationAngle, rotationAxis);
    }
    else if (state == ASCENDING)
    {
        // 🌟 Logic การหมุนสปิน (Procedural Rotation)
        modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(m_currentRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 🌟 3. ปรับ Scale
    modelMatrix = glm::scale(modelMatrix, glm::vec3(currentScale)); 

    // 🌟 4. วาด
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", modelMatrix);
    shader->setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    shader->setVec3("viewPos", viewPos);

    // 🌟 NEW: ส่งค่า Emission Pulse (Procedural Color Animation)
    float emissionPulse = (glm::sin(lifetime * 12.0f) + 1.0f) * 0.5f; // Oscillates between 0.0 and 1.0
    shader->setFloat("emissionPulse", emissionPulse);
    // 🌟 NEW: ส่งค่าเวลาสำหรับการ Displace ใน Vertex Shader (ถ้าคุณต้องการใช้)
    shader->setFloat("time", lifetime);

    if (state == IMPACT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // 🌟 ใช้ Model::Draw ทั่วไป (Model ที่ไม่มี Bone)
    model->Draw(*shader);

    if (state == IMPACT)
    {
        glDisable(GL_BLEND);
    }
}

#endif
