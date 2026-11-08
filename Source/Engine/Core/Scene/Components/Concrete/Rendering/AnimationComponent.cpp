#include "Core/Core.pch.h"
#include "AnimationComponent.h"

#include "Graphics/Objects/Mesh/Skeleton.h"
#include "Graphics/Objects/Mesh/Animation.h"

#include "Core/Assets/Concrete/AnimationAsset.h"

#include "Core/Scene/World/Actor.hpp"

AnimationComponent::AnimationComponent(Actor* aOwner, const Skeleton* aSkeleton)
	: Component(aOwner)
{
	SetupJointGroups(aSkeleton);

	mySkeleton = aSkeleton;
}

void AnimationComponent::Update(float aDeltaTime)
{
	bool jointsNeedRebuild = false;

	for (LayeredAnimation& animation : myAnimationStack)
	{
		LayeredAnimation::AnimationLayer& layer = animation.Layer;
		const AnimationEntry& entry = myAnimations.at(layer.AnimationName);

		layer.AnimationTimer += aDeltaTime * entry.AnimationSpeedMultiplier;

		if (layer.AnimationTimer < layer.FrameTime)
		{
			continue;
		}

		size_t nextFrame = layer.CurrentFrame + 1;

		if (nextFrame >= entry.AnimationAsset->GetAnimation().Frames.size())
		{
			if (!entry.ShouldLoop)
			{
				layer.HasFinished = true;
				jointsNeedRebuild = true;
				continue;
			}

			nextFrame = 0;
		}

		layer.AnimationTimer -= layer.FrameTime;
		layer.CurrentFrame = nextFrame;
		jointsNeedRebuild = true;
	}

	std::erase_if(myAnimationStack, [](const LayeredAnimation& a)
	{
		return a.Layer.HasFinished;
	});

	if (jointsNeedRebuild)
	{
		for (const LayeredAnimation& animation : myAnimationStack)
		{
			const auto& layer = animation.Layer;
			UpdateAnimation(layer.CurrentFrame, 0, CommonUtilities::Matrix4x4f::Identity, myJointTransforms, layer.Group, layer.AnimationName);
		}
	}
}

void AnimationComponent::PlayAnimation(std::string aName, bool aIsNewRoot, JointGroup aGroup)
{
	LayeredAnimation::AnimationLayer layer;
	layer.AnimationName = aName;
	layer.Group = aGroup;
	layer.CurrentFrame = 0;
	layer.AnimationTimer = 0;
	layer.FrameTime = 1.0f / myAnimations.at(aName).AnimationAsset->GetAnimation().FramesPerSecond;
	layer.HasFinished = false;

	LayeredAnimation animation;
	animation.Layer = layer;

	if (aIsNewRoot)
	{
		myAnimationStack.clear();
	}

	myAnimationStack.emplace_back(animation);
}

void AnimationComponent::AddAnimation(std::string_view aName, const AnimationEntry& aEntry)
{
	myAnimations.emplace(aName, aEntry);
}

CommonUtilities::Matrix4x4f* AnimationComponent::GetJointTransforms() const
{
	return const_cast<CommonUtilities::Matrix4x4f*>(myJointTransforms);
}

void AnimationComponent::SetupJointGroups(const Skeleton* aSkeleton)
{
	for (const auto& [name, index] : aSkeleton->JointNameToIndex)
	{
		const bool startsWithRight = name.rfind("Right", 0) == 0;
		const bool startsWithLeft = name.rfind("Left", 0) == 0;

		const bool hasArm = name.find("Arm") != std::string::npos;
		const bool hasHand = name.find("Hand") != std::string::npos;
		const bool hasShoulder = name.find("Shoulder") != std::string::npos;
		const bool hasForeArm = name.find("ForeArm") != std::string::npos;
		const bool hasSpine = name.find("Spine") != std::string::npos;
		const bool hasNeck = name.find("Neck") != std::string::npos;
		const bool hasHead = name.find("Head") != std::string::npos;
		const bool hasLeg = name.find("Leg") != std::string::npos;
		const bool hasFoot = name.find("Foot") != std::string::npos;
		const bool hasToe = name.find("Toe") != std::string::npos;
		const bool hasHips = name.find("Hips") != std::string::npos;

		if (startsWithRight && (hasArm || hasHand || hasShoulder || hasForeArm))
		{
			myJointGroups[JointGroup::UpperBody].emplace_back(index);
			myJointGroups[JointGroup::RightArm].emplace_back(index);
		}
		else if (startsWithLeft && (hasArm || hasHand || hasShoulder || hasForeArm))
		{
			myJointGroups[JointGroup::UpperBody].emplace_back(index);
			myJointGroups[JointGroup::LeftArm].emplace_back(index);
		}
		else if (hasSpine || hasNeck || hasHead)
		{
			myJointGroups[JointGroup::UpperBody].emplace_back(index);
		}
		else if (hasLeg || hasFoot || hasToe || hasHips)
		{
			myJointGroups[JointGroup::LowerBody].emplace_back(index);
		}
	}
}

bool AnimationComponent::LayerEffectsJoint(JointGroup aGroup, size_t aJointIndex)
{
	if (aGroup == JointGroup::All)
	{
		return true;
	}

	auto iterator = myJointGroups.find(aGroup);

	if (iterator != myJointGroups.end())
	{
		const std::vector<size_t>& indices = iterator->second;

		if (std::find(indices.begin(), indices.end(), aJointIndex) != indices.end())
		{
			return true;
		}
	}

	return false;
}

void AnimationComponent::UpdateAnimation(size_t aCurrentFrame, unsigned int aJointIdx, const CommonUtilities::Matrix4x4f& aParentJointTransform, CommonUtilities::Matrix4x4f* outTransforms, JointGroup aGroup, const std::string& aCurrentAnimation)
{
	const Animation& currentAnimation = myAnimations.at(aCurrentAnimation).AnimationAsset->GetAnimation();

	const Skeleton::Joint& currentJoint = mySkeleton->Joints[aJointIdx];

	const CommonUtilities::Matrix4x4f& jointLocalTransform = currentAnimation.Frames[aCurrentFrame].Transforms.at(currentJoint.Name);

	const CommonUtilities::Matrix4x4f localTransform = jointLocalTransform * aParentJointTransform;

	if (LayerEffectsJoint(aGroup, aJointIdx))
	{
		outTransforms[aJointIdx] = currentJoint.BindPoseInverse * localTransform;
	}

	for (const unsigned int childJoint : currentJoint.Children)
	{
		UpdateAnimation(aCurrentFrame, childJoint, localTransform, outTransforms, aGroup, aCurrentAnimation);
	}
}
