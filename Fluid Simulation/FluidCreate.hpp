#pragma once
#include "Grid.hpp"
#include "glm/glm.hpp"

class FluidCreate
{
private:
	int sizeX;
	int sizeY;
	int renderScale;
	Grid<uint8_t> fluidField;
	Grid<glm::dvec4> dyeSource;
	Grid<glm::dvec2> flowSource;
	Grid<double> sourceX;
	Grid<double> sourceY;

	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	raylib::Window window;

public:
	FluidCreate(int _sizeX, int _sizeY, int _renderScale);
	void createLoop();
	void draw();
	void save();

};