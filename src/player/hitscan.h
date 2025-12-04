#ifndef HITSCAN_H
#define HITSCAN_H

#include <glm/glm.hpp>

class Hitscan
{
public:
    glm::vec3 origin;
    glm::vec3 direction;
    float range;
    float damage;
    bool disabled = false;

    static constexpr float DEFAULT_RANGE = 100.0f;
    static constexpr float DEFAULT_DAMAGE = 10.0f;
    static constexpr float HITSCAN_RADIUS = 0.5f; // raycast width

    Hitscan(glm::vec3 pos, glm::vec3 dir, float dmg = DEFAULT_DAMAGE, float rng = DEFAULT_RANGE)
        : origin(pos), direction(glm::normalize(dir)), damage(dmg), range(rng)
    {
    }

    // Check if hitscan ray hits a sphere (demon)
    bool CheckHit(const glm::vec3 &targetPos, float targetRadius, glm::vec3 &hitPoint)
    {
        if (disabled)
        {
            return false;
        }
        
        // Vector from ray origin to target
        glm::vec3 toTarget = targetPos - origin;

        // Project onto ray direction
        float projection = glm::dot(toTarget, direction);

        // Check if target is behind the ray
        if (projection < 0.0f || projection > range)
        {
            return false;
        }

        // Find closest point on ray to target
        glm::vec3 closestPoint = origin + direction * projection;

        // Distance from target to closest point on ray
        float distance = glm::length(targetPos - closestPoint);

        // Check if within hit radius
        if (distance <= (targetRadius + HITSCAN_RADIUS))
        {
            hitPoint = closestPoint;
            disabled = true; // Disable after hit
            return true;
        }

        return false;
    }
};

#endif