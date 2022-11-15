#include "Entropy/EntropyUtils.h"
#include "job.h"

namespace chs
{
	std::vector<std::thread> JobSystem::mThreads;
	std::atomic_bool JobSystem::mShutdownThreads;
	std::mutex JobSystem::mJobMutex;
	std::condition_variable JobSystem::mJobAvailable;
	std::function<void(void)> JobSystem::mJob;
	std::function<void(void)> JobSystem::mJobNotifyFunc;

	void Barrier::Wait()
	{
		// acquire lock
		std::unique_lock<std::mutex> lock(mutex);
		if (--count == 0)
			cv.notify_all();
		else
			// releases lock and waits for notify
			cv.wait(lock, [this]() {return count == 0; });
		// '.dtor' of 'lock' releases lock
	}

	void ThreadJob(std::shared_ptr<Barrier> barrier)
	{
		// wait for all threads to start up
		barrier->Wait();
		barrier.reset();

		// acquire 'JobSystem::mJobMutex' lock
		std::unique_lock<std::mutex> lock(JobSystem::mJobMutex);
		while (!JobSystem::mShutdownThreads)
		{
			// if there are no jobs available wait and release the lock
			// acquire the lock after wait() automatically
			if (!JobSystem::mJob)
				JobSystem::mJobAvailable.wait(lock);
			else
			{
				// copy the job fn from 'JobSystem::mJob', set 'JobSystem::mJob' to null, and release the lock
				// then call the job fn
				auto notify = JobSystem::mJobNotifyFunc;
				auto job = JobSystem::mJob;

				JobSystem::mJob = nullptr;
				JobSystem::mJobNotifyFunc = nullptr;
				// we have updated shared resources,
				// release the lock so a different thread can acquire it
				lock.unlock();
				ET_DEBUG_ASSERT(job);
				job();
				if (notify)
					notify();
				lock.lock();
			}
		}
		// 'lock' lock released by its destructor
	}

	void JobSystem::Init()
	{
		if (!mThreads.empty())
			return;

		// also includes main thread
		// so create numThreads - 1 threads
		auto numThreads = (std::thread::hardware_concurrency() - 2);

		auto barrier = std::make_shared<Barrier>(numThreads);

		for (size_t i = 0; i < numThreads - 1; i++)
			mThreads.emplace_back(std::thread(ThreadJob, barrier));

		// wait for all threads to startup
		barrier->Wait();
	}

	void JobSystem::Shutdown()
	{
		if (mThreads.empty())
			return;

		{
			// scoped RAII lock / unlock mutex
			std::lock_guard<std::mutex> lock(mJobMutex);
			mShutdownThreads = true;
			// wake all threads
			mJobAvailable.notify_all();
			// release lock because 'lock' goes out of scope 
		}
		// wait for all threads
		for (auto& thread : mThreads)
			thread.join();

		mThreads.clear();
		// set mShutdownThreads to false again to be able to restart JobSystem
		// no lock required because there are no other threads active
		mShutdownThreads = false;
	}

	void JobSystem::Job(std::function<void(void)> job, std::function<void(void)> notify)
	{
		// scoped RAII lock / unlock mutex
		std::lock_guard<std::mutex> lock(mJobMutex);
		mJob = job;
		mJobNotifyFunc = notify;
		// wake only one thread
		mJobAvailable.notify_one();
	}
}