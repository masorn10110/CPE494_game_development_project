#ifndef HITSCAN_H
#define HITSCAN_H

#include <glm/glm.hpp>
#include <glad/glad.h>          // Needed for glGenBuffers, glBindBuffer, etc.
#include <learnopengl/shader_m.h>

// ----------------------------------------------------------
// Hitscan Class
// ----------------------------------------------------------
class Hitscan
{
public:
    glm::vec3 origin;
    glm::vec3 direction;
    float range;
    float damage;
    bool disabled = false;
    float ttl = 0.05f;  // exists for 50ms

    static constexpr float DEFAULT_RANGE  = 100.0f;
    static constexpr float DEFAULT_DAMAGE = 10.0f;
    static constexpr float HITSCAN_RADIUS = 0.5f;

    Hitscan(glm::vec3 pos, glm::vec3 dir,
            float dmg = DEFAULT_DAMAGE,
            float rng = DEFAULT_RANGE)
        : origin(pos), direction(glm::normalize(dir)), range(rng), damage(dmg)
    {}

    void Update(float dt)
    {
        if (disabled) return;

        ttl -= dt;
        if (ttl <= 0.0f)
            disabled = true;
    }

    // ------------------------------------------------------
    // Check Hit (Only ONE version needed)
    // ------------------------------------------------------
    bool CheckHit(const glm::vec3 &targetPos, float targetRadius, glm::vec3 &hitPoint)
    {
        if (disabled)
            return false;

        glm::vec3 toTarget = targetPos - origin;
        float projection = glm::dot(toTarget, direction);

        if (projection < 0.0f || projection > range)
            return false;

        glm::vec3 closestPoint = origin + projection * direction;
        float distance = glm::length(targetPos - closestPoint);

        if (distance <= (targetRadius + HITSCAN_RADIUS))
        {
            hitPoint = closestPoint;
            disabled = true;
            return true;
        }

        return false;
    }

    // ------------------------------------------------------
    // Debug Rendering
    // ------------------------------------------------------
    static unsigned int lineVAO;
    static unsigned int lineVBO;
    static bool lineInitialized;

    static void InitDebugLine()
    {
        if (lineInitialized) return;

        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);

        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

        // allocate exactly 2 vec3 positions (24 bytes)
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        lineInitialized = true;
    }

    void Draw(Shader &lineShader, const glm::mat4 &view, const glm::mat4 &projection)
    {
        InitDebugLine();

        glm::vec3 start = origin;
        glm::vec3 end   = origin + direction * range;

        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,                sizeof(glm::vec3), &start);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3), sizeof(glm::vec3), &end);

        lineShader.use();
        lineShader.setMat4("view", view);
        lineShader.setMat4("projection", projection);
        lineShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); // red laser

        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINES, 0, 2);
    }
};

// ----------------------------------------------------------
// Static Variable Definitions (MUST be outside the class)
// ----------------------------------------------------------
unsigned int Hitscan::lineVAO = 0;
unsigned int Hitscan::lineVBO = 0;
bool Hitscan::lineInitialized = false;

#endif // HITSCAN_H
