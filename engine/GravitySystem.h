#pragma once

#include "EntityManager.h"
#include "LocationComponent.h"
#include "SystemManager.h"

namespace ECSEngine
{

template <typename... Components>
class GravitySystem : public System<Components...> {
public:
	const std::string & GetName() const override
	{ static std::string name("GravitySystem"); return name; }
	
	tSystemReturn Run(EngineView<Components...> *engine) override
	{
		auto& entityManager = engine->GetEntityManager();

		for(const auto& entity : entityManager){
			EntityID id = entity.GetID();

			if (!entityManager.template HasComponent<GravityComponent>(id) ||
				!entityManager.template HasComponent<MovementComponent>(id)){
				continue;
			}

			auto& gravity = entityManager.template GetComponent<GravityComponent>(id);
			auto& movement = entityManager.template GetComponent<MovementComponent>(id);

			if(gravity.disabledTimer > 0){
				--gravity.disabledTimer;
				continue;
			}

			movement.velocity.x += gravity.acceleration.x;
			movement.velocity.y += gravity.aceleration.y;
		}
		return kContinue;
	}
	
};


}



