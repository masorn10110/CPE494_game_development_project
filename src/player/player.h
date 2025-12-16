#ifndef PLAYER_H
#define PLAYER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

// #include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include "../playergun/hitscan.h"
#include "../collision/collision.h"
#include "../grenade/grenade.h"

class Player
{
public:
    glm::vec3 position;
    glm::vec3 previousPosition;
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

    Animation stunAnim;

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
        STUNNED,
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
    float fireRate = 1.0f;     // seconds per shot
    float fireTimer = 0.0f;    // counts down
    bool shootPressed = false; // input tracking
    bool wasShootPressed = false;

    // --- Optional: for blending ---
    float shootBlendAmount = 0.0f;
    float shootBlendRate = 0.2f; // controls blending speed

    bool holdingGrenade = false;
    bool wasGrenadeTogglePressed = false;
    float grenadeCooldown = 7.0f; // seconds between grenade throws
    float grenadeTimer = 0.0f;    // countdown timer
    HomingGrenade activeGrenade;
    bool grenadeActive = false;
    bool isStunned = false;
    float stunTimer = 0.0f;
    float stunDuration = 0.0f;

    int health = 100;
    bool deathAnimFinished = false;
    float deathPoseTimer = 0.0f;
    float deathPoseDuration = 10.0f; // seconds to hold last-frame pose
    bool isDead = false;
    glm::vec3 respawnPosition;
    float respawnYaw;
    float invulnerabilityTimer = 0.0f;

    float playerradius = 0.5f; // for hitscan collision

