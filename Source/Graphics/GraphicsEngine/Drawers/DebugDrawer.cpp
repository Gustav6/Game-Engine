#include "GraphicsEngine.pch.h"
#include "DebugDrawer.h"

#include <array>

DebugDrawer& DebugDrawer::Get()
{
	static DebugDrawer myInstance;
	return myInstance;
}

void DebugDrawer::Submit(const WireMeshDrawCall& aDrawCall)
{
	if (aDrawCall.Mesh == nullptr)
	{
		GELOG(Error, "Mesh was submitted with invalid data!");
		return;
	}

	myWireDrawCalls.emplace_back(aDrawCall);
}
void DebugDrawer::Submit(const LineDrawCall& aDrawCall)
{
	if (aDrawCall.Start == aDrawCall.End)
	{
		return;
	}

	myLineDrawCalls.emplace_back(aDrawCall);
}

void DebugDrawer::DrawBox( 
	const CommonUtilities::Vector3f& aMin, 
	const CommonUtilities::Vector3f& aMax, 
	const CommonUtilities::Vector4f& aColor)
{
    std::array<CommonUtilities::Vector3f, 8> corners;

    for (int i = 0; i < corners.size(); i++)
	{
		CommonUtilities::Vector4f position;

		if (i & 1)
		{
			position.x = aMax.x;
		}
		else
		{
			position.x = aMin.x;
		}

		if (i & 2)
		{
			position.y = aMax.y;
		}
		else
		{
			position.y = aMin.y;
		}

		if (i & 4)
		{
			position.z = aMax.z;
		}
		else
		{
			position.z = aMin.z;
		}

        corners[i] = { position.x, position.y, position.z };
    }

    static constexpr int edges[12][2] =
    {
        { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 },
        { 0, 2 }, { 1, 3 }, { 4, 6 }, { 5, 7 },
        { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } 
    };

    for (const auto& edge : edges)
    {
        Submit({ corners[edge[0]], corners[edge[1]], aColor });
    }
}

void DebugDrawer::DrawSphere(float aRadius, const CommonUtilities::Vector3f& aPosition, const CommonUtilities::Vector4f& aColor)
{
	aRadius;
	aPosition;
	aColor;
}

const std::vector<WireMeshDrawCall>& DebugDrawer::GetMeshDrawCalls() const
{
	return myWireDrawCalls;
}

const std::vector<LineDrawCall>& DebugDrawer::GetLineDrawCalls() const
{
	return myLineDrawCalls;
}

void DebugDrawer::Clear()
{
	myWireDrawCalls.clear();
	myLineDrawCalls.clear();
}
