#pragma once

#include <vector>

#include "CameraComponent.h"
#include "CollisionComponent.h"
#include "EntityManager.h"
#include "InputComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "ScoreComponent.h"
#include "SoundManager.h"
#include "SpawnComponent.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SystemManager.h"
#include "WindowManager.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

template <typename... Components>
class EngineView {
public:
    virtual SoundManager& GetSoundManager() = 0;
    virtual SpriteManager& GetSpriteManager() = 0;
    virtual EntityManager<Components...>& GetEntityManager() = 0;
    virtual WindowManager& GetWindowManager() = 0;
};

template <typename... Components>
class ECSEngine : public EngineView<Components...> {
public:
    ECSEngine(unsigned int width, unsigned int height, const std::string& name);
    ~ECSEngine();

    void Run();

    SoundManager& GetSoundManager() override { return mSoundManager; }

    SpriteManager& GetSpriteManager() override { return mSpriteManager; }

    EntityManager<Components...>& GetEntityManager() override { return mEntityManager; }

    WindowManager& GetWindowManager() override { return mWindowManager; }

    SystemManager<Components...>& GetSystemManager() { return mSystemManager; }

private:
    bool ProcessEvents();
    void DebugDraw();

    EntityManager<Components...> mEntityManager;
    SystemManager<Components...> mSystemManager;
    SpriteManager mSpriteManager;
    SoundManager mSoundManager;
    WindowManager mWindowManager;

    // For debugging - pause and step the engine
    bool running, step;
    int debug;
};

template <typename... Components>
ECSEngine<Components...>::ECSEngine(unsigned int width, unsigned int height,
                                    const std::string& name)
    : mWindowManager(width, height, name) {}

template <typename... Components>
ECSEngine<Components...>::~ECSEngine() {}

template <typename... Components>
void ECSEngine<Components...>::Run() {
    while (mWindowManager.GetWindow()->isOpen()) {
        ProcessEvents();
        mSystemManager.Run(this);
        //mWindowManager.GetWindow()->clear();
        //mWindowManager.GetWindow()->display();
        //the screen was flashing with these two in ECSEngine so I commented them out for now
    }
}

template <typename... Components>
bool ECSEngine<Components...>::ProcessEvents() {

    auto setKey = [this](int key, bool down){
        if(key < 0 || key >= static_cast<int>(sf::Keyboard::ScancodeCount)){
            return;
        }

        for(const auto& entity : mEntityManager){
            EntityID id = entity.GetID();
            if(mEntityManager.template HasComponent<InputComponent>(id)){
                mEntityManager.template GetComponent<InputComponent>(id).keydown[key] = down;
            }
        }
    };

    while (const std::optional event = mWindowManager.GetWindow()->pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            mWindowManager.GetWindow()->close();
            return true;
        }
        else if (event->is<sf::Event::Resized>())
        {
            return true;
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) // Just temp key press handler
        {
            setKey(static_cast<int>(keyPressed->scancode), true);
            return true;
        } else if(const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()){
            setKey(static_cast<int>(keyReleased->scancode), false);
            return true;
        }
    }
    return false;
}

template <typename... Components>
void ECSEngine<Components...>::DebugDraw() {}

}  // namespace ECSEngine
