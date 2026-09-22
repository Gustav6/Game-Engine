#include "MeshDrawer.h"

MeshDrawer& MeshDrawer::Get()
{
	static MeshDrawer myInstance;
	return myInstance;
}

void MeshDrawer::Submit(const MeshDrawCall& aDrawCall)
{
	myDrawCalls.emplace_back(aDrawCall);
}