#include "grenade.h"
#include "../player/player.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>

// ------------------------------------------------------------
// 🔥 Explosion Quad (local to this file)
// ------------------------------------------------------------
static unsigned int explosionVAO = 0;
static unsigned int explosionVBO = 0;

static void InitExplosionQuad()
{
    if (explosionVAO != 0) return;

    float quadVertices[] = {
        // positions          // UV
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,

        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f
    };

    glGenVertexArrays(1, &explosionVAO);
    glGenBuffers(1, &explosionVBO);

    glBindVertexArray(explosionVAO);
    glBindBuffer(GL_ARRAY_BUFFER, explosionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

// ------------------------------------------------------------
// 🧨 Update
// ------------------------------------------------------------
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
        drawExplosion = true;
        explosionTimer = 0.0f;
        return;
    }

    dir = glm::normalize(dir);
    position += dir * speed * dt;

    if (drawExplosion)
    {
        explosionTimer += dt;
        if (explosionTimer >= explosionDuration)
            drawExplosion = false;
    }
}

// ------------------------------------------------------------
// 🎨 Draw Explosion
// ------------------------------------------------------------
void HomingGrenade::Draw(Shader& shader,
                         const glm::mat4& view,
                         const glm::mat4& projection)
{
    if (!drawExplosion) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive glow
    glDepthMask(GL_FALSE);

    InitExplosionQuad();

    float t = explosionTimer / explosionDuration;
    t = glm::clamp(t, 0.0f, 1.0f);

    float scale = glm::mix(0.2f, 1.5f, t);
    float alpha = 1.0f - t;

    static float explosionTime = 0.0f;
    explosionTime += 0.016f; // or dt if you pass it

    shader.use();
    shader.setFloat("time", explosionTime);
    shader.setVec4("color", glm::vec4(1.0f));

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    // shader.setVec4("color", glm::vec4(1.0f, 0.6f, 0.1f, alpha));

    // Billboard
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    glm::mat4 billboard = glm::mat4(glm::mat3(view));
    model *= glm::inverse(billboard);

    model = glm::scale(model, glm::vec3(scale));
    shader.setMat4("model", model);

    glBindVertexArray(explosionVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
