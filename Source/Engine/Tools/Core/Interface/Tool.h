#pragma once

class Tool
{
public:
	virtual ~Tool() = default;

	virtual void Update(float aDeltaTime) = 0;
	virtual void Render() = 0;
};

