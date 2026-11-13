#pragma once

#include "EntityManager.h"
#include "LocationComponent.h"
#include "SystemManager.h"
#include "CollisionComponent.h"
#include "InputComponent.h"
#include "Keyboard.h"

namespace ECSEngine
{

const float wallSlideSpeed = 3;

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

			bool hasCollision = entityManager.template HasComponent<CollisionComponent>(id);

			if(hasCollision){
				auto& collision = entityManager.template GetComponent<CollisionComponent>(id);
				if(collision.onGround && movement.velocity.y > 0){
					movement.velocity.y = 0;
				}
				if(collision.onCeiling && movement.velocity.y < 0){
					movement.velocity.y = 0;
				}
			}

			movement.velocity.x += gravity.acceleration.x;
			movement.velocity.y += gravity.acceleration.y;


			if(hasCollision && entityManager.template HasComponent<InputComponent>(id)){
				auto& collision = entityManager.template GetComponent<CollisionComponent>(id);
				auto& input = entityManager.template GetComponent<InputComponent>(id);

				bool pushingIntoLeftWall = collision.onLeftWall && input.keydown[Key::Left];
				bool pushingIntoRightWall = collision.onRightWall && input.keydown[Key::Right];

				if(!collision.onGround && movement.velocity.y > 0 &&(pushingIntoLeftWall || pushingIntoRightWall)){
					if(movement.velocity.y > wallSlideSpeed){
						movement.velocity.y = wallSlideSpeed;
					}
				}
			}
		}
		return kContinue;
	}
	
};


}



