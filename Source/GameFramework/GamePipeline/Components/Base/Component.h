#pragma once

class Actor;

class Component
{
public:
	virtual ~Component() = default;
	Component(Actor* aOwner);

	virtual void Update([[maybe_unused]] float aDeltaTime) {};
	virtual void Render() {};

	Actor* GetOwner() const;

protected:
	Actor* myOwner;
};