#pragma once
#include <vector>
#include <iostream>
#include <fstream>
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

	glm::dvec4 baseDye = { 0,0,0,1 };
	glm::dvec4 barrierColor = { 1,1,1,1 };

	raylib::Window window;

	// Drawing
	int drawType;
	bool rectMode = 0;
	glm::dvec2 drawVel;
	glm::dvec4 drawColor;
	bool drawingRect = 0;
	glm::dvec2 rectBase;
	MouseButton rectButton; // 1 = left, 2 = right
	bool exitWindow = 0;
	int topBorder = 24;

public:
	FluidCreate(int _sizeX, int _sizeY, int _renderScale);
	void createLoop();
	void draw();
	void input();
	glm::dvec2 mouseToGrid(Vector2 mousePos);

	friend class FluidInfo;

};

struct FluidInfo
{
public:
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t renderScale;

	Grid<uint8_t> fluidField;
	Grid<glm::dvec4> dyeSource;
	Grid<glm::dvec2> flowSource;
	glm::dvec4 baseDye;
	glm::dvec4 barrierColor;

public:
	// Construct from FluidCreate
	FluidInfo(FluidCreate data);

	// Load from file
	FluidInfo(std::string fileName);

	// Save to file
	void saveTo(std::string fileName);

};