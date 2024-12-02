#pragma once
#include "Grid.hpp"
#include "Fluid.hpp"
#include <queue>
#include <mutex>
#include <thread>
#include <semaphore>

class Worker
{
	Grid<double>* flowX = nullptr;
	Grid<double>* flowY = nullptr;

	std::queue<glm::ivec2>* tasks;
	std::mutex* tasksMutex;
	std::vector<glm::ivec2> currentTasks;
	int capacity;

	bool join;
	bool joined;

	std::thread* thr = nullptr;
	std::binary_semaphore active{ 1 };
	std::binary_semaphore idle{ 1 };

	void Run()
	{
		joined = false;
		while (!join)
		{
			idle.acquire();
			for (int i = 0; i < currentTasks.size(); i++)
			{
				SolveIncompressibilityAt(currentTasks[i]);
			}
			tasksMutex->lock();
			if (tasks->size() > 0)
			{
				currentTasks = {};
				for (int i = 0; i < capacity && tasks->size() > 0; i++)
				{
					currentTasks.push_back(tasks->front());
					tasks->pop();
				}
				tasksMutex->unlock();

				idle.release();
			}
			else
			{
				tasksMutex->unlock();
				active.release();
			}
		}
		joined = true;
	};
	void SolveIncompressibilityAt(glm::ivec2 pos);


public:
	Worker(Fluid* fluid, std::queue<glm::ivec2>* _tasks, std::mutex* _tasksMutex, int _capacity)
	{
		flowX = &fluid->flowX;
		flowY = &fluid->flowY;

		tasks = _tasks;
		tasksMutex = _tasksMutex;
		currentTasks = {};
		capacity = _capacity;

		join = false;
		idle.acquire();
		thr = new std::thread([this] { this->Run(); });
	};
	void Go()
	{
		active.acquire();
		idle.release();
	};
	void Wait()
	{
		active.acquire();
		active.release();
	}
	void Wait();
	void Join()
	{
		join = true;
		while (true)
		{
			if (joined)
			{
				thr->join();
				break;
			}
		}
	};
};

class Tasker
{
	std::vector<Worker> workers = {};
	glm::ivec2 gridSize;

public:
	Tasker(Fluid* fluid)
};