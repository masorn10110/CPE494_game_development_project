#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>
#include <vector>


    // --------------------------
    // AABB Bounding Box Struct
    // --------------------------
    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;

        BoundingBox() : min(0.0f), max(0.0f) {}
        BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
    };

    // --------------------------
    // AABB vs AABB Collision
    // --------------------------
    bool CheckAABBCollision(const BoundingBox &a, const BoundingBox &b)
    {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
               (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
               (a.min.z <= b.max.z && a.max.z >= b.min.z);
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
