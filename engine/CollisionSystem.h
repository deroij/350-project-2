#pragma once

#include "Entity.h"
#include "CollisionComponent.h"
#include "LocationComponent.h"
#include "ScoreComponent.h"
#include "InputComponent.h"

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

        int score = 0;

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

                bool isPlayer = entityManager.template HasComponent<InputComponent>(id);

                if (otherCollisionComp.staticEntity) {
                    if (CheckCollision(collisionBox, otherCollisionBox)) {
                        if (isPlayer && otherCollisionComp.doesDamage) {
                            auto& movement = entityManager.template GetComponent<MovementComponent>(id);
                            auto& respawn = entityManager.template GetComponent<RespawnComponent>(id);
                            locComp.loc = respawn.loc;
                            collisionComp.collisionBox.topLeft.x = locComp.loc.x;
                            collisionComp.collisionBox.topLeft.y = locComp.loc.y;
                            movement.velocity = Point2D(0,0);
                            continue;
                        }

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
                                if (!isPlayer && entityManager.template HasComponent<MovementComponent>(id)) {
                                    auto& movement = entityManager.template GetComponent<MovementComponent>(id);
                                    movement.velocity.x = movement.velocity.x * -0.5;  // Bounce off the wall
                                }
                            } else {
                                locComp.loc.x += overlapX;
                                collisionComp.onLeftWall = true;
                                if (!isPlayer && entityManager.template HasComponent<MovementComponent>(id)) {
                                    auto& movement = entityManager.template GetComponent<MovementComponent>(id);
                                    movement.velocity.x = movement.velocity.x * -0.5;  // Bounce off the wall
                                }
                            }
                        } else {
                            if (wasAbove || (!wasBelow && collisionBox.topLeft.y < otherCollisionBox.topLeft.y)) {
                                locComp.loc.y -= overlapY;
                                collisionComp.onGround = true;
                            } else {
                                locComp.loc.y += overlapY;
                                collisionComp.onCeiling = true;
                                if (!isPlayer && entityManager.template HasComponent<MovementComponent>(id)) {
                                    auto& movement = entityManager.template GetComponent<MovementComponent>(id);
                                    movement.velocity.y = movement.velocity.y * -0.5;  // Bounce off the ceiling
                                }
                            }
                        }
                        collisionBox.topLeft.x = locComp.loc.x;
                        collisionBox.topLeft.y = locComp.loc.y;
                        //jitter was happening with the player
                    }
                }
                else { 
                    if (CheckCollision(collisionBox, otherCollisionBox)) {
                        if (isPlayer) {
                            score += 11;
                            entityManager.RemoveEntity(otherId);
                        }
                        else if (entityManager.template HasComponent<InputComponent>(otherId)) {
                            score += 11;
                            entityManager.RemoveEntity(id);
                            break;  // Exit the loop since the entity has been removed
                        }
                        else {
                            auto& movement = entityManager.template GetComponent<MovementComponent>(id);
                            auto& otherMovement = entityManager.template GetComponent<MovementComponent>(otherId);
                            auto& otherLocComp = entityManager.template GetComponent<LocationComponent>(otherId);

                            float overlapX = std::min(collisionBox.topLeft.x + collisionBox.width,
                                otherCollisionBox.topLeft.x + otherCollisionBox.width) 
                                - std::max(collisionBox.topLeft.x, otherCollisionBox.topLeft.x);

                            float overlapY = std::min(collisionBox.topLeft.y + collisionBox.height,
                                otherCollisionBox.topLeft.y + otherCollisionBox.height) 
                                - std::max(collisionBox.topLeft.y,otherCollisionBox.topLeft.y);

                            if (overlapX < overlapY) {
                                float separation = overlapX * 0.5f;

                                if (collisionBox.topLeft.x < otherCollisionBox.topLeft.x) {
                                    locComp.loc.x -= separation;
                                    otherLocComp.loc.x += separation;
                                }
                                else {
                                    locComp.loc.x += separation;
                                    otherLocComp.loc.x -= separation;
                                }

                                float velocityA = movement.velocity.x;
                                float velocityB = otherMovement.velocity.x;

                                if ((velocityA - velocityB) * (collisionBox.topLeft.x - otherCollisionBox.topLeft.x) < 0) {
                                    movement.velocity.x = velocityB * -0.9f;
                                    otherMovement.velocity.x = velocityA * -0.9f;
                                }
                            }
                            else {
                                float separation = overlapY * 0.5f;

                                if (collisionBox.topLeft.y < otherCollisionBox.topLeft.y) {
                                    locComp.loc.y -= separation;
                                    otherLocComp.loc.y += separation;
                                }
                                else {
                                    locComp.loc.y += separation;
                                    otherLocComp.loc.y -= separation;
                                }

                                float velocityA = movement.velocity.y;
                                float velocityB = otherMovement.velocity.y;

                                if ((velocityA - velocityB) *
                                    (collisionBox.topLeft.y - otherCollisionBox.topLeft.y) < 0) {
                                    movement.velocity.y = velocityB * -0.9f;
                                    otherMovement.velocity.y = velocityA * -0.9f;
                                }
                            }
                            collisionBox.topLeft.x = locComp.loc.x;
                            collisionBox.topLeft.y = locComp.loc.y;

                            otherCollisionBox.topLeft.x = otherLocComp.loc.x;
                            otherCollisionBox.topLeft.y = otherLocComp.loc.y;
                        }
                    }
                }
            }

            for (const auto& entity : entityManager) {
                EntityID id = entity.GetID();

                if (!entityManager.template HasComponent<ScoreComponent>(id)){
                    continue;
                }

                auto& scoreComp = entityManager.template GetComponent<ScoreComponent>(id);
                scoreComp.score += score;
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
