#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>

namespace chs
{
	class Barrier
	{
	public:
		Barrier(int32_t count) : count(count) {}
		~Barrier() {}
		void Wait();

	private:
		std::mutex mutex;
		std::condition_variable cv;
		int32_t count;
	};

	class JobSystem
	{
	public:
		static void Init();
		static void Shutdown();
		static void Job(std::function<void(void)> job, std::function<void(void)> notify = nullptr);

	private:
		static std::vector<std::thread> mThreads;
		static std::atomic_bool mShutdownThreads;
		static std::mutex mJobMutex;
		static std::condition_variable mJobAvailable;
		static std::function<void(void)> mJob;
		static std::function<void(void)> mJobNotifyFunc;

		friend void ThreadJob(std::shared_ptr<Barrier> barrier);
	};

}