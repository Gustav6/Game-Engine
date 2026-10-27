#include "Core.pch.h"
#include "AnimationAsset.h"

#include "Graphics/Objects/Mesh/Animation.h"

#include <CommonUtilities/Math/Matrix4x4.hpp>

AnimationAsset::AnimationAsset()
{
    myAnimation = std::make_unique<Animation>();
}

AnimationAsset::~AnimationAsset() = default;

const Animation& AnimationAsset::GetAnimation() const
{
    return *myAnimation;
}

bool AnimationAsset::Load(const std::filesystem::path& aPath)
{
    ZoneScopedN("Load Animation")

    std::unique_ptr<TGA::FBX::Animation> tgaAnimation = std::make_unique<TGA::FBX::Animation>();
    const TGA::FBX::FbxImportStatus& importStatus = TGA::FBX::Importer::LoadAnimation(aPath, *tgaAnimation);

    if (!importStatus)
    {
        const std::string pathString = aPath.string();

        switch (importStatus.Result)
        {
            case TGA::FBX::FbxImportResult::InternalError:
            {
                SPDLOG_ERROR("Could not load animation [{}] : Internal FBX importer error!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NotInitialized:
            {
                SPDLOG_ERROR("Could not load animation [{}] : FBX importer is not initialized!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::FileNotFound:
            {
                SPDLOG_ERROR("Could not load animation [{}] : File not found!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NoSkeletonData:
            {
                SPDLOG_ERROR("Could not load animation [{}] : No skeleton data found in file!", pathString);
                break;
            }
            case TGA::FBX::FbxImportResult::NoAnimationData:
            {
                SPDLOG_ERROR("Could not load animation [{}] : No animation data found in file!", pathString);
                break;
            }
            default:
            {
                SPDLOG_ERROR("Could not load animation [{}] : Unknown error!", pathString);
                break;
            }
        }

        return false;
    }

    if (!importStatus.Warnings.empty())
    {
        const std::string pathString = aPath.string();

        for (const auto& [warning, count] : importStatus.Warnings)
        {
            switch (warning)
            {
                case TGA::FBX::FbxImportWarning::TooManySkinWeights:
                {
                    SPDLOG_WARN("Animation [{}] : vertices had too many skin weights and were truncated!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::MissingSkinWeights:
                {
                    SPDLOG_WARN("Animation [{}] : vertices are missing skin weights!", pathString);
                    break;
                }
                case TGA::FBX::FbxImportWarning::DuplicateBoneNames:
                {
                    SPDLOG_WARN("Animation [{}] : duplicate bone names detected, skinning may be incorrect!", pathString);
                    break;
                }
                default:
                {
                    SPDLOG_WARN("Animation [{}] : Unknown warning!", pathString);
                    break;
                }
            }
        }
    }

    std::vector<Animation::Frame> frames;
    frames.reserve(tgaAnimation->Frames.size());

    for (TGA::FBX::Animation::Frame& tgaFrame : tgaAnimation->Frames)
    {
        Animation::Frame frame;

        for (auto& [tgaJointName, tgaJointTransform] : tgaFrame.LocalTransforms)
        {
            memcpy_s(&frame.Transforms[tgaJointName], sizeof(CommonUtilities::Matrix4x4f), &tgaJointTransform, sizeof(TGA::FBX::Matrix));
        }

        frames.emplace_back(std::move(frame));
    }

    myAnimation->Frames = std::move(frames);
    memcpy_s(&myAnimation->Duration, sizeof(float), &tgaAnimation->Duration, sizeof(float));
    memcpy_s(&myAnimation->FramesPerSecond, sizeof(float), &tgaAnimation->FramesPerSecond, sizeof(float));

    return true;
}
