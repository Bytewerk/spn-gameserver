/*
 * Schlangenprogrammiernacht: A programming game for GPN18.
 * Copyright (C) 2018  bytewerk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <queue>

#include "Semaphore.h"

// forward declaration
class Bot;

class BotThreadPool
{
	public:
		enum JobType {
			Move,
			CollisionCheck
		};

		struct Job {
			JobType jobType;

			// inputs
			std::shared_ptr<Bot> bot;

			// output
			// for jobType == Move
			std::size_t steps;
			// for jobType == CollisionCheck
			std::shared_ptr<Bot> killer;

			Job(JobType type, const std::shared_ptr<Bot> myBot)
				: jobType(type), bot(myBot)
			{}
		};

	private:
		std::vector<std::thread> m_threads;
		std::queue< std::unique_ptr<Job> > m_inputJobs;
		std::queue< std::unique_ptr<Job> > m_processedJobs;

		std::mutex m_inputQueueMutex;
		std::mutex m_processedQueueMutex;

		Semaphore m_workAvailSemaphore; // blocks worker threads if there is no work

		bool m_shutdown = false;

		std::condition_variable m_finishedCV;
		std::mutex m_finishedMutex;

		std::size_t m_activeThreads;

	public:
		BotThreadPool(std::size_t num_threads);

		~BotThreadPool();

		/*!
		 * \brief Add a job to be processed in parallel.
		 *
		 * Processing starts immediately.
		 *
		 * \param job  The Job to add.
		 */
		void addJob(std::unique_ptr<Job> job);

		/*!
		 * \brief Wait until all jobs are processed.
		 *
		 * \details
		 * This function blocks until all jobs have been processed by the worker
		 * threads.
		 */
		void waitForCompletion(void);

		/*!
		 * \brief Get next processed job.
		 * \returns The next queue entry (which is removed from the queue) or
		 *          NULL if queue is empty.
		 */
		std::unique_ptr<Job> getProcessedJob(void);
};
