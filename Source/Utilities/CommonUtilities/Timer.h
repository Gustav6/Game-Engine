#pragma once

#include <chrono>

namespace CommonUtilities
{
	class Timer
	{
		public:
			Timer();

			void Update();

			float GetDeltaTime() const;
			double GetTotalTime() const;

		private:
			std::chrono::steady_clock::time_point myLastTimePoint;

			float myDeltaTime;
			double myTotalTime;
	};
};