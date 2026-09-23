#include "GraphicsEngine.pch.h"
#include "DrawCallSubmitter.h"

#include "RenderItem.h"
#include "RenderContext.h"

#include "Assets/BuiltinAssets.h"

#include "GraphicsEngine/ConstantBuffers/ObjectBuffer.h"
#include "GraphicsEngine/ConstantBuffers/AnimationBuffer.h"

#include "GraphicsEngine/Objects/Mesh.h"

#include "GraphicsEngine/Materials/MaterialInterface.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"
#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/Drawers/MeshDrawer.h"

void DrawCallSubmitter::RenderDrawCalls(
	GraphicsEngine& aGraphicsEngine,
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	RenderContext& aContext,
	DrawCallFlags someFlags)
{
	for (const auto& renderItem : aItems)
	{
		if (renderItem.DrawCall == nullptr)
		{
			GELOG(Error, "Tried to render a draw call with a null mesh!");
			continue;
		}

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
			material = &aContext.BuiltinAssets->GetDefaultMaterial();
		}

		inoutCommandList.BeginEvent(mesh.GetName());

		inoutCommandList.SetVertexBuffer(&mesh.myVertexBuffer);
		inoutCommandList.SetIndexBuffer(&mesh.myIndexBuffer);

		ObjectBuffer objectBuffer;
		objectBuffer.World = drawCall.Transform;
		objectBuffer.WorldInvT = (drawCall.Transform.GetInverse()).GetTranspose();
		objectBuffer.HasSkinning = drawCall.HasSkinning;

		aGraphicsEngine.UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

		if (drawCall.HasSkinning)
		{
			AnimationBuffer animationBuffer;
			memcpy(animationBuffer.JointTransforms, drawCall.JointTransforms, 128 * sizeof(CommonUtilities::Matrix4x4f));
			aGraphicsEngine.UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::AnimationBuffer, animationBuffer, 2, PipelineStage_VertexShader);
		}

		if (HasDrawCallFlag(someFlags, DrawCallFlag_BindMaterial))
		{
			if (material->HasParameters())
			{
				if (material->IsMaterialDataDirty())
				{
					material->RefreshMaterialData();
				}

				aGraphicsEngine.UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			if (HasDrawCallFlag(someFlags, DrawCallFlag_BindTextures))
			{
				std::vector<const Texture*> textures(Material::MAX_MATERIAL_TEXTURE_COUNT);

				for (size_t t = 0; t < Material::MAX_MATERIAL_TEXTURE_COUNT; t++)
				{
					const std::shared_ptr<Texture>& texture = material->GetTexture(static_cast<unsigned int>(t));

					if (texture != nullptr)
					{
						textures[t] = texture.get();
					}
				}

				inoutCommandList.SetShaderResources(textures.data(), textures.size(), 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			inoutCommandList.SetPipelineStateObject(material->GetPSO());
		}

		inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);

		// Ends event for the draw call!
		inoutCommandList.EndEvent();
	}
}

bool DrawCallSubmitter::PrepareMeshForRendering(const Mesh& aMesh, RenderHardwareInterface& aRHI)
{
	if (!aMesh.myVertexBuffer.IsValid())
	{
		if (!aRHI.CreateVertexBuffer(aMesh.myName, aMesh.myVertices, aMesh.myVertexBuffer))
		{
			GELOG(Warning, "Could not prepare mesh {}! Vertex Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	if (!aMesh.myIndexBuffer.IsValid())
	{
		if (!aRHI.CreateIndexBuffer(aMesh.myName, aMesh.myIndices, aMesh.myIndexBuffer))
		{
			GELOG(Warning, "Could not prepare mesh {}! Index Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	return true;
}

bool DrawCallSubmitter::HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag)
{
	return (aFlags & aFlag) != 0;
}
