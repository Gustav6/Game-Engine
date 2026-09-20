#include "TextAsset.h"

#include <utility>
#include <memory>

#include "GraphicsEngine/Objects/Text.h"

#include <tracy/core/Tracy.hpp>

#include "Logger/Logger.h"

TextAsset::TextAsset()
{}

TextAsset::~TextAsset() = default;

bool TextAsset::Load(const std::filesystem::path& aPath)
{
    aPath;
    return false;
}
