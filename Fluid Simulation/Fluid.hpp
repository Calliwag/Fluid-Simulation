#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"
#include "Grid.hpp"
#include <thread>
#include <mutex>
#include "FluidCreate.hpp"

class Tasker;

class Fluid
{
public:
	// Simulation Grids
	Grid<double> flowX = {};
	Grid<double> flowY = {};
	Grid <glm::dvec2> flowSource = {};
	Grid<uint8_t> fluidField = {};
	Grid<double> densitySource = {};

	// Density/ Compressibility
	Grid<double> density = {}; // Check this out https://www.clawpack.org/riemann_book/html/Euler.html
	double baseDensity = 0;

	// Dye Related
	glm::dvec4 baseDye = {0,0,0,1};
	glm::dvec4 barrierColor = {1,1,1,1};

	Grid<glm::dvec4> dye = {};
	Grid<glm::dvec4> dyeSource = {};
	double decayValue = 0.0;
	double diffuseValue = 0.0;

	// Temporary Value Storage Grids
	Grid<glm::dvec2> flowGrid = {};
	Grid<double> curlGrid = {};
	Grid<double> pressureGrid = {};

	// Simulation Parameters
	int sizeX;
	int sizeY;
	int frame;
	double timeStep = 0.1;
	double vorticity;
	int relaxationSteps;
	bool compressible = false;
	double compressibility = 0.99;

	// Threading for drawing
	bool updateThreadShouldJoin;
	std::mutex updateMutex;

	// Threading for simulation
	Tasker* mark;

	// Constructor
	Fluid(int _sizeX, int _sizeY);
	Fluid(Image layoutImage, glm::dvec4 dyeColor, double _vorticity, int _relaxationSteps);
	Fluid(Image layoutImage, Image dyeImage, double _vorticity, int _relaxationSteps);
	Fluid(FluidInfo info, double _vorticity, int _relaxationSteps);

	// Simulation
	// Update loop
	void updateLoop();
	void update();

	// Update sources
	void updateFlowSources();
	void updateDensitySources();
	void updateDyeSources();

	// Update flow and curl
	void updateFlowAndCurl();

	// Decay / diffuse dye
	void decayDye();
	void diffuseDye();

	// Solve incompressibility
	void solveIncompressibility();
	void solveIncompressibilityAt(int x, int y);

	// Solve compressibility
	void solveCompressibility();
	void solveCompressibilityAt(int x, int y);

	// Advection
	void advectVelocity();
	void advectDye();
	void advectDensity();

	// Vorticity Confinement
	void vorticityConfinement();
};

#include <queue>
#include "ThreadQ.hpp"
#include <mutex>
#include <thread>
#include <semaphore>

class Worker
{
	Fluid* fluid;

	ThreadQ<glm::ivec2>* tasks;
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
				if (!fluid->compressible) SolveIncompressibilityAt(currentTasks[i]);
			}
			currentTasks = {};
			//tasksMutex->lock();
			if (!tasks->IsEmpty())
			{
				for (int i = 0; i < capacity && !tasks->IsEmpty(); i++)
				{
					currentTasks.push_back(tasks->Get());
					//tasks->pop();
				}
				//tasksMutex->unlock();

				working.release();
			}
			else
			{
				//tasksMutex->unlock();
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
	Worker(Fluid* _fluid, ThreadQ<glm::ivec2>* _tasks, std::mutex* _tasksMutex, int _capacity)
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
	std::vector<Worker*> daniels; // 6 little daniels running around... Mario Kart
	ThreadQ<glm::ivec2> tasks;
	ThreadQ<glm::ivec2> red;
	ThreadQ<glm::ivec2> black;
	std::mutex tasksMutex;
	glm::ivec2 size;
	Grid<uint8_t> fluidField;

	void Compute()
	{
		for (auto* daniel : daniels)
		{
			daniel->Go();
		}
		for (auto* daniel : daniels)
		{
			daniel->Wait();
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
			daniels.push_back(new Worker(fluid, &tasks, &tasksMutex, workerCap));
		}

		red = {};
		for (int x = 1; x < size.x - 1; x++)
		{
			for (int y = x % 2 + 1; y < size.y - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					red.push({ x,y });
				}
			}
		}

		black = {};
		for (int x = 1; x < size.x - 1; x++)
		{
			for (int y = 2 - (x % 2); y < size.y - 1; y += 2)
			{
				if (fluidField[x][y] == 1)
				{
					black.push({ x,y });
				}
			}
		}
	}

	void Solve()
	{
		tasks = red;
		Compute();
		
		tasks = black;
		Compute();
	}
};