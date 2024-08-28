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
	vector<vector<double>> dye = {};
	vector<vector<double>> flowX = {};
	vector<vector<double>> flowY = {};
	vector<vector<bool>> fluidField = {};
	double cellSize = 1;
	double timeStep = .05;
	int renderScale = 8;

	fluid(int _sizeX, int _sizeY);

	void draw();

	void update();

	void gravity();

	void project();

	void advect();
};