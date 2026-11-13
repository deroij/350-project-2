#pragma once

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

        for (const auto& entity : entityManager){
            EntityID id = entity.GetID();

            if(!entityManager.template HasComponent<SpawnComponent>(id)){
                continue;
            }

            auto& spawnComp = entityManager.template GetComponent<SpawnComponent>(id);

            spawnComp.timeToNextSpawn--;
            if(spawnComp.timeToNextSpawn <= 0 && spawnComp.totalSpawns > 0){
                EntityID newEntityID = entityManager.CreateEntity(spawnComp.spawnType + std::to_string(spawnComp.totalSpawns));
                entityManager.template AddComponent<SpriteComponent>(newEntityID, SpriteComponent{spawnComp.spawnTextureID});
                entityManager.template AddComponent<LocationComponent>(newEntityID, 
                    entityManager.template GetComponent<LocationComponent>(id));
                entityManager.template AddComponent<MovementComponent>(newEntityID, MovementComponent{Point2D{0, 0}});
                entityManager.template AddComponent<GravityComponent>(newEntityID, GravityComponent{Point2D{0, 0}});
                entityManager.template AddComponent<CollisionComponent>(newEntityID, CollisionComponent{}); 

                // This is where we add dynamic star behaviour

                spawnComp.timeToNextSpawn = spawnComp.timeBetweenSpawns;
                spawnComp.totalSpawns--;
            }
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
