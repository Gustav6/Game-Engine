#pragma once

#include <Tools/Core/Interface/Tool.h>

#include "DebugInfo.h"

// TODO: Display avreage frame time and FPS over a period of time also min max FPS.
// TODO: Add a graph to display frame time and FPS over time.
// TODO: Give more context to draw call info.

class DebugTool : public Tool
{
public:
	void SetInfo(const DebugInfo& aInfo);

	void Update(float aDeltaTime) override;
	void Render() override;

private:
	DebugInfo myInfo;
};

