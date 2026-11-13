#pragma once

#include "Entity.h"
#include "CollisionComponent.h"
#include "LocationComponent.h"

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

            if (collisionComp.staticEntity || !entityManager.template HasComponent<MovementComponent>(id)) {
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

                auto& otherCollisionComp = entityManager.template GetComponent<CollisionComponent>(otherId);

                auto& prevCollisionBox = collisionComp.previousCollisionBox;
                auto& collisionBox = collisionComp.collisionBox;
                auto& otherCollisionBox = otherCollisionComp.collisionBox;

                if (otherCollisionComp.staticEntity) {
                    if (CheckCollision(collisionBox, otherCollisionBox)) {
                        bool wasAbove = prevCollisionBox.topLeft.y + prevCollisionBox.height <= otherCollisionBox.topLeft.y;
                        bool wasBelow = prevCollisionBox.topLeft.y >= otherCollisionBox.topLeft.y + otherCollisionBox.height;
                        bool wasLeft = prevCollisionBox.topLeft.x + prevCollisionBox.width <= otherCollisionBox.topLeft.x;
                        bool wasRight = prevCollisionBox.topLeft.x >= otherCollisionBox.topLeft.x + otherCollisionBox.width;

                        float overlapX = std::min(collisionBox.topLeft.x + collisionBox.width,
                             otherCollisionBox.topLeft.x + otherCollisionBox.width) 
                        - std::max(collisionBox.topLeft.x, otherCollisionBox.topLeft.x);

                        float overlapY = std::min(collisionBox.topLeft.y + collisionBox.height,
                             otherCollisionBox.topLeft.y + otherCollisionBox.height) 
                        - std::max(collisionBox.topLeft.y, otherCollisionBox.topLeft.y);

                        if (overlapX < overlapY) {
                            if (wasLeft || (!wasRight && collisionBox.topLeft.x < otherCollisionBox.topLeft.x)) {
                                locComp.loc.x -= overlapX;
                                collisionComp.onRightWall = true;
                                // if star, bounce
                            } else {
                                locComp.loc.x += overlapX;
                                collisionComp.onLeftWall = true;
                                // if star, bounce
                            }
                        } else {
                            if (wasAbove || (!wasBelow && collisionBox.topLeft.y < otherCollisionBox.topLeft.y)) {
                                locComp.loc.y -= overlapY;
                                collisionComp.onGround = true;
                                // if star, bounce
                            } else {
                                locComp.loc.y += overlapY;
                                collisionComp.onCeiling = true;
                                // if star, bounce
                            }
                        }
                    }
                }
                else { 
                    // Dynamic on dynamic: player on stars and stars and stars
                }
            }
        }

        return kContinue;
    }
    private:
        bool CheckCollision(const Rect& a, const Rect& b) {
            return (a.topLeft.x < b.topLeft.x + b.width &&
                    a.topLeft.x + a.width > b.topLeft.x &&
                    a.topLeft.y < b.topLeft.y + b.height &&
                    a.topLeft.y + a.height > b.topLeft.y);
        }
};

}  // namespace ECSEngine
