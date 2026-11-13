#pragma once

#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class CollisionSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entityManager = engine->GetEntityManager();

        for (const auto& entity : entityManager) {
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<LocationComponent>(id) ||
                !entityManager.template HasComponent<CollisionComponent>(id)) {
                continue;
            }

            auto& locComp = entityManager.template GetComponent<LocationComponent>(id);
            auto& collisionComp = entityManager.template GetComponent<CollisionComponent>(id);

            collisionComp.collisionBox.topLeft.x = locComp.loc.x;
            collisionComp.collisionBox.topLeft.y = locComp.loc.y;

            if (collisionComp.staticEntity) {
                continue;
            }

            collisionComp.onGround = false;
            collisionComp.onRightWall = false;
            collisionComp.onLeftWall = false;
            collisionComp.onCeiling = false;

            for (const auto& otherEntity : entityManager) {
                EntityID otherId = otherEntity.GetID();

                if (id == otherId) {
                    continue;
                }

                if (!entityManager.template HasComponent<LocationComponent>(otherId) ||
                    !entityManager.template HasComponent<CollisionComponent>(otherId)) {
                    continue;
                }

                auto& otherLocComp = entityManager.template GetComponent<LocationComponent>(otherId);
                auto& otherCollisionComp = entityManager.template GetComponent<CollisionComponent>(otherId);

                if (RectIntersects(collisionComp.collisionBox, otherCollisionComp.collisionBox)) {
                    // Handle ground collision
                    if (collisionComp.collisionBox.topLeft.y + collisionComp.collisionBox.height <=
                        otherCollisionComp.collisionBox.topLeft.y) {
                        locComp.loc.y = otherCollisionComp.collisionBox.topLeft.y - collisionComp.collisionBox.height;
                        collisionComp.onGround = true;
                    }
                    // Handle sides collision
                    else if (collisionComp.collisionBox.topLeft.x + collisionComp.collisionBox.width <=
                             otherCollisionComp.collisionBox.topLeft.x) {
                        locComp.loc.x = otherCollisionComp.collisionBox.topLeft.x - collisionComp.collisionBox.width;
                        collisionComp.onRightWall = true;
                    }
                    else if (collisionComp.collisionBox.topLeft.x >=
                             otherCollisionComp.collisionBox.topLeft.x + otherCollisionComp.collisionBox.width) {
                        locComp.loc.x = otherCollisionComp.collisionBox.topLeft.x + otherCollisionComp.collisionBox.width;
                        collisionComp.onLeftWall = true;
                    }
                    // Handle ceiling collision
                    else if (collisionComp.collisionBox.topLeft.y >=
                             otherCollisionComp.collisionBox.topLeft.y + otherCollisionComp.collisionBox.height) {
                        locComp.loc.y = otherCollisionComp.collisionBox.topLeft.y + otherCollisionComp.collisionBox.height;
                        collisionComp.onCeiling = true;
                    }
                }
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
