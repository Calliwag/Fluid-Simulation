#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"

extern raylib::Window window;

class fluid
{
public:
	int sizeX = 0;
	int sizeY = 0;
	int frames = 0;
	std::vector<std::vector<glm::dvec4>> dye = {};
	std::vector<std::vector<glm::dvec4>> dyeSource = {};
	glm::dvec4 baseDye = { 0,0,0,1 };
	std::vector<std::vector<double>> flowX = {};
	std::vector<std::vector<double>> sourceX = {};
	std::vector<std::vector<double>> flowY = {};
	std::vector<std::vector<double>> sourceY = {};
	std::vector<std::vector<bool>> fluidField = {};
	double cellSize = 1;
	double timeStep = 1.0 / 10;
	int renderScale = 8;
	double decayValue = 1;
	double diffuseValue = 0.0;
	double vorticity = 0.0;
	int relaxationSteps = 20;

	fluid(int _sizeX, int _sizeY);

	void draw();

	void update();

	void updateSources();

	void updateDyeSources();

	glm::dvec2 getGridVelocity(int x, int y);

	void decayDye();

	void project();

	void advect();

	double curl(int x, int y);

	void vorticityConfinement();
};