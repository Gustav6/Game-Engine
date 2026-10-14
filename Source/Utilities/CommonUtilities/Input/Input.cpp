#include "CommonUtilities.pch.h"
#include "Input.h"

namespace CommonUtilities
{
	Input::Input()
	{
		myCursorCurrentPosition = {0, 0};
		myCursorPreviousPosition = {0, 0};
		myCursorTentativePosition = {0, 0};

		myScrollWheelDelta = 0;
		myScrollWheelTentative = 0;

		myHWND = nullptr;
	}

	void Input::SetWindowHandle(HWND aWindowHandle)
	{
		myHWND = aWindowHandle;
	}

	/// <summary>
	/// Will update both mouse and keyboard inputs.
	/// </summary>
	void Input::UpdateInput()
	{
		myKeysPreviousState = myKeysCurrentState;
		myKeysCurrentState = myKeysTentativeState;

		myCursorPreviousPosition = myCursorCurrentPosition;
		myCursorCurrentPosition = myCursorTentativePosition;

		myScrollWheelDelta = myScrollWheelTentative;
		myScrollWheelTentative = 0;
	}

	/// <summary>
	/// This function will only handle relevant events.
	/// </summary>
	/// <returns>[BOOL] Representing if the event was handled.</returns>
	bool Input::UpdateEvents(unsigned int aMessage, u64 aWParam, s64 aLParam)
	{
		switch (aMessage)
		{
			case WM_KEYDOWN:
			{
				unsigned int keyDown = static_cast<unsigned int>(aWParam);
				myKeysTentativeState[keyDown] = true;
				return true;
			}
			case WM_KEYUP:
			{
				unsigned int keyUp = static_cast<unsigned int>(aWParam);
				myKeysTentativeState[keyUp] = false;
				return true;
			}
			case WM_LBUTTONDOWN:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::LMB)] = true;
				return true;
			}
			case WM_LBUTTONUP:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::LMB)] = false;
				return true;
			}
			case WM_RBUTTONDOWN:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::RMB)] = true;
				return true;
			}
			case WM_RBUTTONUP:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::RMB)] = false;
				return true;
			}
			case WM_MBUTTONDOWN:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::MMB)] = true;
				return true;
			}
			case WM_MBUTTONUP:
			{
				myKeysTentativeState[static_cast<size_t>(KeyCode::MMB)] = false;
				return true;
			}
			case WM_MOUSEWHEEL:
			{
				myScrollWheelTentative += GET_WHEEL_DELTA_WPARAM(aWParam);

				return true;
			}
			case WM_MOUSEMOVE:
			{
				myCursorTentativePosition = { static_cast<float>(aLParam & 0xFFFF), static_cast<float>((aLParam >> 16) & 0xFFFF) };

				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// Will iterate through all keys to check current state.
	/// </summary>
	/// <returns>[BOOL] Representing if any key is down.</returns>
	bool Input::IsAnyKeyDown() const
	{
		for (size_t keyIndex = 0; keyIndex < myKeysCurrentState.size(); keyIndex++)
		{
			if (IsKeyDown(static_cast<KeyCode>(keyIndex)))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// Will iterate through all keys to check pressed state.
	/// </summary>
	/// <returns>[BOOL] Representing if any key has been pressed.</returns>
	bool Input::IsAnyKeyPressed() const
	{
		for (size_t keyIndex = 0; keyIndex < myKeysCurrentState.size(); keyIndex++)
		{
			if (IsKeyPressed(static_cast<KeyCode>(keyIndex)))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// Will iterate through all keys to check released state.
	/// </summary>
	/// <returns>[BOOL] Representing if any key has been released.</returns>
	bool Input::IsAnyKeyReleased() const
	{
		for (size_t keyIndex = 0; keyIndex < myKeysCurrentState.size(); keyIndex++)
		{
			if (IsKeyReleased(static_cast<KeyCode>(keyIndex)))
			{
				return true;
			}
		}

		return false;
	}

	/// <summary>
	/// Will check the down state for the given key.
	/// </summary>
	/// <returns>[BOOL] Representing the down state of the given key.</returns>
	bool Input::IsKeyDown(const KeyCode aKeyCode) const
	{
		assert(aKeyCode != KeyCode::Count);

		return myKeysCurrentState[static_cast<size_t>(aKeyCode)];
	}

	/// <summary>
	/// Will check the pressed state for the given key.
	/// </summary>
	/// <param name="aKeyCode"></param>
	/// <returns>[BOOL] Representing the pressed state of the given key.</returns>
	bool Input::IsKeyPressed(const KeyCode aKeyCode) const
	{
		assert(aKeyCode != KeyCode::Count);

		return myKeysCurrentState[static_cast<size_t>(aKeyCode)] && !myKeysPreviousState[static_cast<size_t>(aKeyCode)];
	}

	/// <summary>
	/// Will check the released state for the given key.
	/// </summary>
	/// <returns>[BOOL] Representing the released state of the given key.</returns>
	bool Input::IsKeyReleased(const KeyCode aKeyCode) const
	{
		assert(aKeyCode != KeyCode::Count);

		return !myKeysCurrentState[static_cast<size_t>(aKeyCode)] && myKeysPreviousState[static_cast<size_t>(aKeyCode)];
	}

	/// <summary>
	/// Will get the current cursor position relative to the window.
	/// </summary>
	/// <returns>[POINT] Representing the window cursor coordinates.</returns>
	Vector2f Input::GetMousePosition() const
	{
		return myCursorCurrentPosition;
	}

	/// <summary>
	/// Will get the current cursor position relative to the screen.
	/// </summary>
	/// <returns>[POINT] Representing the screen cursor coordinates.</returns>
	Vector2f Input::GetMousePositionAbsolute() const
	{
		POINT absolutePosition;

		if (GetCursorPos(&absolutePosition))
		{
			return { static_cast<float>(absolutePosition.x), static_cast<float>(absolutePosition.y) };
		}
		else
		{
			return Vector2f::Zero;
		}
	}

	/// <summary>
	/// Will get the delta value for the cursor.
	/// </summary>
	/// <returns>[POINT] Representing the mouse delta coordinates.</returns>
	Vector2f Input::GetMousePositionDelta() const
	{
		return { myCursorCurrentPosition.x - myCursorPreviousPosition.x, myCursorCurrentPosition.y - myCursorPreviousPosition.y };
	}

	/// <summary>
	/// Will get the accumulated scroll delta value.
	/// </summary>
	/// <returns>[INT] Representing the accumulated scroll delta value (starts at 120).</returns>
	int Input::GetScrollWheelDelta() const
	{
		return myScrollWheelDelta;
	}

	/// <summary>
	/// Will show the windows cursor
	/// </summary>
	void Input::ShowMouse() const
	{
		ShowCursor(true);
	}

	/// <summary>
	/// Will hide the windows cursor
	/// </summary>
	void Input::HideMouse() const
	{
		ShowCursor(false);
	}

	/// <summary>
	/// Will lock the cursor to the client window
	/// </summary>
	void Input::CaptureMouse() const
	{
		RECT clipRect;

		GetClientRect(myHWND, &clipRect);

		POINT upperLeft = 
		{
			.x = clipRect.left,
			.y = clipRect.top
		};

		POINT lowerRight = 
		{
			.x = clipRect.right,
			.y = clipRect.bottom
		};

		MapWindowPoints(myHWND, nullptr, &upperLeft, 1);
		MapWindowPoints(myHWND, nullptr, &lowerRight, 1);

		clipRect.left = upperLeft.x;
		clipRect.top = upperLeft.y;
		clipRect.right = lowerRight.x;
		clipRect.bottom = lowerRight.y;

		ClipCursor(&clipRect);
	}

	/// <summary>
	/// Will unlock the cursor so it is no longer locked to the client window
	/// </summary>
	void Input::ReleaseMouse() const
	{
		ClipCursor(nullptr);
	}

	/// <summary>
	/// Will center the cursor to the middle of the application window
	/// </summary>
	void Input::CenterCursor() const
	{
		RECT clientRect;
		GetClientRect(myHWND, &clientRect);

		POINT center = 
		{
			.x = (clientRect.right - clientRect.left) / 2,
			.y = (clientRect.bottom - clientRect.top) / 2
		};

		ClientToScreen(myHWND, &center);

		SetCursorPos(center.x, center.y);
	}
}