#ifndef PLAYER_H
#define PLAYER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include "hitscan.h"

class Player
{
public:
    glm::vec3 position;
    glm::vec3 size;
    float yaw;

    bool isMoving;
    glm::vec3 velocity;
    bool isJumping;
    float jumpVelocity;
    glm::vec3 inputDir;

    Model model;
    Model gunModel;

    Animation idleAnim;
    Animation runAnim;

    Animation jumpstartAnim;
    Animation jumploopAnim;
    Animation jumpdownAnim;

    Animation shootAnim;

    Animation grenadeIdleAnim;
    Animation grenadeRunAnim;

    Animation grenadeJumpStartAnim;
    Animation grenadeJumpLoopAnim;
    Animation grenadeJumpDownAnim;

    Animation grenadeThrowAnim;

    Animation deathAnim;
    Animation deathAnimLastframe;

    Animator animator;

    // --- Blend system ---
    enum AnimState
    {
        IDLE,
        IDLE_RUN,
        RUN,
        RUN_IDLE,
        JUMP_START,
        JUMP_MID,
        JUMP_LAND,
        SHOOT,
        DEAD
    };
    AnimState state = IDLE;

    float blendAmount = 0.0f;
    float blendRate = 0.1f;

    float jumpStartTimer = 0.0f;

    float shootCooldown = 0.25f; // seconds between shots
    float shootTimer = 0.0f;     // countdown
    bool isShooting = false;     // for blending (optional)
    // --- Shooting / fire rate ---
    float fireRate = 0.25f;    // seconds per shot
    float fireTimer = 0.0f;    // counts down
    bool shootPressed = false; // input tracking
    bool wasShootPressed = false;

    // --- Optional: for blending ---
    float shootBlendAmount = 0.0f;
    float shootBlendRate = 0.2f; // controls blending speed

    bool holdingGrenade = false;
    bool wasGrenadeTogglePressed = false;

    int health = 100;
    bool deathAnimFinished = false;
    float deathPoseTimer = 0.0f;
    float deathPoseDuration = 10.0f; // seconds to hold last-frame pose
    bool isDead = false;
    glm::vec3 respawnPosition;
    float respawnYaw;

    Player(const std::string &modelPath,
           const std::string &idlePath,
           const std::string &runPath,
           const std::string &jumpstartPath,
           const std::string &jumploopPath,
           const std::string &jumpdownPath,
           const std::string &shootPath,
           const std::string &gunPath,
           const std::string &gIdle,
           const std::string &gRun,
           const std::string &gJumpStart,
           const std::string &gJumpLoop,
           const std::string &gJumpDown,
           const std::string &gThrow,
           const std::string &deathPath,
           const std::string &deathLastFramePath,
           glm::vec3 startPos)
        : model(modelPath),
          idleAnim(idlePath, &model),
          runAnim(runPath, &model),
          jumpstartAnim(jumpstartPath, &model),
          jumploopAnim(jumploopPath, &model),
          jumpdownAnim(jumpdownPath, &model),
          shootAnim(shootPath, &model),
          animator(&idleAnim),
          grenadeIdleAnim(gIdle, &model),
          grenadeRunAnim(gRun, &model),
          grenadeJumpStartAnim(gJumpStart, &model),
          grenadeJumpLoopAnim(gJumpLoop, &model), // somehow the is better
          grenadeJumpDownAnim(gJumpDown, &model),
          grenadeThrowAnim(gThrow, &model),
          deathAnim(deathPath, &model),
          deathAnimLastframe(deathLastFramePath, &model),
          gunModel(gunPath)
    {
        position = startPos;
        respawnPosition = startPos;
        size = glm::vec3(0.5f, 1.0f, 0.5f);
        yaw = 180.0f;
        respawnYaw = 180.0f;
        velocity = glm::vec3(0.0f);

        isMoving = false;
        isJumping = false;
        jumpVelocity = 0.0f;
        inputDir = glm::vec3(0.0f);
    }

    struct AnimSet
    {
        Animation *idle;
        Animation *run;
        Animation *jumpStart;
        Animation *jumpLoop;
        Animation *jumpDown;
        Animation *shoot; // = throw grenade
    };

    AnimSet GetCurrentAnimSet()
    {
        if (holdingGrenade)
        {
            return {
                &grenadeIdleAnim,
                &grenadeRunAnim,
                &grenadeJumpStartAnim,
                &grenadeJumpLoopAnim,
                &grenadeJumpDownAnim,
                &grenadeThrowAnim};
        }
        else
        {
            return {
                &idleAnim,
                &runAnim,
                &jumpstartAnim,
                &jumploopAnim,
                &jumpdownAnim,
                &shootAnim};
        }
    }

