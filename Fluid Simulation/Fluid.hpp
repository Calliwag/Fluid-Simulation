#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"

class fluid
{
public:

	// Simulation Grids
	std::vector<std::vector<double>> flowX = {};
	std::vector<std::vector<double>> sourceX = {};
	std::vector<std::vector<double>> flowY = {};
	std::vector<std::vector<double>> sourceY = {};
	std::vector<std::vector<bool>> fluidField = {};

	// Dye Related
	std::vector<std::vector<glm::dvec4>> dye = {};
	std::vector<std::vector<glm::dvec4>> dyeSource = {};
	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };
	double decayValue = 1;
	double diffuseValue = 0;

	// Temporary Value Storage Grids
	std::vector<std::vector<glm::dvec2>> flowGrid = {};
	std::vector<std::vector<double>> curlGrid = {};
	std::vector<std::vector<double>> pressureGrid = {};

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
	RenderTexture2D fluidRenderTexture;
	RenderTexture2D linesRenderTexture;
	RenderTexture2D screenRenderTexture;

	// Constructor
	fluid(int _sizeX, int _sizeY);
	fluid(Image layoutImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, glm::dvec4 dyeColor, int _maxFrames);
	fluid(Image layoutImage, Image dyeImage, int _renderScale, int _drawMode, glm::dvec2 _drawMinMax, bool _drawLines, int _lineSize, int _maxFrames);

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

	void advectVelocity();

	void advectDye();

	void updateCurlGrid();

	void vorticityConfinement();
};