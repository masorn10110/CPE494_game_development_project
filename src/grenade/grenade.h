#ifndef HOMING_GRENADE_H
#define HOMING_GRENADE_H

#include <glm/glm.hpp>

// 🔑 Forward declaration (NO include)
class Player;

struct HomingGrenade
{
    glm::vec3 position;
    Player* target = nullptr;

    float speed = 8.0f;
    float stunDuration = 2.5f;
    bool exploded = false;

    void Update(float dt);
};

#endif // HOMING_GRENADE_H
