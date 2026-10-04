#include "GraphicsEngine.pch.h"
#include "DebugDrawer.h"

DebugDrawer& DebugDrawer::Get()
{
	static DebugDrawer myInstance;
	return myInstance;
}

void DebugDrawer::Submit(const DebugDrawCall& aDrawCall)
{
	if (aDrawCall.Mesh == nullptr)
	{
		GELOG(Error, "Mesh was submitted with invalid data!");
		return;
	}

	myDebugDrawCalls.emplace_back(aDrawCall);
}
