#pragma once
#include "Grid.hpp"
#include "Fluid.hpp"
#include <queue>
#include <mutex>
#include <thread>
#include <semaphore>

class Worker
{
	Fluid* fluid;

	std::queue<glm::ivec2>* tasks;
	std::mutex* tasksMutex;
	std::vector<glm::ivec2> currentTasks;
	int capacity;

	bool join;
	bool joined;

	std::thread* thr = nullptr;
	std::binary_semaphore active{ 1 };
	std::binary_semaphore working{ 1 };

	void Run()
	{
		joined = false;
		while (!join)
		{
			working.acquire();
			for (int i = 0; i < currentTasks.size(); i++)
			{
				if(!fluid->compressible) SolveIncompressibilityAt(currentTasks[i]);
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

				working.release();
			}
			else
			{
				tasksMutex->unlock();
				active.release();
			}
		}
		joined = true;
	};
	void SolveIncompressibilityAt(glm::ivec2 pos)
	{
		int& x = pos.x;
		int& y = pos.y;
		double divergence = (fluid->flowX[x + 1][y] * fluid->fluidField[x + 1][y]) -
			(fluid->flowX[x][y] * fluid->fluidField[x - 1][y]) +
			(fluid->flowY[x][y + 1] * fluid->fluidField[x][y + 1]) -
			(fluid->flowY[x][y] * fluid->fluidField[x][y - 1]);
		double fluidCount = fluid->fluidField[x + 1][y] + fluid->fluidField[x - 1][y] + fluid->fluidField[x][y + 1] + fluid->fluidField[x][y - 1];
		double correctionFactor = 1.9 * divergence / fluidCount;
		fluid->flowX[x + 1][y] -= correctionFactor * fluid->fluidField[x + 1][y];
		fluid->flowX[x][y] += correctionFactor * fluid->fluidField[x - 1][y];
		fluid->flowY[x][y + 1] -= correctionFactor * fluid->fluidField[x][y + 1];
		fluid->flowY[x][y] += correctionFactor * fluid->fluidField[x][y - 1];
		fluid->pressureGrid[x][y] -= divergence / (fluidCount * fluid->timeStep);
	};


public:
	Worker(Fluid* _fluid, std::queue<glm::ivec2>* _tasks, std::mutex* _tasksMutex, int _capacity)
	{
		fluid = _fluid;

		tasks = _tasks;
		tasksMutex = _tasksMutex;
		currentTasks = {};
		capacity = _capacity;

		join = false;
		working.acquire();
		thr = new std::thread([this] { this->Run(); });
	};
	void Go()
	{
		active.acquire();
		working.release();
	};
	void Wait()
	{
		active.acquire();
		active.release();
	}
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
	std::vector<Worker> daniels; // 6 little daniels running around... Mario Kart
	std::queue<glm::ivec2> tasks;
	std::mutex tasksMutex;
	glm::ivec2 size;
	Grid<uint8_t> fluidField;

	void Compute()
	{
		for (auto& daniel : daniels)
		{
			daniel.Go();
		}
		for (auto& daniel : daniels)
		{
			daniel.Wait();
		}
	}

public:
	Tasker(Fluid* fluid, int workerCount, int workerCap)
	{
		size = { fluid->sizeX,fluid->sizeY };
		fluidField = fluid->fluidField;

		tasks = {};

		daniels = {};
		for (int i = 0; i < workerCount; i++)
		{
			daniels.push_back(Worker(fluid, &tasks, &tasksMutex, workerCap));
		}
	}

	void Solve()
	{
		for (int x = 1; x < size.x - 1; x++)
		{
			for (int y = x % 2 + 1; y < size.y - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					tasks.push({ x,y });
				}
			}
		}
		Compute();
		for (int x = 1; x < size.x - 1; x++)
		{
			for (int y = 2 - (x % 2); y < size.y - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					tasks.push({ x,y });
				}
			}
		}
		Compute();
	}
};