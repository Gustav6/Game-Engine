#include "GraphicsEngine.pch.h"
#include "MeshDrawer.h"

MeshDrawer& MeshDrawer::Get()
{
	static MeshDrawer myInstance;
	return myInstance;
}

void MeshDrawer::Submit(const MeshDrawCall& aDrawCall)
{
	if (aDrawCall.Mesh == nullptr)
	{
		GELOG(Error, "Mesh was submitted with invalid data!");
		return;
	}

	myDrawCalls.emplace_back(aDrawCall);
}

const std::vector<MeshDrawCall>& MeshDrawer::GetMeshDrawCalls() const
{
	return myDrawCalls;
}

void MeshDrawer::Clear()
{
	myDrawCalls.clear();
}