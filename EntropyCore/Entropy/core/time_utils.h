#pragma once
#include <chrono>

namespace et
{
	class TimeStep
	{
	public:
		TimeStep(float time = 0.0f)
			: mTime(time)
		{}
		~TimeStep() = default;

		operator float() const { return mTime; }
		float GetSeconds() const { return mTime; }
		float GetMilliSeconds() const { return (float)(mTime * 1e3); }

	private:
		float mTime;
	};

	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			mStart = std::chrono::high_resolution_clock::now();
		}

		float Elapsed()
		{
			return (float)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - mStart).count() * 1e-9);
		}

		float ElapsedMilli()
		{

			return (float)(Elapsed() * 1e3);
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
	};

	class Time
	{
	public:
		static float GetTime() { return sAppTimer.Elapsed(); }
		static float GetTimeMilli() { return sAppTimer.ElapsedMilli(); }
		static void Reset() { sAppTimer.Reset(); }
		static float DeltaTime() { return sDelta; }

	private:
		static Timer sAppTimer;
		static float sDelta;

		friend class Application;
	};
}