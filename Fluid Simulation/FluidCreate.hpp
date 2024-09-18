#pragma once
#include <vector>
#include <iostream>
#include "glm/glm.hpp"
#include "raylib-cpp.hpp"
#include "Grid.hpp"

class FluidCreate
{
private:
	int sizeX;
	int sizeY;
	int renderScale;

	Grid<uint8_t> fluidField = {};
	Grid<glm::dvec4> dyeSource = {};
	Grid<glm::dvec2> flowSource = {};
	Grid<double> sourceX = {};
	Grid<double> sourceY = {};

	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	raylib::Window window;

	int drawType;
	glm::dvec2 drawVel;

public:
	FluidCreate(int _sizeX, int _sizeY, int _renderScale);
	void createLoop();
	void draw();
	void input();
	glm::ivec2 mouseToGrid(Vector2 mousePos);
	void save();

};