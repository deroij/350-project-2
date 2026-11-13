#pragma once

#include <unordered_map>

#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
#include "CameraComponent.h"
#include "EntityManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

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
                    center.x += shake.offset.x;
                    center.y += shake.offset.y;
                    --shake.frameLimit;
                }
            }


            windowManager.SetWorldCenter(center);
            windowManager.SetWorldScale(scale);

        }
        return kContinue;
     }
};

}  // namespace ECSEngine
