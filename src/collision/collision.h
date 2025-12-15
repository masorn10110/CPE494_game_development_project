#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include <vector>

// --------------------------
// AABB Bounding Box Struct
// --------------------------
struct BoundingBox
{
    glm::vec3 minCorner;
    glm::vec3 maxCorner;

    BoundingBox() : minCorner(0.0f), maxCorner(0.0f) {}
    BoundingBox(glm::vec3 min, glm::vec3 max) : minCorner(min), maxCorner(max) {}
};

// --------------------------
// AABB vs AABB Collision
// --------------------------
inline bool CheckAABBCollision(const BoundingBox &a, const BoundingBox &b)
{
    return (a.minCorner.x <= b.maxCorner.x && a.maxCorner.x >= b.minCorner.x) &&
           (a.minCorner.y <= b.maxCorner.y && a.maxCorner.y >= b.minCorner.y) &&
           (a.minCorner.z <= b.maxCorner.z && a.maxCorner.z >= b.minCorner.z);
}

// --------------------------
// Wall Collision System
// --------------------------
inline bool CheckWallCollision(
    glm::vec3 &characterPosition,
    const glm::vec3 &previousPosition,
    float radius,
    const std::vector<BoundingBox> &walls)
{
    // Create a bounding box around the character
    BoundingBox charBox(
        characterPosition - glm::vec3(radius, 0.0f, radius),
        characterPosition + glm::vec3(radius, 3.0f, radius));

    for (const auto &wall : walls)
    {
        if (CheckAABBCollision(charBox, wall))
        {
            characterPosition = previousPosition; // rollback
            return true;
        }
    }

    return false;
}

#endif
