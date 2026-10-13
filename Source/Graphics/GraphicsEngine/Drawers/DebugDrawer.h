#pragma once

#include <vector>

#include <CommonUtilities/Math/Matrix4x4.hpp>

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

class Mesh;

struct WireMeshDrawCall
{
    const Mesh* Mesh = nullptr;
    CommonUtilities::Matrix4x4f Transform;
    CommonUtilities::Vector4f Color = { 1.f, 1.f, 1.f, 1.f };
}; 

struct LineDrawCall
{
    CommonUtilities::Vector3f Start;
    CommonUtilities::Vector3f End;
    CommonUtilities::Vector4f Color = { 1.f, 1.f, 1.f, 1.f };
};

class DebugDrawer
{
public:
    static DebugDrawer& Get();

    void Submit(const WireMeshDrawCall& aDrawCall);
    void Submit(const LineDrawCall& aDrawCall);

	void DrawBox(
		const CommonUtilities::Vector3f& aMin,
		const CommonUtilities::Vector3f& aMax,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

	void DrawSphere(
		float aRadius,
		const CommonUtilities::Vector3f& aPosition,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

    const std::vector<WireMeshDrawCall>& GetMeshDrawCalls() const;
    const std::vector<LineDrawCall>& GetLineDrawCalls() const;

    void Clear();

private:
	std::vector<WireMeshDrawCall> myWireDrawCalls;
	std::vector<LineDrawCall> myLineDrawCalls;
};