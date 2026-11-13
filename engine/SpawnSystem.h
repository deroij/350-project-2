#pragma once

#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& entityManager = engine->GetEntityManager();

        bool spawnFlag = false;

        EntityID spawnerID;

        for (const auto& entity : entityManager){
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<SpawnComponent>(id)){
                continue;
            }

            auto& spawnComp = entityManager.template GetComponent<SpawnComponent>(id);
            spawnerID = id;

            spawnComp.timeToNextSpawn--;

            if (spawnComp.timeToNextSpawn <= 0 && spawnComp.totalSpawns > 0){
                spawnFlag = true;

                spawnComp.timeToNextSpawn = spawnComp.timeBetweenSpawns;
                spawnComp.totalSpawns--;
            }
        }

        if (spawnFlag) { // Current system is hardcoded because we only have one spawner, need to add func to entity manager for non-hardcode
            auto& spawnComp = entityManager.template GetComponent<SpawnComponent>(spawnerID);
            auto& locComp = entityManager.template GetComponent<LocationComponent>(spawnerID);

            EntityID newEntityID = entityManager.CreateEntity(spawnComp.spawnType + std::to_string(spawnComp.totalSpawns));

            SpriteComponent sc{};
            sc.textureID = spawnComp.spawnTextureID;
            sc.bounds = Rect{Point2D(0,0),64,64};
            sc.worldSpace = true;
            entityManager.template AddComponent<SpriteComponent>(newEntityID, sc);

            LocationComponent lc{};
            lc.loc = Point2D{locComp.loc.x, locComp.loc.y};
            entityManager.template AddComponent<LocationComponent>(newEntityID, lc);

            CollisionComponent cc{};
            cc.staticEntity = false;
            cc.collisionBox = Rect{{0,0}, 64, 64};
            cc.previousCollisionBox = Rect{{0,0}, 64, 64};
            entityManager.template AddComponent<CollisionComponent>(newEntityID, cc);

            entityManager.template AddComponent<MovementComponent>(newEntityID, MovementComponent{Point2D{3, 1}});
            entityManager.template AddComponent<GravityComponent>(newEntityID, GravityComponent{Point2D{0, 0}});

            // This is where we add dynamic star behaviour
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
