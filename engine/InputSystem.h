#pragma once

#include <algorithm>
#include <string>


#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

const float kMaxRunVelocity = 6;
const float kDashVelocity = 40;
const float kJumpVelocity = 25;
const float kGroundRunAcceleration = 1;
const float kAirRunAcceleration = 0.8f;
const float kDashInterval = 25;

const int kDashCooldown = 30;
const int kDashFrames = 5;

const float kWallJumpVelocityX = 14;
const float kWallJumpVelocityY = 25;
const int kWallJumpFrames = 16;

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& entityManager = engine->GetEntityManager();

        for(const auto& entity : entityManager){
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<InputComponent>(id) ||
                !entityManager.template HasComponent<MovementComponent>(id) ||
                !entityManager.template HasComponent<CollisionComponent>(id)){
                    continue;
            }

            auto& input = entityManager.template GetComponent<InputComponent>(id);
            auto& movement = entityManager.template GetComponent<MovementComponent>(id);
            auto& collision = entityManager.template GetComponent<CollisionComponent>(id);

            bool left = input.keydown[Key::Left];
            bool right = input.keydown[Key::Right];

            bool jumpDown = input.keydown[Key::Z];
            bool jumpWasDown = input.lastKeydown[Key::Z];
            bool jumpPressed = jumpDown && !jumpWasDown;

            if(input.lastDash > 0){
                --input.lastDash;
            }

            bool dashing = input.lastDash > kDashCooldown - kDashFrames;

            bool pushingIntoLeftWall = collision.onLeftWall && left;
            bool pushingIntoRightWall = collision.onRightWall && right;

            bool dashPressed = input.keydown[Key::C] && !input.lastKeydown[Key::C];
            if(dashPressed && input.lastDash <= 0 && (left != right)){
                if((right && !pushingIntoRightWall) || (left && pushingIntoLeftWall)){
                    movement.velocity.x = kDashVelocity;
                } else {
                    movement.velocity.x = -kDashVelocity;
                }
                movement.velocity.y = 0;
                input.lastDash = kDashCooldown;
                dashing = true;

                if(entityManager.template HasComponent<GravityComponent>(id)){
                    entityManager.template GetComponent<GravityComponent>(id).disabledTimer = kDashFrames;
                }
            }

            if(input.wallJumpProtection > 0){
                --input.wallJumpProtection;
            }
            bool wallJumping = input.wallJumpProtection > 0;

            if(jumpPressed && !collision.onGround && movement.velocity.y > 0 && (pushingIntoLeftWall || pushingIntoRightWall)){
                if(pushingIntoLeftWall){
                    movement.velocity.x = kWallJumpVelocityX;
                    input.wallJumpedFrom = -1;
                } else {
                    movement.velocity.x = -kWallJumpVelocityX;
                    input.wallJumpedFrom = 1;
                }

                movement.velocity.y = -kWallJumpVelocityY;
                input.wallJumpProtection = kWallJumpFrames;
                wallJumping = true;
            }

            if(!dashing){
                float accel;
                if(collision.onGround){
                    accel = kGroundRunAcceleration;
                    input.wallJumpedFrom = 0;
                } else {
                    accel = kAirRunAcceleration;
                }

                if(right && !left){
                    movement.velocity.x += accel;
                } else if(left && !right){
                    movement.velocity.x -= accel;
                } else {
                    movement.velocity.x = 0;
                }

                if (!((input.wallJumpedFrom == -1 && right) || (input.wallJumpedFrom == 1 && left))) {
                    movement.velocity.x = std::clamp(movement.velocity.x, -kMaxRunVelocity, kMaxRunVelocity);
                }
            }

            
            if(jumpPressed && collision.onGround){ 
                movement.velocity.y = -kJumpVelocity;
            }

            bool jumpReleased = !jumpDown && jumpWasDown;
            if(jumpReleased && movement.velocity.y < 0){
                movement.velocity.y = 0;
            }

            input.lastKeydown = input.keydown;
            
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
