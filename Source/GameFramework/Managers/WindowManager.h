#pragma once

#include "CommonUtilities/Math/Vector2.hpp"

struct HWND__;
using HWND = HWND__*;

class WindowManager
{
public:
	static WindowManager& Get();

	void SetWindowHandle(HWND aWindowHandle);

	CommonUtilities::Vector2f GetScalingFactor();

	void SetBaseResolution(CommonUtilities::Vector2ui aResolution);
	void SetResolution(CommonUtilities::Vector2ui aResolution);

	CommonUtilities::Vector2ui GetResolution() const;

	void SetWidth(unsigned int aWidth);
	void SetHeight(unsigned int aHeight);

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	void SetBorderless(bool aIsBorderless);
	bool IsBorderless() const;

private:
	WindowManager() = default;

	HWND myWindowHandle = nullptr;

	CommonUtilities::Vector2ui myResolution{ 1920, 1080 };
	CommonUtilities::Vector2ui myBaseResolution{ 1920, 1080 };

	bool myIsBorderless = false;
};