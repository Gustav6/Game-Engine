#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Game/Component/Transform.h"

#include "Game/Component/Component.h"

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		auto& rawPointer = myComponentLookup[typeid(T)] = std::make_unique<T>(this, std::forward<Args>(args)...);
		return static_cast<T*>(rawPointer.get());
	}

	template<typename T>
	void RemoveComponent()
	{
		myComponentLookup.erase(typeid(T));
	}

	template<typename T>
	T* GetComponent()
	{
		auto iterator = myComponentLookup.find(typeid(T));

		if (iterator == myComponentLookup.end())
		{
			return nullptr;
		}

		return static_cast<T*>(iterator->second.get());
	}

	virtual void Update(float aDeltaTime);

	Transform& GetTransform();

	void SetEnabled(bool aState);
	bool GetIsEnabled() const;

private:
	bool myIsEnabled;

	Transform myTransform;

	std::unordered_map<std::type_index, std::unique_ptr<Component>> myComponentLookup;
};