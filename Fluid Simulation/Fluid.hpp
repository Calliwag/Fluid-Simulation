#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"

extern raylib::Window window;

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
	int sizeX = 0;
	int sizeY = 0;
	int frames = 0;
	double timeStep = 1.0 / 10;
	double vorticity = 0.0;
	int relaxationSteps = 50;

	// Rendering
	int renderScale = 6;
	int drawMode = 0; // 0 = dye, 1 = pressure, 2 = curl
	glm::dvec2 pressureMinMax = { -0,0 };
	glm::dvec2 curlMinMax = { -0,0 };
	bool drawLines = 1;
	int lineSize = 4;
	int maxDrawFrames = 0;
	RenderTexture2D fluidRenderTexture;
	RenderTexture2D linesRenderTexture;
	RenderTexture2D screenRenderTexture;

	// Constructor
	fluid(int _sizeX, int _sizeY);
	fluid(Image layoutImage, glm::dvec4 dyeColor);

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