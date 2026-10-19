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

void DebugDrawer::DrawWireframe(
	const CommonUtilities::Matrix4x4f& aTransform,
	const Mesh* aMesh,
	const CommonUtilities::Vector4f& aColor)
{
	if (aMesh == nullptr)
	{
		SPDLOG_ERROR("Mesh was submitted with invalid data!");
		return;
	}

	myWireDrawCalls.emplace_back(aMesh, aTransform, aColor);
}
void DebugDrawer::DrawLine(
	const CommonUtilities::Vector3f& aStart,
	const CommonUtilities::Vector3f& aEnd,
	const CommonUtilities::Vector4f& aColor)
{
	if (aStart == aEnd)
	{
		return;
	}

	myLineDrawCalls.emplace_back(aStart, aEnd, aColor);
}

void DebugDrawer::DrawCross(
	float aExtent, 
	const CommonUtilities::Vector3f& aPosition,
	const CommonUtilities::Vector4f& aColor)
{
	if (aExtent <= 0.0f)
	{
		return;
	}

	const CommonUtilities::Vector3f x = { aExtent, 0.0f, 0.0f };
	const CommonUtilities::Vector3f y = { 0.0f, aExtent, 0.0f };
	const CommonUtilities::Vector3f z = { 0.0f, 0.0f, aExtent };

	myLineDrawCalls.emplace_back(aPosition - x, aPosition + x, aColor);
	myLineDrawCalls.emplace_back(aPosition - y, aPosition + y, aColor);
	myLineDrawCalls.emplace_back(aPosition - z, aPosition + z, aColor);
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

void DebugDrawer::DrawCone(
	float aRadius, 
	float aHeight, 
	const CommonUtilities::Vector3f& aPosition,
	const CommonUtilities::Vector3f& aDirection, 
	const CommonUtilities::Vector4f& aColor)
{
	constexpr int amountOfSegments = 24;
	constexpr float twoPi = 2.0f * CommonUtilities::Math::PI<float>;
	constexpr int spokeEvery = 3;

	if (aRadius <= 0.0f || aHeight <= 0.0f)
	{
		return;
	}

	if (aDirection.LengthSqr() < 0.000001f)
	{
		return;
	}

	const CommonUtilities::Vector3f forward = aDirection.GetNormalized();

	CommonUtilities::Vector3f temp = { 0.0f, 1.0f, 0.0f };

	if (std::abs(CommonUtilities::Vector3f::Dot(forward, temp)) > 0.99f)
	{
		temp = { 1.0f, 0.0f, 0.0f };
	}

	const CommonUtilities::Vector3f right = CommonUtilities::Vector3f::Cross(temp, forward).GetNormalized();
	const CommonUtilities::Vector3f up = CommonUtilities::Vector3f::Cross(forward, right);

	const CommonUtilities::Vector3f apex = aPosition;
	const CommonUtilities::Vector3f baseCenter = aPosition + forward * aHeight;

	CommonUtilities::Vector3f previous = baseCenter + right * aRadius;

	for (int i = 1; i <= amountOfSegments; ++i)
	{
		const float angle = twoPi * static_cast<float>(i) / static_cast<float>(amountOfSegments);

		const CommonUtilities::Vector3f current = baseCenter + right * (std::cos(angle) * aRadius) + up * (std::sin(angle) * aRadius);

		myLineDrawCalls.emplace_back(previous, current, aColor);

		if (i % spokeEvery == 0)
		{
			myLineDrawCalls.emplace_back(apex, current, aColor);
		}

		previous = current;
	}
}

const std::vector<WireMeshDrawCall>& DebugDrawer::GetWireframeDrawCalls() const
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
