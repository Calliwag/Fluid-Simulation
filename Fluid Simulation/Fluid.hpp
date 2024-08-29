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
	std::vector<std::vector<double>> flowX = {};
	std::vector<std::vector<double>> flowY = {};
	std::vector<std::vector<bool>> fluidField = {};
	double cellSize = 1;
	double timeStep = .1;
	int renderScale = 8;
	double decayValue = 0.995;
	double diffuseValue = 0.01;
	double vorticity = 0.1;

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