#include "WindowManager.h"

void WindowManager::SetResolution(unsigned int aWidth, unsigned int aHeight)
{
    aWidth;
    aHeight;
}

unsigned int WindowManager::GetWidth() const
{
    return myWidth;
}

unsigned int WindowManager::GetHeight() const
{
    return myHeight;
}

void WindowManager::SetBorderless(bool aBorderless)
{
    myBorderless = aBorderless;
}

bool WindowManager::IsBorderless() const
{
    return myBorderless;
}
