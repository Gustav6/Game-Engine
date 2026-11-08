#pragma once

#include <Tools/Core/Interface/Tool.h>

#include "DebugInfo.h"

class DebugTool : public Tool
{
public:
	void SetInfo(const DebugInfo& aInfo);

	void Update(float aDeltaTime) override;
	void Render() override;

private:
	DebugInfo myInfo;
};

