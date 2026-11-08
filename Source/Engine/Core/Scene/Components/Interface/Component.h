#pragma once

class Actor;

class Component
{
public:
	Component(Actor* aOwner);
	virtual ~Component() = default;

	virtual void Update([[maybe_unused]] float aDeltaTime) {};
	virtual void Render() {};

	Actor* GetOwner() const;
	
	void Enable();
	void Disable();
	bool GetIsEnabled() const;

protected:
	Actor* myOwner;

	bool myIsEnabled;
};