    // ============================================================
    // PRIVATE HELPER METHODS
    // ============================================================
private:
    void UpdateDeathState(float dt)
    {
        if (health <= 0)
        {
            if (state != DEAD)
            {
                EnterDeadState();
            }

            if (!deathAnimFinished)
            {
                UpdateDeathAnimation(dt);
            }
            else
            {
                UpdateDeathPose(dt);
            }

            animator.UpdateAnimation(dt);
        }
    }

    void EnterDeadState()
    {
        state = DEAD;
        deathAnimFinished = false;
        deathPoseTimer = 0.0f;
        animator.m_CurrentTime = 0.0f;
        animator.PlayAnimation(&deathAnim, nullptr, 0.0f, 0.0f, 0.0f);
    }

    void UpdateDeathAnimation(float dt)
    {
        float duration = deathAnim.GetDuration() / deathAnim.GetTicksPerSecond() / 2.0f;

        if (animator.m_CurrentTime >= duration)
        {
            deathAnimFinished = true;
            animator.PlayAnimation(&deathAnimLastframe, nullptr, 0.0f, 0.0f, 0.0f);
            animator.m_CurrentTime = 0.0f;
        }
    }

    void UpdateDeathPose(float dt)
    {
        deathPoseTimer += dt;

        if (deathPoseTimer >= deathPoseDuration)
        {
            Respawn();
        }
    }

    void Respawn()
    {
        health = 100;
        isDead = false;
        velocity = glm::vec3(0.0f);
        isJumping = false;
        position = respawnPosition;
        yaw = respawnYaw;
        state = IDLE;
        animator.m_CurrentTime = 0.0f;
        animator.PlayAnimation(&idleAnim, nullptr, 0.0f, 0.0f, 0.0f);
    }

    void UpdateInput(bool up, bool down, bool left, bool right, bool tggrenade)
    {
        inputDir = glm::vec3(0.0f);
        if (up)
            inputDir.z -= 1.0f;
        if (down)
            inputDir.z += 1.0f;
        if (left)
            inputDir.x -= 1.0f;
        if (right)
            inputDir.x += 1.0f;

        bool wantsToMove = (glm::length(inputDir) > 0.1f);
        if (wantsToMove)
            inputDir = glm::normalize(inputDir);

        HandleGrenadeToggle(tggrenade);
    }

    void HandleGrenadeToggle(bool tggrenade)
    {
        bool grenadeEdge = (tggrenade && !wasGrenadeTogglePressed);
        wasGrenadeTogglePressed = tggrenade;

        if (grenadeEdge)
        {
            holdingGrenade = !holdingGrenade;
            AnimSet set = GetCurrentAnimSet();
            animator.PlayAnimation(set.idle, nullptr, 0.0f, 0.0f, 0.0f);
        }
    }

    void UpdateMovement(float dt)
    {
        velocity = glm::vec3(0.0f);
        const float accel = 100.0f;

        if (glm::length(inputDir) > 0.1f)
            velocity += inputDir * accel * dt;

        isMoving = glm::length(velocity) > 0.05f;
        position += velocity * dt;

        if (isMoving)
        {
            float targetYaw = glm::degrees(atan2(velocity.x, velocity.z));
            float diff = fmodf(targetYaw - yaw + 540.0f, 360.0f) - 180.0f;
            yaw += diff * 10.0f * dt; // smooth turning
        }
    }

    void UpdateJump(float dt, const AnimSet &set)
    {
        const float jumpForce = 8.5f;
        const float gravity = -20.0f;

        if (false)
        {
        } // placeholder for jump input check

        if (isJumping)
        {
            position.y += jumpVelocity * dt;
            jumpVelocity += gravity * dt;

            if (position.y <= 0.0f)
            {
                position.y = 0.0f;
                isJumping = false;
                state = JUMP_LAND;
                blendAmount = 0.0f;

                Animation *endAnim = isMoving ? set.run : set.idle;
                animator.PlayAnimation(set.jumpDown, endAnim, animator.m_CurrentTime, 0.0f, 0.0f);
            }
        }
    }

    void InitiateJump(const AnimSet &set)
    {
        const float jumpForce = 8.5f;
        if (!isJumping && position.y <= 0.01f)
        {
            isJumping = true;
            jumpVelocity = jumpForce;
            state = JUMP_START;
            blendAmount = 0.0f;
            jumpStartTimer = 0.0f;

            animator.m_CurrentTime = 0.0f; // <-- IMPORTANT FIX
            animator.m_CurrentTime2 = 0.0f;

            Animation *from = isMoving ? set.run : set.idle;
            animator.PlayAnimation(from, set.jumpStart, 0.0f, 0.0f, 0.0f);
        }
    }

