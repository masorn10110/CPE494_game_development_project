#ifndef HOMING_GRENADE_H
#define HOMING_GRENADE_H

#include <glm/glm.hpp>

// 🔑 Forward declarations (NO heavy includes)
class Player;
class Shader;

struct HomingGrenade
{
    glm::vec3 position;
    Player *target = nullptr;

    float speed = 16.0f;
    float stunDuration = 2.5f;

    bool exploded = false;

    // 💥 Explosion visuals
    bool drawExplosion = false;
    float explosionTimer = 0.0f;
    float explosionDuration = 0.5f;

    void Update(float dt);

    // 🔥 MUST MATCH CPP EXACTLY
    void Draw(Shader &shader,
              const glm::mat4 &view,
              const glm::mat4 &projection);
};

#endif // HOMING_GRENADE_H
