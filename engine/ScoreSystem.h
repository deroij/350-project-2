#pragma once

namespace ECSEngine {

template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto& entityManager = engine->GetEntityManager();

        for (const auto& entity : entityManager) {
            EntityID id = entity.GetID();

            if (!entityManager.template HasComponent<ScoreComponent>(id)) {
                continue;
            }

            auto& scoreComp = entityManager.template GetComponent<ScoreComponent>(id);

            // Update the digit textures based on the current score
            int score = scoreComp.score;
            for (int i = 0; i < 3; ++i) {
                int digit = score % 10;
                auto& spriteComp = entityManager.template GetComponent<SpriteComponent>(scoreComp.digitID[i]);

                spriteComp.textureID = scoreComp.digitTextureID[digit];
                score /= 10;
            }
        }
        
        return kContinue; 
    }
};

}  // namespace ECSEngine
