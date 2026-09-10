#pragma once

class WindowManager
{
public:
	void SetResolution(unsigned int aWidth, unsigned int aHeight);

	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	void SetBorderless(bool aBorderless);
	bool IsBorderless() const;

private:
	bool myBorderless;
	
	unsigned int myWidth;
	unsigned int myHeight;
};