#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"
#include "Grid.hpp"
#include <thread>
#include <mutex>
#include "FluidCreate.hpp"

class Fluid
{
public:
	// Simulation Grids
	Grid<double> flowX = {};
	Grid<double> flowY = {};
	Grid <glm::dvec2> flowSource = {};
	Grid<uint8_t> fluidField = {};

	// Density/ Compressibility
	Grid<double> density = {}; // Check this out https://www.clawpack.org/riemann_book/html/Euler.html
	double baseDensity = 0.5;

	// Dye Related
	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	Grid<glm::dvec4> dye = {};
	Grid<glm::dvec4> dyeSource = {};
	double decayValue = 0;
	double diffuseValue = 0;

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

	// Threading for drawing
	bool updateThreadShouldJoin;
	std::mutex updateMutex;

	// Constructor
	Fluid(int _sizeX, int _sizeY);
	Fluid(Image layoutImage, glm::dvec4 dyeColor, double _vorticity, int _relaxationSteps);
	Fluid(Image layoutImage, Image dyeImage, double _vorticity, int _relaxationSteps);
	Fluid(FluidInfo info, double _vorticity, int _relaxationSteps);

	// Main Loop
	void mainLoop();

	// Drawing
	void draw();

	void storeScreenImage();

	// Simulation
	// Update loop
	void updateLoop();
	void update();

	// Update sources
	void updateFlowSources();
	void updateDyeSources();

	// Update flow and curl
	void updateFlowAndCurl();

	// Decay / diffuse dye
	void decayDye();
	void diffuseDye();

	// Solve incompressibility
	void solveIncompressibility();
	void solveIncompressibilityAt(int x, int y);

	// Advection
	void advectVelocity();
	void advectDye();

	// Vorticity Confinement
	void vorticityConfinement();
};