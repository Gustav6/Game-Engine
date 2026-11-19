#pragma once

#include <Tools/Core/Interface/ITool.h>

class Texture;

class SceneViewport : public ITool
{
public:
	SceneViewport();
	~SceneViewport();

	void Update(float aDeltaTime) override;
	void Render() override;

private:
	std::unique_ptr<Texture> myRenderTarget;
};

