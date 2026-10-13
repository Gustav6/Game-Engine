#include "GraphicsEngine.pch.h"
#include "DebugDrawer.h"

#include <array>

#include "CommonUtilities/Math/MathUtils.hpp"

#include <tracy/core/Tracy.hpp>

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
	const CommonUtilities::Vector3f corner1 = { aMax.x, aMin.y, aMin.z };
	const CommonUtilities::Vector3f corner2 = { aMax.x, aMax.y, aMin.z };
	const CommonUtilities::Vector3f corner3 = { aMin.x, aMax.y, aMin.z };
	const CommonUtilities::Vector3f corner4 = { aMin.x, aMin.y, aMax.z };
	const CommonUtilities::Vector3f corner5 = { aMax.x, aMin.y, aMax.z };
	const CommonUtilities::Vector3f corner6 = { aMin.x, aMax.y, aMax.z };

	myLineDrawCalls.reserve(myLineDrawCalls.size() + 12);

	myLineDrawCalls.emplace_back(aMin, corner1, aColor);
	myLineDrawCalls.emplace_back(corner1, corner2, aColor);
	myLineDrawCalls.emplace_back(corner2, corner3, aColor);
	myLineDrawCalls.emplace_back(corner3, aMin, aColor);

	myLineDrawCalls.emplace_back(corner4, corner5, aColor);
	myLineDrawCalls.emplace_back(corner5, aMax, aColor);
	myLineDrawCalls.emplace_back(aMax, corner6, aColor);
	myLineDrawCalls.emplace_back(corner6, corner4, aColor);

	myLineDrawCalls.emplace_back(aMin, corner4, aColor);
	myLineDrawCalls.emplace_back(corner1, corner5, aColor);
	myLineDrawCalls.emplace_back(corner2, aMax, aColor);
	myLineDrawCalls.emplace_back(corner3, corner6, aColor);
}

void DebugDrawer::DrawSphere(
	float aRadius, 
	const CommonUtilities::Vector3f& aPosition, 
	const CommonUtilities::Vector4f& aColor)
{
	constexpr int amountOfSegments = 24;
	constexpr float twoPi = CommonUtilities::Math::PI<float> * 2.0f;

	struct Segment
	{
		CommonUtilities::Vector3f From;
		CommonUtilities::Vector3f To;
	};

	static const std::array<Segment, amountOfSegments * 3> unitSphere = []
    {
        std::array<Segment, amountOfSegments * 3> segments = {};

        for (int i = 0; i < amountOfSegments; ++i)
        {
            const float a0 = twoPi * static_cast<float>(i) / amountOfSegments;
            const float a1 = twoPi * static_cast<float>(i + 1) / amountOfSegments;

            const float c0 = std::cos(a0), s0 = std::sin(a0);
            const float c1 = std::cos(a1), s1 = std::sin(a1);

            segments[i * 3 + 0] = { { c0, s0, 0.f }, { c1, s1, 0.f } };
            segments[i * 3 + 1] = { { c0, 0.f, s0 }, { c1, 0.f, s1 } };
            segments[i * 3 + 2] = { { 0.f, c0, s0 }, { 0.f, c1, s1 } };
        }

        return segments;
    }();

	myLineDrawCalls.reserve(myLineDrawCalls.size() + unitSphere.size());

	for (const Segment& segment : unitSphere) 
	{
		CommonUtilities::Vector3f from = aPosition + segment.From * aRadius;
		CommonUtilities::Vector3f to = aPosition + segment.To * aRadius;

		myLineDrawCalls.emplace_back(from, to, aColor);
	}
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