    void UpdateShooting(float dt, const AnimSet &set)
    {
        if (fireTimer > 0.0f)
            fireTimer -= dt;

        // Only track shoot press if NOT holding grenade (continuous fire)
        if (!holdingGrenade)
        {
            wasShootPressed = false;
        }
        // If holding grenade, wasShootPressed is managed in HandleGrenadeThrow
    }

    void HandleShoot(const AnimSet &set)
    {
        // Only allow hitscan shooting when NOT holding grenade
        if (!holdingGrenade && fireTimer <= 0.0f && !isJumping && !isMoving)
        {
            fireTimer = fireRate;
            shootBlendAmount = 1.0f;

            // Fire hitscan ray
            FireHitscan();

            Animation *baseAnim = isMoving ? set.run : set.idle;
            animator.PlayAnimation(baseAnim, set.shoot, 0.0f, 0.0f, shootBlendAmount);

            animator.m_CurrentTime = 0.0f;
            state = SHOOT;
        }
    }

    void HandleGrenadeThrow(bool shoot, const AnimSet &set)
    {
        // Only allow grenade throw if holding grenade
        if (!holdingGrenade)
            return;

        // Edge detection: fire only on button press, not hold
        bool throwEdge = (shoot && !wasShootPressed);
        wasShootPressed = shoot;

        if (throwEdge && !isJumping && !isMoving)
        {
            shootBlendAmount = 1.0f;

            // Throw grenade animation
            Animation *baseAnim = isMoving ? set.run : set.idle;
            animator.PlayAnimation(baseAnim, set.shoot, 0.0f, 0.0f, shootBlendAmount);

            animator.m_CurrentTime = 0.0f;
            state = SHOOT;
        }
    }

    void UpdateAnimationStateMachine(float dt, const AnimSet &set)
    {
        auto EaseInOut = [](float x)
        { return x * x * (3.0f - 2.0f * x); };

        switch (state)
        {
        case IDLE:
            if (!isJumping && isMoving)
            {
                blendAmount = 0.0f;
                state = IDLE_RUN;
                animator.PlayAnimation(set.idle, set.run, animator.m_CurrentTime, 0.0f, 0.0f);
            }
            break;

        case IDLE_RUN:
        {
            blendAmount += blendRate;
            float eased = EaseInOut(glm::clamp(blendAmount, 0.0f, 1.0f));
            animator.PlayAnimation(set.idle, set.run, animator.m_CurrentTime, animator.m_CurrentTime2, eased);

            if (blendAmount >= 1.0f)
            {
                animator.PlayAnimation(set.run, nullptr, animator.m_CurrentTime2, 0.0f, 0.0f);
                state = RUN;
                blendAmount = 0.0f;
            }
        }
        break;

        case RUN:
            if (!isMoving)
            {
                state = RUN_IDLE;
                blendAmount = 0.0f;
            }
            animator.PlayAnimation(set.run, nullptr, animator.m_CurrentTime, animator.m_CurrentTime2, 0.0f);
            break;

        case RUN_IDLE:
        {
            blendAmount += blendRate;
            float eased = EaseInOut(glm::clamp(blendAmount, 0.0f, 1.0f));
            animator.PlayAnimation(set.run, set.idle, animator.m_CurrentTime, animator.m_CurrentTime2, eased);

            if (blendAmount >= 1.0f)
            {
                animator.PlayAnimation(set.idle, nullptr, animator.m_CurrentTime2, 0.0f, 0.0f);
                state = IDLE;
                blendAmount = 0.0f;
            }
        }
        break;

        case JUMP_START:
        {
            jumpStartTimer += dt;
            float jumpStartDuration = set.jumpStart->GetDuration() / (set.jumpStart->GetTicksPerSecond());
            animator.PlayAnimation(set.jumpStart, nullptr, animator.m_CurrentTime, 0.0f, 0.0f);

            if (jumpStartTimer >= jumpStartDuration)
            {
                state = JUMP_MID;
                animator.PlayAnimation(set.jumpLoop, nullptr, 0.0f, 0.0f, 0.0f);
                blendAmount = 0.0f;
            }
        }
        break;

        case JUMP_MID:
            if (blendAmount == 0.0f)
            {
                animator.PlayAnimation(set.jumpLoop, nullptr, 0.0f, 0.0f, 0.0f);
            }

            if (jumpVelocity < -4.0f)
            {
                state = JUMP_LAND;
                blendAmount = 0.0f;
            }
            break;

        case JUMP_LAND:
        {
            blendAmount += blendRate * 1.4f;
            float eased = EaseInOut(glm::clamp(blendAmount, 0.0f, 1.0f));

            Animation *endAnim = isMoving ? set.run : set.idle;
            animator.PlayAnimation(set.jumpDown, endAnim, animator.m_CurrentTime, animator.m_CurrentTime2, eased);

            if (blendAmount >= 1.0f)
            {
                animator.PlayAnimation(endAnim, nullptr, animator.m_CurrentTime2, 0.0f, 0.0f);
                state = isMoving ? RUN : IDLE;
                blendAmount = 0.0f;
            }
        }
        break;

        case SHOOT:
        {
            float duration = set.shoot->GetDuration() / set.shoot->GetTicksPerSecond();

            if (animator.m_CurrentTime >= duration)
            {
                state = isMoving ? RUN : IDLE;
                shootBlendAmount = 0.0f;

                Animation *baseAnim = isMoving ? set.run : set.idle;
                animator.PlayAnimation(baseAnim, nullptr, animator.m_CurrentTime, 0.0f, 0.0f);
            }
        }
        break;
        }
    }

public:
    // ============================================================
    // PUBLIC UPDATE METHOD
    // ============================================================
    void Update(float dt, bool up, bool down, bool left, bool right, bool jump, bool shoot, bool tggrenade)
    {
        // Check death state first
        if (health <= 0)
        {
            UpdateDeathState(dt);
            return;
        }

        AnimSet set = GetCurrentAnimSet();

        UpdateInput(up, down, left, right, tggrenade);
        UpdateMovement(dt);

        if (jump)
            InitiateJump(set);

        UpdateJump(dt, set);

        // Handle shooting differently based on weapon type
        if (holdingGrenade)
        {
            HandleGrenadeThrow(shoot, set);
        }
        else
        {
            if (shoot)
                HandleShoot(set);
        }

        UpdateShooting(dt, set);
        UpdateAnimationStateMachine(dt, set);

        animator.UpdateAnimation(dt);
    }

