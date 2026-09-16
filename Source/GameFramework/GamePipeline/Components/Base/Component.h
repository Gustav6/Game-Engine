#pragma once

class Actor;

class Component
{
public:
	Component(Actor* aOwner);

	virtual void Update(float aDeltaTime) {};
	virtual void Render() {};

protected:
	Actor* myOwner;
};