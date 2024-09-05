#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"
#include "Grid.hpp"
#include <thread>
#include <future>

class fluid
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

	// Rendering
	raylib::Window window;
	int renderScale;
	int drawMode; // 0 = dye, 1 = pressure, 2 = curl
	glm::dvec2 drawMinMax;
	bool drawLines;
	int lineSize;
	int maxFrames;

	Grid<double> drawGrid;

	RenderTexture2D fluidRenderTexture;
	RenderTexture2D linesRenderTexture;
	RenderTexture2D screenRenderTexture;

	// Threading for drawing
	bool isUpdating = 0;
	std::thread* updateThread = nullptr;

	// Constructor
	fluid(int _sizeX, int _sizeY);
	fluid(Image layoutImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, glm::dvec4 dyeColor, int _maxFrames);
	fluid(Image layoutImage, Image dyeImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, int _maxFrames);

	// Destructor
	~fluid();

	// Main Loop
	void mainLoop();

	// Drawing
	void draw();

	// Simulation
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