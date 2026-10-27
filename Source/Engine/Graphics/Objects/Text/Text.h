#pragma once

#include <string_view>
#include <string>
#include <memory>
#include <vector>

#include "Graphics/Objects/GPU/Buffer.h"

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

struct Vertex;
class Font;

class Text
{
	friend class GraphicsEngine;
	friend class ScreenSpaceRenderPass;

public:
	Text();
	~Text();

	void SetText(std::string_view aText);
	void SetSize(float aPointSize);
	void SetColor(const CommonUtilities::Vector4f& aColor);
	void SetDepth(float aZOrder);

	CommonUtilities::Vector2f GetTextBounds(const Font& aFont) const;

	const std::string& GetText() const;

	bool RebuildGeometry(const Font& aFont);

private:
	std::vector<Vertex> myVertices;
	std::vector<unsigned int> myIndices;

	Buffer myVertexBuffer;
	Buffer myIndexBuffer;

	std::string myText;

	CommonUtilities::Vector4f myColor;

	float mySize;
	float myZOrder;

	bool myIsDirty;
};