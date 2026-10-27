#include "GraphicsEngine.pch.h"
#include "DrawCallSubmitter.h"

#include "Graphics/Core/RenderItem.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/Resources/BuiltinAssets.h"

#include "Graphics/ConstantBuffers/ObjectBuffer.h"
#include "Graphics/ConstantBuffers/AnimationBuffer.h"

#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Materials/MaterialInterface.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/Drawers/MeshDrawer.h"

void DrawCallSubmitter::RenderDrawCalls(
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	RenderContext& aContext,
	DrawCallFlags someFlags)
{
	thread_local std::array<const Texture*, Material::MAX_MATERIAL_TEXTURE_COUNT> textureList;

	for (const auto& renderItem : aItems)
	{
		textureList.fill(nullptr);

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
			material = aContext.BuiltinAssets->GetDefaultMaterial();
		}

		inoutCommandList.BeginEvent(mesh.GetName());

		inoutCommandList.SetVertexBuffer(&mesh.myVertexBuffer);
		inoutCommandList.SetIndexBuffer(&mesh.myIndexBuffer);

		ObjectBuffer objectBuffer;
		objectBuffer.World = drawCall.Transform;
		objectBuffer.WorldInvT = (drawCall.Transform.GetInverse()).GetTranspose();
		objectBuffer.HasSkinning = drawCall.HasSkinning;

		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

		if (drawCall.HasSkinning)
		{
			AnimationBuffer animationBuffer;
			memcpy(animationBuffer.JointTransforms, drawCall.JointTransforms, 128 * sizeof(CommonUtilities::Matrix4x4f));
			aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::AnimationBuffer, animationBuffer, 2, PipelineStage_VertexShader);
		}

		if (HasDrawCallFlag(someFlags, DrawCallFlag_BindMaterial))
		{
			if (material->HasParameters())
			{
				if (material->IsMaterialDataDirty())
				{
					material->RefreshMaterialData();
				}

				aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
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

		inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);

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
