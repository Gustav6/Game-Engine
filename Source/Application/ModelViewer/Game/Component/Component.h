#pragma once

class GameObject;

class Component
{
public:
	Component(GameObject* aOwner);

	virtual void Update(float aDeltaTime) {};
	virtual void Render() {};

protected:
	GameObject* myOwner;
};