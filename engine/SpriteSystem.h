#pragma once

#include <bitset>
#include <unordered_map>

#include "CameraComponent.h"
#include "EntityManager.h"
#include "MathUtil.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "LocationComponent.h"
#include "SpriteComponent.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>

namespace ECSEngine {

template <typename... Components>
class SpriteSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpriteSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override { 
        auto& entityManager = engine->GetEntityManager();
        auto& spriteManager = engine->GetSpriteManager();
        auto& windowManager = engine->GetWindowManager();
        auto window = windowManager.GetWindow();

        window->clear(sf::Color(40, 40, 90));

        for (const auto& entity : entityManager){
            EntityID id = entity.GetID();

            if(!entityManager.template HasComponent<SpriteComponent>(id) ||
               !entityManager.template HasComponent<LocationComponent>(id)){
                continue;
            }

            auto& spriteComp = entityManager.template GetComponent<SpriteComponent>(id);
            auto& location = entityManager.template GetComponent<LocationComponent>(id);

            Rect rect{{location.loc.x + spriteComp.bounds.topLeft.x, location.loc.y + spriteComp.bounds.topLeft.y},
                        spriteComp.bounds.width, spriteComp.bounds.height};
            
            if(spriteComp.worldSpace){
                rect = windowManager.WorldToWindow(rect);
            }

            sf::Sprite& sprite = spriteManager.GetSprite(spriteComp.textureID);
            sf::IntRect textRect = sprite.getTextureRect();
            sprite.setPosition({rect.topLeft.x, rect.topLeft.y});
            sprite.setScale({rect.width / textRect.size.x, rect.height / textRect.size.y});

            

            window->draw(sprite);

        }

        window->display();
        return kContinue;
     }
};

}  // namespace ECSEngine
