#pragma once
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"

using namespace std;

extern raylib::Window window;

class fluid
{
public:
	int sizeX = 0;
	int sizeY = 0;
	int frames = 0;
	vector<vector<Color>> dye = {};
	vector<vector<double>> flowX = {};
	vector<vector<double>> flowY = {};
	vector<vector<bool>> fluidField = {};
	double cellSize = 1;
	double timeStep = .1;
	int renderScale = 8;
	double decayValue = 0.99;
	double vorticity = 0.05;

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