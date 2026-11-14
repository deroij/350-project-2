#pragma once

#include <unordered_map>
#include <algorithm>   


#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
#include "CameraComponent.h"
#include "EntityManager.h"
#include "CollisionComponent.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

const int kShakeFrames = 8;
const float kShakeThreshold = 10;
const float kShakeScale = 0.3f;
const float kShakeMax = 12;

namespace ECSEngine {

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& entityManager = engine->GetEntityManager();
        auto& windowManager = engine->GetWindowManager();
        float windowWidth = windowManager.GetWindowBounds().width;

        for(auto& entity: entityManager){
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<CameraComponent>(id) ||
                !entityManager.template HasComponent<CameraFollower>(id)){
                    continue;
            }

            auto& camera = entityManager.template GetComponent<CameraComponent>(id);
            auto& follower = entityManager.template GetComponent<CameraFollower>(id);

            if(!entityManager.template HasComponent<LocationComponent>(follower.entityID)){
                continue;
            }
            auto& target = entityManager.template GetComponent<LocationComponent>(follower.entityID);

            if (entityManager.template HasComponent<CameraShake>(id) &&
                entityManager.template HasComponent<MovementComponent>(follower.entityID) &&
                entityManager.template HasComponent<CollisionComponent>(follower.entityID)){
                    auto& shake = entityManager.template GetComponent<CameraShake>(id);
                    auto& targetMove = entityManager.template GetComponent<MovementComponent>(follower.entityID);
                    auto& targetCollision = entityManager.template GetComponent<CollisionComponent>(follower.entityID);

                    if(shake.frameLimit <= 0){
                        float hitX = 0;
                        float hitY = 0;

                        if(targetCollision.onGround && targetMove.velocity.y > kShakeThreshold){
                            hitY = targetMove.velocity.y;
                        } else if(targetCollision.onCeiling && targetMove.velocity.y < -kShakeThreshold){
                            hitY = targetMove.velocity.y;
                        }
                        if((targetCollision.onLeftWall || targetCollision.onRightWall) && (targetMove.velocity.x > kShakeThreshold || targetMove.velocity.x < -kShakeThreshold)){
                            hitX = targetMove.velocity.x;
                        }

                        if(hitX != 0 || hitY != 0){
                            shake.offset.x = std::clamp(hitX * kShakeScale, -kShakeMax, kShakeMax);
                            shake.offset.y = std::clamp(hitY * kShakeScale, -kShakeMax, kShakeMax);
                            shake.frameLimit = kShakeFrames;
                        }
                    }
                }

            float scale = camera.worldUnitsPerPixel;

            float screenX = (target.loc.x - camera.position.x) / scale + windowWidth / 2.0f;
            float hardLeft = camera.hardExclusion.topLeft.x;
            float hardRight = hardLeft + camera.hardExclusion.width;
            float softLeft = camera.softExclusion.topLeft.x;
            float softRight = softLeft + camera.softExclusion.width;

            if(screenX < hardLeft){
                camera.position.x -= (hardLeft - screenX) * scale;
            } else if(screenX > hardRight){
                camera.position.x += (screenX - hardRight)* scale;
            } else if(screenX < softLeft){
                float gap = softLeft - screenX;
                float step;
                if(gap < 1.0f){
                    step = gap;
                } else {
                    step = gap*camera.speed;
                }
                camera.position.x -= step * scale;
            } else if(screenX > softRight){
                float gap = screenX - softRight;
                float step;
                if(gap < 1.0f){
                    step = gap;
                } else {
                    step = gap * camera.speed;
                }
                camera.position.x += step * scale;
            }

            Point2D center = camera.position;
            if(entityManager.template HasComponent<CameraShake>(id)){
                auto& shake = entityManager.template GetComponent<CameraShake>(id);
                if(shake.frameLimit > 0){
                    float strength = shake.frameLimit / static_cast<float>(kShakeFrames);
                    float direction = 1.0f;
                    if(shake.frameLimit % 2 == 0){
                        direction -= 1.0f;
                    }
                    center.x += shake.offset.x * strength * direction;
                    center.y += shake.offset.y * strength * direction;
                    --shake.frameLimit;
                }
            }


            windowManager.SetWorldCenter(center);
            windowManager.SetWorldScale(scale);

            // std::cout << "player x " << target.loc.x << "  y " << target.loc.y << "\n";
            //debugging camera position
        }
        return kContinue;
     }
};

}  // namespace ECSEngine
