#include "CommonUtilities.pch.h"
#include "Timer.h"

namespace CommonUtilities
{
	Timer::Timer()
	{
		myDeltaTime = 0;
		myTotalTime = 0;

		myLastTimePoint = std::chrono::high_resolution_clock::now();
	}

	/// <summary>
	/// Will update the delta time and the total time.
	/// </summary>
	void Timer::Update()
	{
		auto currentTimePoint = std::chrono::high_resolution_clock::now();

		float elapsed = std::chrono::duration<float>(currentTimePoint - myLastTimePoint).count();

		myDeltaTime = elapsed;

		myTotalTime += myDeltaTime;

		myLastTimePoint = currentTimePoint;
	}

	/// <summary>
	/// Will get the delta time for the current frame.
	/// </summary>
	/// <returns>[FLOAT] Representing the delta time for the current frame.</returns>
	float Timer::GetDeltaTime() const
	{
		return myDeltaTime;
	}

	/// <summary>
	/// Will get the total time (since) timer started.
	/// </summary>
	/// <returns>[FLOAT] Representing the applications total run time.</returns>
	double Timer::GetTotalTime() const
	{
		return myTotalTime;
	}
};