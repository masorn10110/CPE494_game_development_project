#include "grenade.h"
#include "../player/player.h"

void HomingGrenade::Update(float dt)
{
    if (!target || target->health <= 0)
    {
        exploded = true;
        return;
    }

    glm::vec3 dir = target->position - position;
    float dist = glm::length(dir);

    if (dist < 0.4f)
    {
        target->ApplyStun(stunDuration);
        exploded = true;
        return;
    }

    dir = glm::normalize(dir);
    position += dir * speed * dt;
}
