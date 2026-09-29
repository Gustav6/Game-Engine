#include "GraphicsEngine.pch.h"
#include "GBuffer.h"

GBuffer::GBuffer() = default;
GBuffer::~GBuffer() = default;

const Texture* GBuffer::GetTextures() const
{
	return myTextures.data();
}
