#ifndef STONEWALL_H
#define STONEWALL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

// กำหนดค่าคงที่ (อ้างอิงจากขนาดโมเดล OBJ)
#define WALL_RISE_TIME 0.5f // เวลายกกำแพง
#define WALL_FADE_TIME 0.5f // เวลาที่กำแพงจะจางหายไป

class StoneWall
{
private:
    Model *model;
    Shader *shader;

    glm::vec3 m_centerPosition;   // ตำแหน่งศูนย์กลางของฐานกำแพง
    glm::vec3 m_forwardDirection; // ทิศทางที่ Demon หันหน้า (สำหรับวางกำแพงให้ตั้งฉาก)

    // m_segmentCount ถูกใช้เป็นตัวกำหนดความยาว (Length Multiplier)
    int m_lengthMultiplier;
    float m_maxHeight; // ความสูงสูงสุดที่กำแพงจะพุ่งขึ้น
    float m_duration;  // ระยะเวลาคงอยู่ทั้งหมด (รวม rise และ fade)
    float m_lifetime;  // เวลาที่ผ่านไป

    // ค่าคงที่สำหรับ Scale
    const float WALL_THICKNESS_SCALE = 0.25f;
    const float WALL_DEFAULT_DISTANCE = 13.5f;

public:
    StoneWall(Model &wallModel, Shader &wallShader, float maxHeight = 1.0f)
        : model(&wallModel),
          shader(&wallShader),
          m_maxHeight(maxHeight),
          m_lifetime(0.0f),
          m_lengthMultiplier(0),
          m_duration(0.0f)
    {
    }

    void Cast(const glm::vec3 &casterPos, const glm::vec3 &forwardDir, int lengthMultiplier, float totalDuration);
    void Update(float deltaTime);
    void Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos);

    glm::vec3 GetCenterPosition() const { return m_centerPosition; }
    glm::vec3 GetForwardDirection() const { return m_forwardDirection; }
    int GetLengthMultiplier() const { return m_lengthMultiplier; }
    bool IsActive() const { return m_lifetime < m_duration; }
    void Reset()
    {
        m_lifetime = m_duration + 1.0f; // ตั้งค่าให้เกินเวลา เพื่อให้ IsActive() เป็น false
    }
};

// ====================================================================
// 🔴 Implementation
// ====================================================================

inline void StoneWall::Cast(const glm::vec3 &casterPos, const glm::vec3 &forwardDir, int lengthMultiplier, float totalDuration)
{
    // 1. คำนวณทิศทางไปข้างหน้า (Horizontal Forward)
    m_forwardDirection = glm::normalize(glm::vec3(forwardDir.x, 0.0f, forwardDir.z));

    // 2. คำนวณทิศทางด้านข้าง (Right Vector)
    // rightDir = cross(forward, up)
    glm::vec3 rightDir = glm::normalize(glm::cross(m_forwardDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

    // 3. กำหนดค่า Offset ด้านข้างที่ต้องการ (เช่น 2.0f)
    // float sideOffset = 1.5f; // 👈 ปรับค่านี้: +2.0f คือไปทางขวา, -2.0f คือไปทางซ้าย

    // 4. กำหนดตำแหน่งศูนย์กลางกำแพง
    m_centerPosition = casterPos + m_forwardDirection * WALL_DEFAULT_DISTANCE;

    // 🌟 FIX: เพิ่ม Offset ด้านข้าง (แกน X)
    // m_centerPosition += rightDir * sideOffset;
    m_centerPosition += rightDir;

    m_centerPosition.y = casterPos.y;

    m_lengthMultiplier = lengthMultiplier;
    m_duration = totalDuration;
    m_lifetime = 0.0f;
}

inline void StoneWall::Update(float deltaTime)
{
    if (!IsActive())
        return;
    m_lifetime += deltaTime;
}

inline void StoneWall::Draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &viewPos)
{
    if (!IsActive())
        return;

    float riseTime = WALL_RISE_TIME;
    float fadeTime = WALL_FADE_TIME;

    float y_displacement = 0.0f;
    float currentAlpha = 1.0f;

    // --- 1. คำนวณ Animation State ---
    if (m_lifetime < riseTime)
    {
        y_displacement = (m_lifetime / riseTime) * m_maxHeight;
    }
    else
    {
        y_displacement = m_maxHeight;
        if (m_lifetime > m_duration - fadeTime)
        {
            float fadeTimer = m_lifetime - (m_duration - fadeTime);
            currentAlpha = 1.0f - glm::clamp(fadeTimer / fadeTime, 0.0f, 1.0f);
        }
    }

    // --- 2. คำนวณ Rotation ---
    glm::vec3 wallDirection = m_forwardDirection;
    float angle = glm::atan(wallDirection.x, wallDirection.z);
    float finalRotationAngle = angle;

    // --- 3. ตั้งค่า Shader และ วาดกำแพงชิ้นเดียว ---
    shader->use();

    // 🟢 Uniforms
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setFloat("alphaFade", currentAlpha);
    shader->setFloat("emissionIntensity", currentAlpha);
    shader->setVec3("viewPos", viewPos);

    // 🔴 ตำแหน่ง (Translation)
    glm::vec3 segmentWorldPos = m_centerPosition + glm::vec3(0.5, 0, 0);
    float initialYOffset = m_centerPosition.y - m_maxHeight;
    segmentWorldPos.y = initialYOffset + y_displacement;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, segmentWorldPos);

    // 🔴 Rotation
    modelMatrix = glm::rotate(modelMatrix, finalRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    // 🔴 Scale (X: ความยาว, Y: ความสูง, Z: ความหนา)
    float lengthScale = (float)m_lengthMultiplier;
    float thicknessScale = WALL_THICKNESS_SCALE;

    modelMatrix = glm::scale(modelMatrix, glm::vec3(thicknessScale, m_maxHeight, lengthScale));

    shader->setMat4("model", modelMatrix);

    // 🔴 วาด
    model->Draw(*shader);
}

#endif