#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Core/Scene/Components/Base/Component.h"

namespace CommonUtilities
{
	template <typename T>
	class Matrix4x4;
}

struct Animation;
struct Skeleton;
class AnimationAsset;

enum class JointGroup
{
	All,
	UpperBody,
	LowerBody,
	RightArm,
	LeftArm,
};

struct AnimationEntry
{
	const std::shared_ptr<AnimationAsset> AnimationAsset;
	const bool ShouldLoop = false;
	const float AnimationSpeedMultiplier = 1;
};

struct LayeredAnimation
{
	struct AnimationLayer
	{
		std::string AnimationName;
		JointGroup Group = JointGroup::All;
		size_t CurrentFrame = 0;
		float AnimationTimer = 0;
		float FrameTime = 0;
		bool HasFinished = false;
	};

	AnimationLayer Layer;
};

class AnimationComponent : public Component
{
public:
	AnimationComponent(Actor* aOwner, const Skeleton* aSkeleton);

	void Update(float aDeltaTime) override;

	void PlayAnimation(std::string aName, bool aIsNewRoot, JointGroup aGroup = JointGroup::All);

	void AddAnimation(std::string_view aName, const AnimationEntry& aEntry);

	CommonUtilities::Matrix4x4f* GetJointTransforms() const;

private:
	void SetupJointGroups(const Skeleton* aSkeleton);

	bool LayerEffectsJoint(JointGroup aGroup, size_t aJointIndex);

	void UpdateAnimation(size_t aCurrentFrame, unsigned int aJointIdx, const CommonUtilities::Matrix4x4f& aParentJointTransform, CommonUtilities::Matrix4x4f* outTransforms, JointGroup aGroup, const std::string& aCurrentAnimation);
	
	const Skeleton* mySkeleton;

	std::vector<LayeredAnimation> myAnimationStack;
	std::unordered_map<std::string, AnimationEntry> myAnimations;

	std::unordered_map<JointGroup, std::vector<size_t>> myJointGroups;

	CommonUtilities::Matrix4x4f myJointTransforms[128];
};

