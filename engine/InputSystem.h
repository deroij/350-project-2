#pragma once

#include <algorithm>
#include <string>

#include "Keyboard.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine {

const float kMaxRunVelocity = 6;
const float kDashVelocity = 50;
const float kJumpVelocity = 25;
const float kGroundRunAcceleration = 1;
const float kAirRunAcceleration = 0.8f;
const float kDashInterval = 25;

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

            //May need to change later for y acceleration and x acceleration
            float accel = kGroundRunAcceleration;
            
            if(collision.onGround){
                accel = kGroundRunAcceleration;
            } else {
                accel = kAirRunAcceleration;
            }

            bool left = input.keydown[Key::Left];
            bool right = input.keydown[Key::Right];

            if(right && !left){
                movement.velocity.x += accel;
            } else if(left && !right){
                movement.velocity.x -= accel;
            } else {
                movement.velocity.x = 0;
            }

            movement.velocity.x = std::clamp(movement.velocity.x, -kMaxRunVelocity, kMaxRunVelocity);

            bool jumpDown = input.keydown[Key::Z];
            bool jumpWasDown = input.lastKeydown[Key::Z];

            bool jumpPressed = jumpDown && !jumpWasDown;
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
