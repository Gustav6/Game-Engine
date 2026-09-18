#pragma once

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"


#include "GamePipeline/World/Actor.hpp"



class Player : public Actor
{
public:

	Player() = default;
	~Player();

	void Init();
	void Update(float aDeltaTime);



private:


};