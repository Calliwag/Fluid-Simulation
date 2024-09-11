#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"
#include "Grid.hpp"
#include <thread>
#include <mutex>

class Fluid
{
public:
	// Simulation Grids
	Grid<double> flowX = {};
	Grid<double> sourceX = {};
	Grid<double> flowY = {};
	Grid<double> sourceY = {};
	Grid<uint8_t> fluidField = {};

	// Dye Related
	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	Grid<glm::dvec4> dye = {};
	Grid<glm::dvec4> dyeSource = {};
	double decayValue = 1;
	double diffuseValue = 0;

	// Temporary Value Storage Grids
	Grid<glm::dvec2> flowGrid;
	Grid<double> curlGrid = {};
	Grid<double> pressureGrid = {};

	// Simulation Parameters
	int sizeX;
	int sizeY;
	int frames = 0;
	double timeStep = 1.0 / 10;
	double vorticity = 0.0;
	int relaxationSteps = 50;

	// Threading for drawing
	bool updateThreadShouldJoin = 0;
	std::mutex m;

	// Constructor
	Fluid(int _sizeX, int _sizeY);
	Fluid(Image layoutImage, glm::dvec4 dyeColor);
	Fluid(Image layoutImage, Image dyeImage);

	// Destructor
	~Fluid();

	// Main Loop
	void mainLoop();

	// Drawing
	void draw();

	void storeScreenImage();

	// Simulation
	void updateLoop();

	void update();

	void updateFlowSources();

	void updateDyeSources();

	void updateFlowGrid();

	void decayDye();

	void diffuseDye();

	void solveIncompressibility();

	bool solveIncompressibilityAt(int x, int y);

	void advectVelocity();

	void advectDye();

	void updateCurlGrid();

	void vorticityConfinement();
};