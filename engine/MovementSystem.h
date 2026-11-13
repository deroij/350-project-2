#pragma once

#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class MovementSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("MovementSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entityManager = engine->GetEntityManager();

        for (const auto& entity : entityManager) {
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<LocationComponent>(id) ||
                !entityManager.template HasComponent<MovementComponent>(id)) {
                continue;
            }

            auto& locComp = entityManager.template GetComponent<LocationComponent>(id);
            auto& moveComp = entityManager.template GetComponent<MovementComponent>(id);

            locComp.loc.x += moveComp.velocity.x;
            locComp.loc.y += moveComp.velocity.y;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
