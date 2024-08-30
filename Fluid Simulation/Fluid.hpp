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
	glm::dvec4 baseDye = { 0,0,0,1 };
	std::vector<std::vector<double>> flowX = {};
	std::vector<std::vector<double>> flowY = {};
	std::vector<std::vector<bool>> fluidField = {};
	double cellSize = 1;
	double timeStep = 1.0 / 10;
	int renderScale = 4;
	double decayValue = 0.999;
	double diffuseValue = 0.0;
	double vorticity = 0.0;

	fluid(int _sizeX, int _sizeY);

	void draw();

	void update();

	glm::dvec2 getGridVelocity(int x, int y);

	void decayDye();

	void project();

	void advect();

	double curl(int x, int y);

	void vorticityConfinement();
};