    unsigned int score = 0;

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
           const std::string &stunPath,
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
          stunAnim(stunPath, &model),
          gunModel(gunPath)
    {
        position = startPos;
        previousPosition = startPos;
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
                this->score -= this->score > 50 ? 50 : this->score;
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

    void UpdateMovement(float dt, const std::vector<BoundingBox> &worldWalls)
    {
        velocity = glm::vec3(0.0f);
        const float speed = 2.5f;

        if (glm::length(inputDir) > 0.1f)
            velocity = inputDir * speed * dt; // displacement this frame

        isMoving = glm::length(velocity) > 0.01f;

        ResolveWallCollisions(worldWalls);

        if (isMoving)
        {
            float targetYaw = glm::degrees(atan2(velocity.x, velocity.z));
            float diff = fmodf(targetYaw - yaw + 540.0f, 360.0f) - 180.0f;
            yaw += diff * 10.0f * dt;
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

    void ThrowGrenade(Player *target)
    {
        if (grenadeActive)
            return; // only one grenade at a time
        if (grenadeTimer > 0.0f)
            return; // respect cooldown

        activeGrenade.position = position + glm::vec3(0.0f, 0.6f, 0.0f);
        activeGrenade.target = target;
        activeGrenade.exploded = false;

        grenadeActive = true;
        grenadeTimer = grenadeCooldown; // reset cooldown
    }

    void HandleGrenadeThrow(bool shoot, const AnimSet &set, Player* target)
    {
        if (!holdingGrenade) return;

        bool throwEdge = (shoot && !wasShootPressed);
        wasShootPressed = shoot;

        if (throwEdge && !isJumping && !isMoving && !grenadeActive && grenadeTimer <= 0.0f)
        {
            // Play throw animation
            Animation *baseAnim = isMoving ? set.run : set.idle;
            animator.PlayAnimation(baseAnim, set.shoot, 0.0f, 0.0f, 1.0f);

            animator.m_CurrentTime = 0.0f;
            state = SHOOT;

            // Activate grenade
            ThrowGrenade(target); // <- must call this
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

    void ResolveWallCollisions(const std::vector<BoundingBox> &worldWalls)
    {
        glm::vec3 nextPos = position;
        glm::vec3 safePos = previousPosition; // track last safe position

        // X axis
        nextPos.x += velocity.x;
        if (CheckWallCollision(nextPos, safePos, playerradius, worldWalls))
        {
            nextPos.x = safePos.x; // rollback X
        }
        else
        {
            safePos.x = nextPos.x; // update safe X
        }

        // Z axis
        nextPos.z += velocity.z;
        if (CheckWallCollision(nextPos, safePos, playerradius, worldWalls))
        {
            nextPos.z = safePos.z; // rollback Z
        }
        else
        {
            safePos.z = nextPos.z; // update safe Z
        }

        position = nextPos;
        previousPosition = safePos; // update overall safe position
    }

    void UpdateStun(float dt)
    {
        if (!isStunned)
            return;

        stunTimer -= dt;
        velocity = glm::vec3(0.0f);
        isMoving = false;

        if (stunTimer <= 0.0f)
        {
            isStunned = false;
            state = IDLE;
            animator.PlayAnimation(&idleAnim, nullptr, 0.0f, 0.0f, 0.0f);
        }
    }

public:
    // ============================================================
    // PUBLIC UPDATE METHOD
    // ============================================================
    void Update(float dt, bool up, bool down, bool left, bool right, bool jump, bool shoot, bool tggrenade, float deltaTime, const std::vector<BoundingBox> &worldWalls, Player *target)
    {
        // Check death state first
        if (health <= 0)
        {
            UpdateDeathState(dt);
            return;
        }

        if (isStunned)
        {
            UpdateStun(dt);
            animator.UpdateAnimation(dt);
            return; // 🚫 stop movement / shooting
        }
        // Update grenade cooldown timer
        if (grenadeTimer > 0.0f)
            grenadeTimer -= dt;
        if (invulnerabilityTimer > 0.0f)
        {
            invulnerabilityTimer -= deltaTime;
        }

        AnimSet set = GetCurrentAnimSet();

        UpdateInput(up, down, left, right, tggrenade);
        UpdateMovement(dt, worldWalls);

        if (jump)
            InitiateJump(set);

        UpdateJump(dt, set);

        // Handle shooting differently based on weapon type
        if (holdingGrenade)
        {
            HandleGrenadeThrow(shoot, set, target);
        }
        else
        {
            if (shoot)
                HandleShoot(set);
        }
        if (grenadeActive)
        {
            activeGrenade.Update(dt);

            if (activeGrenade.exploded)
            {
                grenadeActive = false;
            }
        }

        UpdateShooting(dt, set);
        UpdateAnimationStateMachine(dt, set);

        animator.UpdateAnimation(dt);
        lastHitscan.Update(dt);
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

    void ApplyStun(float duration)
    {
        if (health <= 0)
            return;

        isStunned = true;
        stunDuration = duration;
        stunTimer = duration;

        velocity = glm::vec3(0.0f);
        state = STUNNED;

        // Optional: play stunned animation or idle
        animator.PlayAnimation(&stunAnim, nullptr, 0.0f, 0.0f, 0.0f);
    }

    // ----------------------------------------------------
    // DRAW FUNCTION (unchanged)
    // ----------------------------------------------------
    void Draw(Shader &animShader, Shader &lightingShader, Shader &hitscanShader, Shader &explodeShader,
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
        if (grenadeActive)
            activeGrenade.Draw(explodeShader, view, projection);

        lastHitscan.Draw(hitscanShader, view, projection);
    }

    void Reset(glm::vec3 startPos)
    {
        // 1. รีเซ็ตสถานะพื้นฐาน
        position = startPos;
        previousPosition = startPos;
        respawnPosition = startPos; // ถ้าอยากเปลี่ยนจุดเกิดให้เปลี่ยนที่นี่ด้วย

        yaw = 180.0f; // หรือค่าเริ่มต้นที่คุณต้องการ
        velocity = glm::vec3(0.0f);

        health = 100;
        score = 0; // รีเซ็ตคะแนนด้วย (ถ้าต้องการ)

        isDead = false;
        deathAnimFinished = false;
        deathPoseTimer = 0.0f;

        isMoving = false;
        isJumping = false;
        jumpVelocity = 0.0f;

        // 2. รีเซ็ตสถานะการโจมตี
        isShooting = false;
        shootTimer = 0.0f;
        fireTimer = 0.0f;
        shootPressed = false;
        wasShootPressed = false;

        holdingGrenade = false;
        wasGrenadeTogglePressed = false;

        grenadeActive = false;
        isStunned = false;
        stunTimer = 0.0f;
        stunDuration = 0.0f;
        grenadeCooldown = 7.0f; // seconds between grenade throws
        grenadeTimer = 0.0f;

        // 3. รีเซ็ต Animation State
        state = IDLE;
        blendAmount = 0.0f;
        shootBlendAmount = 0.0f;
        jumpStartTimer = 0.0f;

        animator.m_CurrentTime = 0.0f;
        animator.m_CurrentTime2 = 0.0f;
        animator.PlayAnimation(&idleAnim, nullptr, 0.0f, 0.0f, 0.0f);

        // 4. รีเซ็ตค่าอมตะ (Invulnerability)
        invulnerabilityTimer = 0.0f;

        // 5. รีเซ็ต Hitscan (ย้ายไปจุดปลอดภัย)
        lastHitscan = Hitscan(glm::vec3(0), glm::vec3(0, 0, 1), 10.0f);
    }
};

#endif // PLAYER_H