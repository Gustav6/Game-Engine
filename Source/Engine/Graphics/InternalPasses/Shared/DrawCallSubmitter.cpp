#include "Graphics/GraphicsEngine.pch.h"
#include "DrawCallSubmitter.h"

#include "Graphics/Core/RenderItem.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/Resources/BuiltinAssets.h"

#include "Graphics/Buffers/ObjectBuffer.h"
#include "Graphics/Buffers/AnimationBuffer.h"
#include "Graphics/Buffers/InstanceDataBuffer.h"

#include "Graphics/Buffers/Manager/BufferManager.h"

#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Materials/MaterialInterface.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/Drawers/MeshDrawer.h"

DrawCallSubmitter& DrawCallSubmitter::Get()
{
	static DrawCallSubmitter myInstance;
	return myInstance;
}

bool DrawCallSubmitter::Initialize(RenderHardwareInterface& aRHI)
{
	unsigned bufferSize = sizeof(InstanceData) * MAX_MESH_INSTANCES;
	unsigned bufferStride = sizeof(InstanceData);

	if (!aRHI.CreateStructuredBuffer("InstanceBuffer", bufferSize, bufferStride, *myInstanceBuffer))
	{
		SPDLOG_ERROR("Failed to create Instance Buffer!");
		return false;
	}

	return true;
}

DrawCallSubmitter::DrawCallSubmitter()
{
	myInstanceBuffer = std::make_unique<Buffer>();
}

void DrawCallSubmitter::RenderDrawCalls(
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	RenderContext& aContext,
	DrawCallFlags someFlags)
{
	thread_local std::vector<const Texture*> textureList;

	for (const auto& renderItem : aItems)
	{
		textureList.resize(Material::MAX_MATERIAL_TEXTURE_COUNT, nullptr);

		const MeshDrawCall& drawCall = *renderItem.DrawCall;
		const Mesh& mesh = *drawCall.Mesh;
		const Mesh::Element& element = mesh.myElements[renderItem.ElementIndex];
		const MaterialInterface* material = drawCall.Materials[renderItem.MaterialIndex];

		if (!PrepareMeshForRendering(mesh, *aContext.RHI))
		{
			continue;
		}

		if (material == nullptr)
		{
			material = aContext.BuiltinAssets->GetDefaultMaterial(MaterialType::Surface);
		}

		inoutCommandList.BeginEvent(mesh.GetName());

		inoutCommandList.SetVertexBuffer(&mesh.myVertexBuffer);
		inoutCommandList.SetIndexBuffer(&mesh.myIndexBuffer);

		ObjectBuffer objectBuffer;
		objectBuffer.World = drawCall.Transform;
		objectBuffer.WorldInvT = (drawCall.Transform.GetInverse()).GetTranspose();
		objectBuffer.HasSkinning = drawCall.HasSkinning;
		objectBuffer.IsInstanced = !drawCall.Instances.empty();
		objectBuffer.ObjectID = renderItem.ObjectID;

		aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader | PipelineStage_PixelShader);

		if (drawCall.HasSkinning)
		{
			AnimationBuffer animationBuffer;
			memcpy(animationBuffer.JointTransforms, drawCall.JointTransforms, 128 * sizeof(CommonUtilities::Matrix4x4f));
			aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::AnimationBuffer, animationBuffer, 2, PipelineStage_VertexShader);
		}

		if (HasDrawCallFlag(someFlags, DrawCallFlag_BindMaterial))
		{
			if (material->HasParameters())
			{
				if (material->IsMaterialDataDirty())
				{
					material->RefreshMaterialData();
				}

				aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			if (HasDrawCallFlag(someFlags, DrawCallFlag_BindTextures))
			{
				for (size_t t = 0; t < Material::MAX_MATERIAL_TEXTURE_COUNT; t++)
				{
					const std::shared_ptr<Texture>& texture = material->GetTexture(static_cast<unsigned int>(t));

					if (texture != nullptr)
					{
						textureList[t] = texture.get();
					}
				}

				inoutCommandList.SetShaderResources(textureList.data(), textureList.size(), 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			inoutCommandList.SetPipelineStateObject(material->GetPSO());
		}

		if (drawCall.Instances.size() > 0)
		{
			const unsigned int instanceCount = std::min(static_cast<unsigned int>(drawCall.Instances.size()), MAX_MESH_INSTANCES);

			if (instanceCount > MAX_MESH_INSTANCES)
			{
				SPDLOG_WARN("Too many instances submitted!");
				continue;
			}

			inoutCommandList.UpdateConstantBuffer(*myInstanceBuffer, drawCall.Instances.data(), instanceCount * sizeof(InstanceData));
			
			inoutCommandList.SetShaderResource(myInstanceBuffer.get(), 20, PipelineStage_VertexShader);

			inoutCommandList.DrawIndexedInstanced(element.NumIndices, element.IndexOffset, instanceCount);

			inoutCommandList.UnbindShaderResource(20, PipelineStage_VertexShader);
		}
		else
		{
			inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);
		}

		inoutCommandList.EndEvent();
	}
}

bool DrawCallSubmitter::PrepareMeshForRendering(const Mesh& aMesh, RenderHardwareInterface& aRHI)
{
	if (!aMesh.myVertexBuffer.IsValid())
	{
		if (!aRHI.CreateVertexBuffer(aMesh.myName, aMesh.myVertices, aMesh.myVertexBuffer))
		{
			SPDLOG_WARN("Could not prepare mesh {}! Vertex Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	if (!aMesh.myIndexBuffer.IsValid())
	{
		if (!aRHI.CreateIndexBuffer(aMesh.myName, aMesh.myIndices, aMesh.myIndexBuffer))
		{
			SPDLOG_WARN("Could not prepare mesh {}! Index Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	return true;
}

bool DrawCallSubmitter::HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag)
{
	return (aFlags & aFlag) != 0;
}
