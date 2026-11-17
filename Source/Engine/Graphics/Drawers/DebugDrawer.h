#pragma once

#include <vector>

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

#include <Graphics/Objects/GPU/InstanceData.h>

class Mesh;

struct WireMeshDrawCall
{
	const Mesh* Mesh = nullptr;
	CommonUtilities::Matrix4x4f Transform;
	CommonUtilities::Vector4f Color = { 1.f, 1.f, 1.f, 1.f };
};

struct OutlineDrawCall
{
	const Mesh* Mesh = nullptr;
	CommonUtilities::Matrix4x4f Transform;
	CommonUtilities::Vector4f Color = { 1.f, 1.f, 1.f, 1.f };
	const InstanceData* Instances;
	unsigned int AmountOfInstances = 0;
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

	void DrawWireframe(
		const CommonUtilities::Matrix4x4f& aTransform,
		const Mesh* aMesh = nullptr,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

    void DrawMeshOutline(
		const CommonUtilities::Matrix4x4f& aTransform,
		const Mesh* aMesh = nullptr,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f },
		const InstanceData* Instances = nullptr,
		unsigned int AmountOfInstances = 0);

    void DrawLine(
    	const CommonUtilities::Vector3f& aStart,
		const CommonUtilities::Vector3f& aEnd,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

	void DrawCross(
		float aExtent,
		const CommonUtilities::Vector3f& aPosition,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

	void DrawBox(
		const CommonUtilities::Vector3f& aMin,
		const CommonUtilities::Vector3f& aMax,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

	void DrawSphere(
		float aRadius,
		const CommonUtilities::Vector3f& aPosition,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

	void DrawCone(
		float aRadius,
		float aHeight,
		const CommonUtilities::Vector3f& aPosition,
		const CommonUtilities::Vector3f& aDirection,
		const CommonUtilities::Vector4f& aColor = { 1.f, 1.f, 1.f, 1.f });

    const std::vector<WireMeshDrawCall>& GetWireframeDrawCalls() const;
    const std::vector<LineDrawCall>& GetLineDrawCalls() const;
    const std::vector<OutlineDrawCall>& GetMeshOutlineDrawCalls() const;

    void BeginFrame();

private:
	std::vector<WireMeshDrawCall> myWireDrawCalls;
	std::vector<LineDrawCall> myLineDrawCalls;
	std::vector<OutlineDrawCall> myOutlineDrawCalls;
};