    void FireHitscan()
    {
        // Calculate forward direction based on player yaw
        float yawRad = glm::radians(yaw);
        glm::vec3 fireDir(sin(yawRad), 0.0f, cos(yawRad));

        // Fire from hand position
        glm::vec3 firePos = position + glm::vec3(0.0f, 0.5f, 0.0f);

        // Create hitscan ray - store it temporarily or use it immediately
        // You'll call this in your game loop to check collisions
        lastHitscan = Hitscan(firePos, fireDir, 10.0f);
    }

    // Store last hitscan for collision checking in game loop
    Hitscan lastHitscan = Hitscan(glm::vec3(0), glm::vec3(0, 0, 1), 10.0f);

    // ----------------------------------------------------
    // DRAW FUNCTION (unchanged)
    // ----------------------------------------------------
    void Draw(Shader &animShader, Shader &lightingShader,
              const glm::mat4 &view, const glm::mat4 &projection)
    {
        //------------------------------------------------------------
        // 1. Draw the animated player model
        //------------------------------------------------------------
        animShader.use();
        animShader.setMat4("projection", projection);
        animShader.setMat4("view", view);

        // upload bone matrices
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        // player world matrix
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::rotate(modelMat, glm::radians(yaw), glm::vec3(0, 1, 0));
        modelMat = glm::scale(modelMat, glm::vec3(0.5f)); // your character scale
        animShader.setMat4("model", modelMat);

        model.Draw(animShader);

        //------------------------------------------------------------
        // 2. Draw the gun, using the hand bone transform
        //------------------------------------------------------------
        const std::string handName = "mixamorig_RightHand";

        if (animator.m_BoneWorldTransforms.find(handName) == animator.m_BoneWorldTransforms.end())
        {
            std::cout << "ERROR: Hand bone NOT FOUND: " << handName << std::endl;
            return;
        }

        // Bone transform ALREADY in local model space (no modelMat!)
        glm::mat4 handTransform = animator.m_BoneWorldTransforms[handName];

        // small alignment offset (start small)
        glm::mat4 gunOffset = glm::mat4(1.0f);
        // gunOffset = glm::translate(gunOffset, glm::vec3(0.05f, -0.05f, 0.15f));
        gunOffset = glm::rotate(gunOffset, glm::radians(-90.0f), glm::vec3(0, 1, 0));
        gunOffset = glm::rotate(gunOffset, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        // gunOffset = glm::scale(gunOffset, glm::vec3(0.5f));  // adjust to your gun size

        // FINAL world transform:
        //      Player transform * Bone transform * alignment
        glm::mat4 gunMatrix = modelMat * handTransform * gunOffset;

        //------------------------------------------------------------
        // 3. Draw gun with lighting shader
        //------------------------------------------------------------
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("model", gunMatrix);

        if (!holdingGrenade)
            gunModel.Draw(lightingShader);
    }
};

#endif // PLAYER_H