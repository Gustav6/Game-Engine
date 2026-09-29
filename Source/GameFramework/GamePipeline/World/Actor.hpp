#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "GamePipeline/Components/Transform/Transform.h"
#include "GamePipeline/Components/Base/Component.h"
#include "GamePipeline/Components/Registry/ComponentRegistryManager.h"

class Actor
{
public:
	Actor();
	virtual ~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	/*
	 * @return A pointer to the component. The pointer is only valid for the relevant scope and must not be stored for later use.
	 */
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		return ComponentRegistryManager::Get().AddComponent<T>(this, std::forward<Args>(args)...);
	}

	template<typename T>
	void RemoveComponent()
	{
		ComponentRegistryManager::Get().RemoveComponent<T>(this);
	}

	/*
	 * @return A pointer to the component. The pointer is only valid for the relevant scope and must not be stored for later use.
	 */
	template<typename T>
	T* GetComponent()
	{
		return ComponentRegistryManager::Get().GetComponent<T>(this);
	}
	
	virtual void Update(float aDeltaTime);

	void Destroy();
	bool GetIsDestroyed() const;

	Transform& GetTransform();
	const Transform& GetTransform() const;

	void Enable(bool aState);
	bool GetIsEnabled() const;

private:
	Transform myTransform;

	bool myIsDestroyed;
	bool myIsEnabled;
};