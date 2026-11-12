#pragma once

#include <cassert>
#include <iostream>
#include <limits>
#include <type_traits>

#include "Component.h"
#include "Entity.h"

namespace ECSEngine {

template <typename... Components>
class EntityManager {
public:
    static constexpr size_t kNullComponent = std::numeric_limits<size_t>::max();
    [[nodiscard]] EntityID CreateEntity(const std::string& name) {
        for (size_t i = 0; i < mFreeEntities.size(); ++i) {
            if (mFreeEntities[i]) {
                mFreeEntities[i] = false;
                mEntities[i] = Entity(i, name);
                mEntities[i].mComponents.resize(sizeof...(Components), kNullComponent);
                return i;
            }
        }
        EntityID id = mEntities.size();
        mEntities.push_back(Entity(id, name));
        mEntities.back().mComponents.resize(sizeof...(Components), kNullComponent);
        mFreeEntities.push_back(false);
        return id;
    }

    template <typename T>
    void RemoveComponentIfPresent(EntityID entity) {
        size_t componentIndex = GetComponentIndex<T>();
        size_t storageIndex = mEntities[entity].mComponents[componentIndex];

        if (storageIndex != kNullComponent) {
            auto& storage = GetComponentStorage<T>();
            storage.mValid[storageIndex] = false;
            storage.mFreeList.push_back(storageIndex);
            mEntities[entity].mComponents[componentIndex] = kNullComponent;
        }
    }

    void RemoveEntity(EntityID entity) {
        // Expands to cover all cases.
        (RemoveComponentIfPresent<Components>(entity), ...);

        mFreeEntities[entity] = true;
    }

    template <typename T>
    void AddComponent(EntityID entity, T component) {
        if (HasComponent<T>(entity)) {
            return;
        }
        
        size_t componentIndex = GetComponentIndex<T>();
        auto& storage = GetComponentStorage<T>();
        size_t storageIndex;

        if (!storage.mFreeList.empty()) {
            storageIndex = storage.mFreeList.back();
            storage.mFreeList.pop_back();
            storage.mStorage[storageIndex] = component;
            storage.mValid[storageIndex] = true;
        }
        else {
            storageIndex = storage.mStorage.size();
            storage.mStorage.push_back(component);
            storage.mValid.push_back(true);
        }

        mEntities[entity].mComponents[componentIndex] = storageIndex;
    }

    template <typename T>
    void RemoveComponent(EntityID entity) {
        RemoveComponentIfPresent<T>(entity);
    }

    template <typename T>
    bool HasComponent(EntityID entity) const {
        size_t index = GetComponentIndex<T>();
        size_t storageIndex = mEntities[entity].mComponents[index];
        return storageIndex != kNullComponent;
    }

    template <typename T>
    T& GetComponent(EntityID entity) {
        if (HasComponent<T>(entity)) {
            size_t index = GetComponentIndex<T>();
            size_t storageIndex = mEntities[entity].mComponents[index];
            return GetComponentStorage<T>().mStorage[storageIndex];
        }
    }

    template <typename T>
    ComponentStorage<T>& GetComponentStorage() {
        return std::get<ComponentStorage<T>>(mComponents);
    }

    using tEntity = std::vector<Entity>;
	using const_iterator = tEntity::const_iterator;
	const_iterator cbegin() const { return mEntities.begin(); }
	const_iterator cend() const { return mEntities.end(); }
	const_iterator begin() { return mEntities.begin(); }
	const_iterator end() { return mEntities.end(); }


private:
    std::tuple<ComponentStorage<Components>...> mComponents;
    std::vector<Entity> mEntities;
    std::vector<size_t> mFreeEntities;

    template <typename T>
    static constexpr size_t GetComponentIndex() {
        constexpr bool matches[] = {
            std::is_same_v<T, Components>...
        };

        for (size_t i = 0; i < sizeof...(Components); ++i) {
            if (matches[i]) {
                return i;
            }
        }

        return kNullComponent;
    }
};

}  // namespace ECSEngine
