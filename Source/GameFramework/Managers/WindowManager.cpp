#include "GameFramework.pch.h"
#include "WindowManager.h"

#define	WIN32_LEAN_AND_MEAN
#include <Windows.h>

WindowManager& WindowManager::Get()
{
    static WindowManager myInstance;
    return myInstance;
}

void WindowManager::SetWindowHandle(HWND aWindowHandle)
{
    myWindowHandle = aWindowHandle;
}

CommonUtilities::Vector2f WindowManager::GetScalingFactor()
{
    return {
        static_cast<float>(myResolution.x) / static_cast<float>(myBaseResolution.x),
        static_cast<float>(myResolution.y) / static_cast<float>(myBaseResolution.y)
    };
}

void WindowManager::SetBaseResolution(CommonUtilities::Vector2ui aResolution)
{
    myBaseResolution = aResolution;
}

void WindowManager::SetResolution(CommonUtilities::Vector2ui aResolution)
{
    myResolution = aResolution;
}

CommonUtilities::Vector2ui WindowManager::GetResolution() const
{
    return myResolution;
}

void WindowManager::SetWidth(unsigned int aWidth)
{
    myResolution.x = aWidth;
}

void WindowManager::SetHeight(unsigned int aHeight)
{
    myResolution.y = aHeight;
}

unsigned int WindowManager::GetWidth() const
{
    return myResolution.x;
}

unsigned int WindowManager::GetHeight() const
{
    return myResolution.y;
}

void WindowManager::SetBorderless(bool aIsBorderless)
{
    if (myIsBorderless == aIsBorderless)
    {
		return;
    }

    LONG_PTR style = GetWindowLongPtr(myWindowHandle, GWL_STYLE);

    if (aIsBorderless)
    {
        style &= ~WS_OVERLAPPEDWINDOW;
        style |= WS_POPUP;
    }
    else
    {
        style &= ~WS_POPUP;
        style |= WS_OVERLAPPEDWINDOW;
    }

    SetWindowLongPtr(myWindowHandle, GWL_STYLE, style);

    SetWindowPos(
        myWindowHandle,
        nullptr,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE |
        SWP_NOZORDER | SWP_NOACTIVATE |
        SWP_FRAMECHANGED
    );

    myIsBorderless = aIsBorderless;
}

bool WindowManager::IsBorderless() const
{
    return myIsBorderless;
}
