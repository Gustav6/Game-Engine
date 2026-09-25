#include "Text.h"

#include "Font.h"

#include "Materials/Material.h"

#include "GraphicsEngine/Objects/GPU/Vertex.h"

Text::Text()
{
	myIsDirty = true;
	mySize = 12.0f;
	myColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

Text::~Text() = default;

void Text::SetText(std::string_view aText)
{
	myText = aText;
	myIsDirty = true;
}

void Text::SetSize(float aPointSize)
{
	mySize = aPointSize;
	myIsDirty = true;
}

void Text::SetColor(const CommonUtilities::Vector4f& aColor)
{
	myColor = aColor;
	myIsDirty = true;
}

void Text::SetDepth(float aZOrder)
{
	myZOrder = aZOrder;
}

bool Text::RebuildGeometry(const Font& aFont)
{
	myVertices.clear();
	myIndices.clear();

	if (myText.empty())
	{
		return true;
	}

	myVertices.reserve(myText.size() * 4 + 4);
	myIndices.reserve(myText.size() * 4 + 4);

	const CommonUtilities::Vector4f lineOffset = CommonUtilities::Vector4f(0, -aFont.GetLineHeight(), 0, 0);

	CommonUtilities::Vector4f cursor = CommonUtilities::Vector4f::Zero;

	CommonUtilities::Vector4f color = myColor;

	unsigned int row = 0;

	for (size_t p = 0; p < myText.size(); p++)
	{
		const char c = myText[p];

		switch (c)
		{
			case '\n':
			{
				row++;
				cursor = CommonUtilities::Vector4f::Zero;
				cursor.y = lineOffset.y * row;
				continue;
			}
			case ' ':
			{
				if (const Font::Glyph* glyph = aFont.GetGlyph(c))
				{
					CommonUtilities::Vector4f advance = CommonUtilities::Vector4f(glyph->Advance, 0, 0, 0);
					cursor += advance;
				}
				continue;
			}
		}

		if (const Font::Glyph* glyph = aFont.GetGlyph(c))
		{
			const unsigned int currentVertexCount = static_cast<unsigned int>(myVertices.size());

			Vertex v1, v2, v3, v4;

			v1.Position = CommonUtilities::Vector4f(glyph->PlaneBounds.Left, glyph->PlaneBounds.Top, 0, 1) + cursor + lineOffset;
			v1.UV0 = { glyph->AtlasBounds.Left, 1 - glyph->AtlasBounds.Top };
			v1.Color = color;

			v2.Position = CommonUtilities::Vector4f(glyph->PlaneBounds.Right, glyph->PlaneBounds.Top, 0, 1) + cursor + lineOffset;
			v2.UV0 = { glyph->AtlasBounds.Right, 1 - glyph->AtlasBounds.Top };
			v2.Color = color;

			v3.Position = CommonUtilities::Vector4f(glyph->PlaneBounds.Right, glyph->PlaneBounds.Bottom, 0, 1) + cursor + lineOffset;
			v3.UV0 = { glyph->AtlasBounds.Right, 1 - glyph->AtlasBounds.Bottom };
			v3.Color = color;

			v4.Position = CommonUtilities::Vector4f(glyph->PlaneBounds.Left, glyph->PlaneBounds.Bottom, 0, 1) + cursor + lineOffset;
			v4.UV0 = { glyph->AtlasBounds.Left, 1 - glyph->AtlasBounds.Bottom };
			v4.Color = color;

			// Text bounds check

			myVertices.emplace_back(v1);
			myVertices.emplace_back(v2);
			myVertices.emplace_back(v3);
			myVertices.emplace_back(v4);

			myIndices.emplace_back(currentVertexCount + 0);
			myIndices.emplace_back(currentVertexCount + 1);
			myIndices.emplace_back(currentVertexCount + 2);
			myIndices.emplace_back(currentVertexCount + 0);
			myIndices.emplace_back(currentVertexCount + 2);
			myIndices.emplace_back(currentVertexCount + 3);

			const CommonUtilities::Vector4f advance = CommonUtilities::Vector4f(glyph->Advance, 0, 0, 0);
			cursor += advance;
		}
	}

	return true;
}