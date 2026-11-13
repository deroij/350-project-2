#pragma once

#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class CollisionLateSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("CollisionLateSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entityManager = engine->GetEntityManager();

        for (const auto& entity : entityManager){
            EntityID id = entity.GetID();

            if(!entityManager.template HasComponent<CollisionComponent>(id)) {
                continue;
            }

            auto &collisionComp = entityManager.template GetComponent<CollisionComponent>(id);

            if (collisionComp.staticEntity) {
                continue;
            }

            collisionComp.previousCollisionBox = collisionComp.collisionBox;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
