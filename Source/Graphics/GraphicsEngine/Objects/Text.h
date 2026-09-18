#pragma once

#include <string_view>
#include <memory>

#include "Buffer.h"
#include "Font.h"
#include "Materials/Material.h"

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector4.hpp"
#include "CommonUtilities/Math/Matrix4x4.hpp"

struct Vertex;

class Text
{
	friend class GraphicsEngine;

public:
	Text(std::string_view aName);
	~Text();

	void SetText(std::string_view aText);
	void SetFont(const std::shared_ptr<Font> aFont);
	void SetFontMaterial(std::shared_ptr<Material> aMaterial);
	void SetSize(float aPointSize);
	void SetZOrder(float aOrder);
	void SetColor(const CommonUtilities::Vector4f& aColor);

	void SetPosition(const CommonUtilities::Vector2f& aPosition);

	const CommonUtilities::Matrix4x4f& GetRenderMatrix() const;

private:
	bool RebuildGeometry();
	void RebuildTextRenderMatrix();

	CommonUtilities::Vector2f myPosition;

	float myZOrder;
	float mySize;

	CommonUtilities::Vector4f myColor;

	CommonUtilities::Matrix4x4f myTextRenderMatrix;

	std::shared_ptr<Font> myFont;
	std::shared_ptr<Material> myFontMaterial;

	std::vector<Vertex> myVertices;
	std::vector<unsigned int> myIndices;
	mutable Buffer myVertexBuffer;
	mutable Buffer myIndexBuffer;

	bool myIsDirty;

	std::string myName;

	std::string myTextString;
};