#pragma once

class Tool
{
public:
	virtual ~Tool() = default;

	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;

	//virtual void Disable() = 0;
	//virtual void Enable() = 0;

protected:
	Tool() = default;
};

